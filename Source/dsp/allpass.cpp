#include "allpass.h"
#include <cmath>
#include <algorithm>

AllpassFilter::AllpassFilter(float initCutoff, float initReso) : cutoff(initCutoff), reso(initReso) {
    // Member variables are already initialized in the header with default values
    updateCoefficients();
}

AllpassFilter::~AllpassFilter() {
    // Destructor logic
}

void AllpassFilter::updateCoefficients() {
    // Update filter coefficients based on cutoff and resonance
    // Second-order allpass filter using RBJ cookbook formula

    // Clamp cutoff to valid range
    float freq = std::clamp(cutoff, 20.f, sampleRate * 0.5f);

    // Clamp resonance to avoid instability (Q factor)
    float Q = std::max(0.01f, reso);

    // Calculate angular frequency
    constexpr float PI = 3.14159265359f;
    float omega_c = 2.f * PI * freq / sampleRate;

    // Calculate alpha parameter
    float alpha = std::sin(omega_c) / (2.f * Q);

    // Calculate denominator coefficients (normalized by a0)
    float a0 = 1.f + alpha;
    a1 = -2.f * std::cos(omega_c) / a0;
    a2 = (1.f - alpha) / a0;

    // For allpass filter: numerator coefficients are reversed denominator
    b0 = a2;  // (1 - alpha) / a0
    b1 = a1;  // -2 * cos(omega_c) / a0
    b2 = 1.f; // a0 / a0 = 1
}

void AllpassFilter::processBlock(float* input_buffer, float* output_buffer, int n_frames){
    for(int n = 0; n < n_frames; ++n) {
        // Direct Form II Transposed implementation
        // Computes: y[n] = b0*x[n] + s1[n-1]
        // where s1[n] = s2[n-1] + b1*x[n] - a1*y[n]
        //       s2[n] = b2*x[n] - a2*y[n]

        float input = input_buffer[n];
        float output = b0 * input + x1;

        // Update state variables
        x1 = x2 + b1 * input - a1 * output;
        x2 = b2 * input - a2 * output;

        output_buffer[n] = output;
    }
}