# RTES Challenge F25 – Tremor / Dyskinesia / FOG Detection
_B-L475E-IOT01A · mbed · PlatformIO_

## 1. Overview

This project implements my solution to **RTES Challenge F25** on the  
**ST B-L475E-IOT01A Discovery kit**.

The firmware runs a small on-board pipeline:

- sample 3-axis acceleration from the LSM6DSL (±2 g, 52 Hz);
- buffer data into 3 s windows;
- compute acceleration magnitude and estimate step count;
- run a 256-point DFT/FFT and extract band energy in
  - 3–5 Hz (tremor band),
  - 5–7 Hz (dyskinesia band);
- convert band RMS values to discrete levels `tremor_level` / `dyskinesia_level` ∈ {0,1,2,3};
- detect a coarse Freezing of Gait (FOG) flag from recent step history;
- expose the result via serial log, Teleplot, BLE GATT, and LEDs.

All computations are done on the MCU; the PC is only used for flashing and monitoring.

---

## 2. Hardware and software

**Board**

- ST **B-L475E-IOT01A** (STM32L475VGT6)
- IMU: **LSM6DSL** accelerometer
- Peripherals:
  - `LED1` – tremor indicator
  - `LED2` – dyskinesia indicator

**Toolchain**

- VSCode + PlatformIO
- Platform: `ststm32`, board: `disco_l475vg_iot01a`
- Framework: `mbed` (6.x)
- On-board programmer: ST-LINK
- Optional tools:
  - VSCode **Teleplot** extension
  - Phone app **nRF Connect** for BLE testing

---

## 3. Directory layout

Headers are under `include/`, sources under `src/`:

```text
RTES-F25/
├── include/
│   ├── ble_service.h      // BLE GATT wrapper
│   ├── config.h           // sampling, FFT, thresholds
│   ├── detector.h         // tremor/dysk/FOG decision logic
│   ├── fft_utils.h        // magnitude, FFT, step counter
│   └── lsm6dsl_driver.h   // minimal LSM6DSL driver
├── src/
│   ├── ble_service.cpp
│   ├── detector.cpp
│   ├── fft_utils.cpp
│   ├── lsm6dsl_driver.cpp
│   └── main.cpp           // main loop, LEDs, serial, Teleplot
├── mbed_app.json
├── platformio.ini
└── README.md
```

Module summary:

- **config.h** – sampling settings, FFT length, frequency bands and thresholds.
- **lsm6dsl_driver** – I²C configuration and `lsm6dsl_read_accel(ax, ay, az)` in g.
- **fft_utils** – magnitude computation, simple step counter, DFT magnitude.
- **detector** – integrates band energy, computes RMS and returns a `DetectionResult`
  with step count, band RMS values and the tremor/dysk/FOG levels.
- **ble_service** – custom BLE service:
  - service UUID `0xF250`
  - 3× `uint8_t` characteristics (`0xF251`, `0xF252`, `0xF253`) for tremor, dyskinesia and FOG.
- **main.cpp** – owns the buffers, runs the 3 s pipeline, drives LEDs, prints logs,
  sends Teleplot lines and calls `ble_service_update()`.

---

## 4. Algorithm and runtime behaviour

For each 3 s window (`SAMPLES_PER_WINDOW` samples):

1. store `ax/ay/az` into local buffers;
2. compute magnitude `|a|` and run peak-based step counting;
3. zero-pad to `FFT_LENGTH` and compute the magnitude spectrum;
4. integrate the tremor band (3–5 Hz) and dyskinesia band (5–7 Hz);
5. convert band RMS into levels 0–3 using thresholds from `config.h`;
6. update FOG based on recent windows and current step count;
7. update LEDs, BLE characteristics and serial output.

Example serial line:

```text
[WIN] steps=8, tremor_rms=0.0417 g, dysk_rms=0.0685 g, tremor_lvl=1, dysk_lvl=1, fog=0
```

The firmware also prints Teleplot-style lines:

```text
>steps:8
>tremor_rms:0.0417
>dysk_rms:0.0685
>tremor_lvl:1
>dysk_lvl:1
>fog:0
```

Teleplot can plot these variables in real time while the board is worn at the waist.

---

## 5. BLE interface and LEDs

BLE configuration:

- device name: `RTES-F25`
- service UUID: `0xF250`
- characteristics:
  - `0xF251` – tremor level (`uint8_t`, read + notify)
  - `0xF252` – dyskinesia level (`uint8_t`, read + notify)
  - `0xF253` – FOG flag (`uint8_t`, read + notify)

In **nRF Connect**, I connect to `RTES-F25`, open service `F250`, enable
notifications on all three characteristics, and observe one update per window.

LED behaviour:

- `LED1` is on when `tremor_level > 0`, off otherwise;
- `LED2` is on when `dyskinesia_level > 0`, off otherwise;
- when `fog_level > 0`, both LEDs blink together briefly.

---

## 6. Build and run

Steps:

1. Install VSCode, PlatformIO and the ST-LINK driver.
2. Open this folder in VSCode as a PlatformIO project.
3. Connect the board through the ST-LINK USB port (CN7).
4. Select environment `disco_l475vg_iot01a`.
5. Build and upload the firmware.
6. Open a serial monitor at 115200 baud. Every 3 s one `[WIN]` line and several
   Teleplot lines should appear.
7. Optionally connect Teleplot to the same COM port or test BLE with nRF Connect.


