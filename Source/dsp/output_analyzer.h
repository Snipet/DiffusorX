#pragma once
#include "audio_processor.h"
#include "juce_dsp/juce_dsp.h"
#include "kiss_fftr.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


class DiffusorXOutputAnalyzer {
public:
    DiffusorXOutputAnalyzer(size_t buffer_size, size_t fft_size, juce::AudioProcessorValueTreeState& state);
    ~DiffusorXOutputAnalyzer();
    void generateMagnitudeSpectrum();
    float* getMagnitudeSpectrum() {return output_magnitude_spectrum_ptr; }
    size_t getBufferSize() {return buffer_size; }
    size_t getFFTSize() {return fft_size; }

private:
    juce::AudioBuffer<float> buffer;
    juce::AudioProcessorValueTreeState& apvts;
    size_t buffer_size;
    size_t fft_size;
    float* output_magnitude_spectrum_ptr;
    float* magnitude_spectrum_1;
    float* magnitude_spectrum_2;
    float* fft_input;
    kiss_fft_cpx* fft_output;
    kiss_fftr_cfg fft_cfg;
    std::unique_ptr<DiffusorXMonoAudioProcessor> audio_processor;
    juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>,juce::dsp::IIR::Filter<float>,juce::dsp::IIR::Filter<float>,juce::dsp::IIR::Filter<float>> highpass_chain;

    inline float hannXWindow(int n, int N) {
        return (n < ((N-1) / 2)) ? 1.f : (0.5f * (1.0f - cosf((2.0f * M_PI * n) / (N - 1))));
    }

    inline float hannWindow(int n, int N) {
        return 0.5f * (1.0f - cosf((2.0f * M_PI * n) / (N - 1)));
    }
};