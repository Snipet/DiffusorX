#pragma once

class AllpassFilter {
public:
    AllpassFilter(float initCutoff, float initReso);
    ~AllpassFilter();
    void processBlock(float* input_buffer, float* output_buffer, int n_frames);
    void setCutoff(float frequency) { cutoff = frequency; }
    void setResonance(float resonance) { reso = resonance; }

private:
    float cutoff; // In Hz [20 Hz, 22050 Hz]
    float reso;
    float a1, a2, b0, b1, b2;
    float x1, x2; // Previous input samples
    float z1 = 0.f, z2 = 0.f; // Delay elements


    // Functions
    void updateCoefficients();
};