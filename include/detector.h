#ifndef DETECTOR_H
#define DETECTOR_H

#include <cstdint>
#include <cstddef>

#include "config.h"

// Structure used to pass detection results between main and BLE layers
struct DetectionResult {
    std::uint8_t tremor_level;      // 0 = none, 1..3 = intensity
    std::uint8_t dyskinesia_level;  // 0 = none, 1..3 = intensity
    std::uint8_t fog_level;         // 0 = none, 1 = FOG detected

    float tremor_band_rms_g;        // RMS in 3–5 Hz band
    float dyskinesia_band_rms_g;    // RMS in 5–7 Hz band
    float step_rate_hz;             // estimated step rate in the current window
};

// Detect tremor / dyskinesia / FOG from a single-sided magnitude spectrum and step count
DetectionResult detect_conditions(const float *spectrum_mag,
                                    std::size_t spectrum_bins,
                                    std::uint16_t step_count);

#endif // DETECTOR_H
