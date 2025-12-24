#include "fft_utils.h"
#include "config.h"

#include <cmath>
#include <algorithm>

void compute_magnitude(const float *ax,
                       const float *ay,
                       const float *az,
                        std::size_t n,
                       float *mag_out)
{
    for (std::size_t i = 0; i < n; ++i) {
        const float x = ax[i];
        const float y = ay[i];
        const float z = az[i];
        mag_out[i] = std::sqrt(x * x + y * y + z * z);
    }
}

// Very simple threshold + minimum-interval step estimator
std::uint16_t estimate_step_count(const float *mag,
                                    std::size_t n)
{
    std::uint16_t steps = 0;
    std::size_t last_step_index = 0;
    bool first_step = true;

    for (std::size_t i = 0; i < n; ++i) {
        // Simple threshold: count a step when signal crosses from below to above threshold
        if (mag[i] > STEP_MAG_THRESHOLD_G) {
            if (first_step) {
                ++steps;
                first_step = false;
                last_step_index = i;
            } else {
                if (i >= last_step_index + STEP_MIN_INTERVAL) {
                    ++steps;
                    last_step_index = i;
                }
            }
        }
    }

    return steps;
}

void compute_dft_magnitude(const float *time_data,
                            std::size_t time_samples,
                           float *mag_out,
                            std::size_t fft_length)
{
    // Single-sided spectrum; only need 0 .. N/2
    const std::size_t half = fft_length / 2;

    for (std::size_t k = 0; k < half; ++k) {
        float real = 0.0f;
        float imag = 0.0f;

        for (std::size_t n = 0; n < time_samples; ++n) {
            const float angle = 2.0f * static_cast<float>(M_PI) * static_cast<float>(k * n)
                                / static_cast<float>(fft_length);
            const float sample = time_data[n];
            real += sample * std::cos(angle);
            imag -= sample * std::sin(angle);
        }

        // The remaining (fft_length - time_samples) are implicitly zero-padded,
        // no explicit handling is necessary here.

        const float mag = std::sqrt(real * real + imag * imag) / static_cast<float>(time_samples);
        mag_out[k] = mag;
    }
}
