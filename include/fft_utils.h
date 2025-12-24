#ifndef FFT_UTILS_H
#define FFT_UTILS_H

#include <cstddef>
#include <cstdint>

// Compute magnitude from 3-axis acceleration (units: g)
void compute_magnitude(const float *ax,
                       const float *ay,
                       const float *az,
                        std::size_t n,
                       float *mag_out);

// Simple step-count estimator using the magnitude array
// Returns the estimated step count within the window
std::uint16_t estimate_step_count(const float *mag,
                                    std::size_t n);

// Simple DFT (O(N^2)) producing a single-sided magnitude spectrum
// time_data: input time-domain data (magnitude) — only first time_samples used
// time_samples: number of valid samples (e.g., 156)
// fft_length: DFT length (e.g., 256). If > time_samples, the remainder is zero-padded
// mag_out: output single-sided magnitude spectrum (length at least fft_length/2)
void compute_dft_magnitude(const float *time_data,
                            std::size_t time_samples,
                           float *mag_out,
                            std::size_t fft_length);

#endif // FFT_UTILS_H
