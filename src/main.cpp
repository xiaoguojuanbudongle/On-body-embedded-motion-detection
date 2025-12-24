#include "mbed.h"

#include "config.h"
#include "lsm6dsl_driver.h"
#include "fft_utils.h"
#include "detector.h"
#include "ble_service.h"

using namespace std::chrono;

// On-board LEDs (see UM2153 Table 2)
static DigitalOut led_tremor(LED1); // PA5 - indicates tremor intensity
static DigitalOut led_dysk(LED2);   // PB14 - indicates dyskinesia intensity

// Serial output (for debugging)
static BufferedSerial pc(USBTX, USBRX, 115200);

// Sample buffers
static float g_ax[SAMPLES_PER_WINDOW];
static float g_ay[SAMPLES_PER_WINDOW];
static float g_az[SAMPLES_PER_WINDOW];

static float g_mag[SAMPLES_PER_WINDOW];
static float g_spectrum[FFT_LENGTH / 2];

static std::size_t g_sample_index = 0;

// Simple wrapper for formatted serial output
static void pc_printf(const char *fmt, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len > 0) {
        pc.write(buffer, len);
    }
}

// Update on-board LEDs based on detection results
static void update_leds(const DetectionResult &res)
{
    // Current convention:
    // - tremor_level = 0: LED1 off; 1–3: LED1 on
    // - dysk_level   = 0: LED2 off; 1–3: LED2 on
    // - fog_level  > 0 : quick double-flash of both LEDs to signal FOG

    if (res.tremor_level > 0) {
        led_tremor = 1;
    } else {
        led_tremor = 0;
    }

    if (res.dyskinesia_level > 0) {
        led_dysk = 1;
    } else {
        led_dysk = 0;
    }

    if (res.fog_level > 0) {
        // Perform a quick double-flash as a blocking visual notification
        // (3 s windows; the short blocking is acceptable here)
        for (int i = 0; i < 2; ++i) {
            led_tremor = !led_tremor;
            led_dysk   = !led_dysk;
            ThisThread::sleep_for(200ms);
        }
    }
}

// Process one complete 3s window: spectrum -> detection -> LED/BLE/Teleplot
// This function runs the full per-window pipeline and publishes results.
static void process_window()
{
    // 1) Compute magnitude
    compute_magnitude(g_ax, g_ay, g_az, SAMPLES_PER_WINDOW, g_mag);

    // 2) Estimate step count
    const std::uint16_t step_count = estimate_step_count(g_mag, SAMPLES_PER_WINDOW);

    // 3) Compute DFT magnitude spectrum
    compute_dft_magnitude(g_mag, SAMPLES_PER_WINDOW, g_spectrum, FFT_LENGTH);

    // 4) Band energy + FOG detection
    DetectionResult res = detect_conditions(
        g_spectrum,
        FFT_LENGTH / 2,
        step_count
    );

    // Print a line of debug info so values are readable over serial
    pc_printf("[WIN] steps=%u, tremor_rms=%.4f g, dysk_rms=%.4f g, tremor_lvl=%u, dysk_lvl=%u, fog=%u\r\n",
              step_count,
              res.tremor_band_rms_g,
              res.dyskinesia_band_rms_g,
              res.tremor_level,
              res.dyskinesia_level,
              res.fog_level);

    // Teleplot output: uses ">name:value" format so VSCode Teleplot plugin can plot directly
    pc_printf(">steps:%u\r\n", step_count);
    pc_printf(">tremor_rms:%.4f\r\n",   res.tremor_band_rms_g);
    pc_printf(">dysk_rms:%.4f\r\n",     res.dyskinesia_band_rms_g);
    pc_printf(">tremor_lvl:%u\r\n",     res.tremor_level);
    pc_printf(">dysk_lvl:%u\r\n",       res.dyskinesia_level);
    pc_printf(">fog:%u\r\n",            res.fog_level);

    // 5) Update LEDs
    update_leds(res);

    // 6) Update the three BLE characteristics
    ble_service_update(res.tremor_level, res.dyskinesia_level, res.fog_level);
}

int main()
{
    // Quick greeting
    pc_printf("\r\nRTES F25 - Shake, Rattle, Roll and Freeze\r\n");
    pc_printf("Board: B-L475E-IOT01A, IMU: LSM6DSL, fs=%.1f Hz, window=%.1f s\r\n",
              SAMPLE_FREQUENCY_HZ, WINDOW_SECONDS);

    // Initial LED states
    led_tremor = 0;
    led_dysk   = 0;

    // Initialize IMU
    bool imu_ok = lsm6dsl_init();
    if (!imu_ok) {
        pc_printf("[ERROR] LSM6DSL init failed, check I2C wiring / board config\r\n");
    }

    // Initialize BLE
    ble_service_init();

    // Sampling timer
    Timer sample_timer;
    sample_timer.start();
    auto last_sample_time = sample_timer.elapsed_time();
    const microseconds sample_period_us(
        static_cast<int>(1000000.0f / SAMPLE_FREQUENCY_HZ)
    );

    while (true) {
        // 1) Timed sampling
        auto now = sample_timer.elapsed_time();
        if (now - last_sample_time >= sample_period_us) {
            last_sample_time += sample_period_us;

            float ax = 0.0f, ay = 0.0f, az = 0.0f;
            bool ok = lsm6dsl_read_accel(ax, ay, az);

            if (ok && g_sample_index < SAMPLES_PER_WINDOW) {
                g_ax[g_sample_index] = ax;
                g_ay[g_sample_index] = ay;
                g_az[g_sample_index] = az;
                ++g_sample_index;
            }

            if (g_sample_index >= SAMPLES_PER_WINDOW) {
                process_window();
                g_sample_index = 0;
            }
        }

        // 2) Let BLE process stack events
        ble_service_process();

        // 3) Short sleep to reduce busy-waiting
        ThisThread::sleep_for(2ms);
    }
}
