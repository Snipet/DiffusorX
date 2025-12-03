#include "audio_processor.h"

DiffusorXMonoAudioProcessor::DiffusorXMonoAudioProcessor(juce::AudioProcessorValueTreeState& state)
    : sample_rate(44100.0f), active_stages(0), cutoff_hz(1000.0f), resonance(0.5f), apvts(state), oversampling(1, DIFFUSORX_JUCE_OVERSAMPLING_PARAM, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true)
{
    // Initialize allpass filters
    for(int i = 0; i < 32; ++i) {
        allpass_filters.push_back(new AllpassFilter(cutoff_hz, resonance));
    }

    mono_buffer = nullptr;
}

DiffusorXMonoAudioProcessor::~DiffusorXMonoAudioProcessor()
{
    for(auto* filter : allpass_filters) {
        delete filter;
    }
}

void DiffusorXMonoAudioProcessor::updateAllpassParameters()
{
    for(auto* filter : allpass_filters) {
        filter->setCutoff(cutoff_hz);
        filter->setResonance(resonance);
        filter->updateCoefficients();
    }
}

void DiffusorXMonoAudioProcessor::setDiffuseStages(int stages)
{
    active_stages = stages;
}

void DiffusorXMonoAudioProcessor::prepareToPlay(float sr, int n_frames){
    sample_rate = sr * static_cast<float>(DIFFUSORX_OVERSAMPLING);
    max_n_frames = n_frames * DIFFUSORX_OVERSAMPLING;
    for(auto* filter : allpass_filters) {
        filter->setSampleRate(sample_rate);
        filter->reset();
    }

    // Allocate mono buffer
    if(mono_buffer != nullptr) {
        delete[] mono_buffer;
    }
    mono_buffer = new float[max_n_frames];

    oversampling.initProcessing(n_frames);
}

void DiffusorXMonoAudioProcessor::processBlock(juce::dsp::AudioBlock<float>& buffer)
{

    const float plugin_frequency = *apvts.getRawParameterValue("frequency");
    const float plugin_resonance =*apvts.getRawParameterValue("resonance") * 2.5f + 0.3;
    if(plugin_frequency != cutoff_hz || plugin_resonance != resonance){
        cutoff_hz = plugin_frequency;
        resonance = plugin_resonance;
        updateAllpassParameters();
    }
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto oversampledBlock = oversampling.processSamplesUp(block);
    auto* oversampled_buffer = oversampledBlock.getChannelPointer(0);
    size_t n_frames = oversampledBlock.getNumSamples();

    // Copy input to mono_buffer
    memcpy(mono_buffer, oversampled_buffer, sizeof(float) * n_frames);

    // Apply active allpass filters
    for(int i = 0; i < 32; ++i) {
        allpass_filters[i]->processBlock(mono_buffer, mono_buffer, n_frames);
    }

    // Copy processed mono_buffer to output_buffer
    memcpy(oversampled_buffer, mono_buffer, sizeof(float) * n_frames);

    oversampling.processSamplesDown(block);
}