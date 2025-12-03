#pragma once
#include <vector>
#include "allpass.h"
#include  "juce_audio_processors/juce_audio_processors.h"
#include "juce_dsp/juce_dsp.h"

#define DIFFUSORX_OVERSAMPLING 2
#define DIFFUSORX_JUCE_OVERSAMPLING_PARAM 0

class DiffusorXMonoAudioProcessor {
public:
    DiffusorXMonoAudioProcessor(juce::AudioProcessorValueTreeState& state);
    ~DiffusorXMonoAudioProcessor();

    void processBlock(juce::dsp::AudioBlock<float>& buffer);
    void prepareToPlay(float sr, int n_frames);
    void setDiffuseStages(int stages);
    void setCutoffFrequency(float frequency) { cutoff_hz = frequency; updateAllpassParameters(); }
    void setResonance(float reso) { resonance = reso; updateAllpassParameters(); }
    //void resetAllpassFilters();

private:
    float sample_rate;
    std::vector<AllpassFilter*> allpass_filters;
    int active_stages;
    float cutoff_hz;
    float resonance;
    int max_n_frames;
    float* mono_buffer;
    juce::AudioProcessorValueTreeState& apvts;
    juce::dsp::Oversampling<float> oversampling;
    void updateAllpassParameters();
};