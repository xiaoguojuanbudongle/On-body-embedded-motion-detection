#include "detector.h"
#include "config.h"

#include <cmath>

// Global internal state used for cross-window FOG detection
// (i.e. to detect a sudden stop after a period of walking)
static std::size_t g_consecutive_walking_windows = 0;

static std::uint8_t classify_level(float rms_g,
                                   float l1,
                                   float l2,
                                   float l3)
{
    if (rms_g < l1) {
        return 0;
    } else if (rms_g < l2) {
        return 1;
    } else if (rms_g < l3) {
        return 2;
    } else {
        return 3;
    }
}

DetectionResult detect_conditions(const float *spectrum_mag,
                                  std::size_t spectrum_bins,
                                  std::uint16_t step_count)
{
    DetectionResult res{};
    res.tremor_level      = 0;
    res.dyskinesia_level  = 0;
    res.fog_level         = 0;
    res.tremor_band_rms_g = 0.0f;
    res.dyskinesia_band_rms_g = 0.0f;
    res.step_rate_hz      = 0.0f;

    if (spectrum_bins == 0) {
        return res;
    }

    // Frequency resolution: fs / N
    const float df = SAMPLE_FREQUENCY_HZ / static_cast<float>(FFT_LENGTH);

    float tremor_power = 0.0f;
    float dysk_power   = 0.0f;
    std::size_t tremor_bins = 0;
    std::size_t dysk_bins   = 0;

    // k=0 is the DC component; skip it
    for (std::size_t k = 1; k < spectrum_bins; ++k) {
        const float f = df * static_cast<float>(k);
        const float mag = spectrum_mag[k];
        const float p   = mag * mag;

        if (f >= TREMOR_F_MIN_HZ && f < TREMOR_F_MAX_HZ) {
            tremor_power += p;
            ++tremor_bins;
        } else if (f >= DYSK_F_MIN_HZ && f < DYSK_F_MAX_HZ) {
            dysk_power += p;
            ++dysk_bins;
        }
    }

    if (tremor_bins > 0) {
        res.tremor_band_rms_g = std::sqrt(tremor_power / static_cast<float>(tremor_bins));
    }
    if (dysk_bins > 0) {
        res.dyskinesia_band_rms_g = std::sqrt(dysk_power / static_cast<float>(dysk_bins));
    }

    // Tremor / dyskinesia intensity classification
    // (thresholds can be tuned based on experimental data)
    res.tremor_level = classify_level(res.tremor_band_rms_g,
                                      TREMOR_LEVEL1_RMS_G,
                                      TREMOR_LEVEL2_RMS_G,
                                      TREMOR_LEVEL3_RMS_G);

    res.dyskinesia_level = classify_level(res.dyskinesia_band_rms_g,
                                          DYSK_LEVEL1_RMS_G,
                                          DYSK_LEVEL2_RMS_G,
                                          DYSK_LEVEL3_RMS_G);

    // Step rate estimate: steps / window time
    res.step_rate_hz = static_cast<float>(step_count) / WINDOW_SECONDS;

    // Determine whether current window corresponds to "walking"
    const bool is_walking = (res.step_rate_hz >= 0.5f); // >0.5 Hz considered walking

    if (is_walking) {
        ++g_consecutive_walking_windows;
        res.fog_level = 0; // If the user is walking, this window is not FOG
    } else {
        // If there were several consecutive walking windows and this one suddenly
        // shows no gait, mark it as a FOG event
        if (g_consecutive_walking_windows >= FOG_MIN_WALKING_WINDOWS) {
            res.fog_level = 1;
        } else {
            res.fog_level = 0;
        }

        // Reset consecutive walking window counter regardless
        g_consecutive_walking_windows = 0;
    }

    return res;
}
