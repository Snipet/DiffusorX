#include "allpass.h"
#include <cmath>

AllpassFilter::AllpassFilter(float initCutoff, float initReso) : cutoff(initCutoff), reso(initReso) {
    a1 = 0.f;
    a2 = 0.f;
    b0 = 0.f;
    b1 = 0.f;
    b2 = 0.f;
    updateCoefficients();
}

AllpassFilter::~AllpassFilter() {
    // Destructor logic
}

void AllpassFilter::updateCoefficients() {
    // Update filter coefficients based on cutoff and resonance
    // Placeholder implementation
    float omega_c = 2.f * 3.14159265359f * cutoff / 44100.f; // Assuming sample rate of 44100 Hz
    float alpha = sin(omega_c) / (2.f * reso);

    float a0 = 1.f + alpha;
    a1 = -2.f * cos(omega_c) / a0;
    a2 = (1.f - alpha) / a0;
    b0 = a2;
    b1 = a1;
    b2 = 1.f;
}

void AllpassFilter::processBlock(float* input_buffer, float* output_buffer, int n_frames){
    for(int n = 0; n < n_frames; ++n) {
        // Direct Form II Transposed implementation
        float input = input_buffer[n];
        float output = b0 * input + b1 * x1 + b2 * x2 - a1 * z1 - a2 * z2;

        // Update delay elements
        x2 = x1;
        x1 = input;

        z2 = z1;
        z1 = output;

        output_buffer[n] = output;
    }
}