#pragma once

class AllpassFilter {
public:
    AllpassFilter(float initCutoff, float initReso);
    ~AllpassFilter();
    void processBlock(float* input_buffer, float* output_buffer, int n_frames);
    void setCutoff(float frequency) { cutoff = frequency; }
    void setResonance(float resonance) { reso = resonance; }
    void setSampleRate(float sr) { sampleRate = sr; updateCoefficients(); }
    void updateCoefficients();
    void reset() { x1 = 0.f; x2 = 0.f; }

private:
    float cutoff; // In Hz [20 Hz, 22050 Hz]
    float reso;   // Resonance/Q factor
    float sampleRate = 44100.f;

    // Biquad filter coefficients
    float a1 = 0.f, a2 = 0.f, b0 = 0.f, b1 = 0.f, b2 = 0.f;

    // State variables for Direct Form II Transposed
    float x1 = 0.f, x2 = 0.f;
};