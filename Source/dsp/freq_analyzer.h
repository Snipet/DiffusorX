#pragma once
#include "kiss_fftr.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class FreqAnalyzer {
public:
    FreqAnalyzer(int bufferSize);
    ~FreqAnalyzer();

    void processBlock(const float* input, int numSamples);
    int getBufferSize() const { return buffer_size; }
    float* getDBSpectrum() const { return output_buffer; }

private:
    float* input_buffer;
    float* output_buffer;
    float* fft_input_buffer;
    kiss_fft_cpx* fft_output;
    int buffer_size;
    size_t write_index;
    kiss_fftr_cfg fft_cfg;

    inline float hannWindow(int n, int N) {
        return 0.5f * (1.0f - cosf((2.0f * M_PI * n) / (N - 1)));
    }

    void processBuffer();

};