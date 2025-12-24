#ifndef CONFIG_H
#define CONFIG_H

#include <cstddef>

// Sampling frequency and window length (fixed by the challenge)
static constexpr float SAMPLE_FREQUENCY_HZ = 52.0f;   // 52 Hz ODR of LSM6DSL
static constexpr float WINDOW_SECONDS      = 3.0f;    // 3 s processing window

// 52 * 3 ≈ 156 samples per window
static constexpr std::size_t SAMPLES_PER_WINDOW =
    static_cast<std::size_t>(SAMPLE_FREQUENCY_HZ * WINDOW_SECONDS + 0.5f);

// Use a 256-point DFT/FFT for ~0.2 Hz frequency resolution.
// The 156 time samples are zero-padded up to FFT_LENGTH.
static constexpr std::size_t FFT_LENGTH = 256;

// LSM6DSL sensitivity at ±2 g: 0.061 mg/LSB ≈ 0.000061 g/LSB
static constexpr float ACC_G_PER_LSB = 0.000061f;

// ------------------------------------------------------------
// Step detection (waist-worn, based on acceleration magnitude)
// ------------------------------------------------------------

// At the waist, the magnitude is around 1 g at rest.
// Normal walking produces peaks around 1.2–1.4 g.
static constexpr float STEP_MAG_THRESHOLD_G = 1.18f;

// Minimum samples between two detected steps.
// 20 samples @ 52 Hz ≈ 0.38 s (upper bound on step frequency).
static constexpr std::size_t STEP_MIN_INTERVAL = 20;

// ------------------------------------------------------------
// Tremor / dyskinesia frequency bands (Hz)
// ------------------------------------------------------------

static constexpr float TREMOR_F_MIN_HZ = 3.0f;
static constexpr float TREMOR_F_MAX_HZ = 5.0f;
static constexpr float DYSK_F_MIN_HZ   = 5.0f;
static constexpr float DYSK_F_MAX_HZ   = 7.0f;

// ------------------------------------------------------------
// Tremor / dyskinesia intensity thresholds (band RMS, in g)
// ------------------------------------------------------------
//
// Tuned with test data when the board is worn at the waist.
// Typical values:
//   rest:          ~0.015–0.025 g
//   normal walking ~0.03–0.07 g
//   strong shaking  >0.07 g

static constexpr float TREMOR_LEVEL1_RMS_G = 0.03f;  // noticeable tremor
static constexpr float TREMOR_LEVEL2_RMS_G = 0.07f;  // clear tremor
static constexpr float TREMOR_LEVEL3_RMS_G = 0.12f;  // strong tremor

static constexpr float DYSK_LEVEL1_RMS_G   = 0.03f;
static constexpr float DYSK_LEVEL2_RMS_G   = 0.07f;
static constexpr float DYSK_LEVEL3_RMS_G   = 0.12f;

// ------------------------------------------------------------
// FOG decision
// ------------------------------------------------------------

// Minimum consecutive "walking" windows before a sudden stop is
// considered a candidate FOG event.
static constexpr std::size_t FOG_MIN_WALKING_WINDOWS = 2;

#endif // CONFIG_H
