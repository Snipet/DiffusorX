#include "freq_analyzer.h"

FreqAnalyzer::FreqAnalyzer(int bufferSize)
    : buffer_size(bufferSize)
{
    input_buffer = new float[buffer_size];
    fft_input_buffer = new float[buffer_size];
    output_buffer = new float[buffer_size / 2 + 1];
    fft_output = new kiss_fft_cpx[buffer_size / 2 + 1];
    fft_cfg = kiss_fftr_alloc(buffer_size, 0, nullptr, nullptr);
    write_index = 0;
}

FreqAnalyzer::~FreqAnalyzer()
{
    if(input_buffer){
        delete[] input_buffer;
    }

    if(fft_input_buffer){
        delete[] fft_input_buffer;
    }

    if(output_buffer){
        delete[] output_buffer;
    }

    if(fft_output){
        delete[] fft_output;
    }

    if(fft_cfg) {
        kiss_fft_free(fft_cfg);
    }
}

void FreqAnalyzer::processBlock(const float* input, int numSamples)
{
    for(int i = 0; i < numSamples; ++i) {
        input_buffer[write_index] = input[i];
        write_index = (write_index + 1) % buffer_size;

        if(write_index == 0) {
            processBuffer();
        }
    }
}

void FreqAnalyzer::processBuffer()
{

    // Copy input buffer to FFT input buffer 
    for(int n = 0; n < buffer_size; ++n) {
        fft_input_buffer[n] = input_buffer[n];
    }

    // Apply Hann window
    for(int n = 0; n < buffer_size; ++n) {
        fft_input_buffer[n] = fft_input_buffer[n] * hannWindow(n, buffer_size);
    }

    // Perform FFT
    kiss_fftr(fft_cfg, fft_input_buffer, fft_output);
    const float scale = 1.0f / (static_cast<float>(buffer_size) / 2.f);
    for(int k = 0; k < buffer_size / 2 + 1; ++k) {
        output_buffer[k] = sqrtf(fft_output[k].r * fft_output[k].r + fft_output[k].i * fft_output[k].i) * scale;
    }

    // Convert to DB scale
    for(int k = 0; k < buffer_size / 2 + 1; ++k) {
        if(output_buffer[k] < 0.00001f) {
            output_buffer[k] = 0.00001f; // Avoid log(0)
        }
        output_buffer[k] = 20.0f * log10f(output_buffer[k]);

        // Normalize to 0 dB = 1.0, -60 dB = 0.0
        output_buffer[k] = (output_buffer[k] + 60.0f) / 60.0f;
        if(output_buffer[k] < 0.0f) {
            output_buffer[k] = 0.0f;
        }
    }
}

