#include "audio_processor.h"

DiffusorXMonoAudioProcessor::DiffusorXMonoAudioProcessor(juce::AudioProcessorValueTreeState& state)
    : sample_rate(44100.0f), active_stages(0), cutoff_hz(1000.0f), resonance(0.5f), apvts(state), oversampling(1, DIFFUSORX_JUCE_OVERSAMPLING_PARAM, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true)
{
    // Initialize allpass filters
    for(int i = 0; i < DIFFUSORX_MAX_STAGES; ++i) {
        allpass_filters.push_back(new AllpassFilter(cutoff_hz, resonance));
    }

    mono_buffer = nullptr;
    cutoff_hz_state = 0.f;
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

    cutoff_hz_state = cutoff_hz;
}

void DiffusorXMonoAudioProcessor::processBlock(juce::dsp::AudioBlock<float>& buffer)
{

    const float plugin_frequency = *apvts.getRawParameterValue("frequency");
    const float plugin_resonance =*apvts.getRawParameterValue("resonance") * 2.5f + 0.3;

    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto oversampledBlock = oversampling.processSamplesUp(block);
    auto* oversampled_buffer = oversampledBlock.getChannelPointer(0);
    size_t n_frames = oversampledBlock.getNumSamples();

    float g = 0.06 * static_cast<float>(n_frames) / 512.f;
    float cutoff = (plugin_frequency * g + cutoff_hz_state) / (g + 1.f);
    cutoff_hz_state = 2.f * cutoff - cutoff_hz_state;

    const int plugin_diffuse_stages = *apvts.getRawParameterValue("diffuse_stages");
    if(cutoff != cutoff_hz || plugin_resonance != resonance){
        cutoff_hz = cutoff;
        resonance = plugin_resonance;
        updateAllpassParameters();
    }

    // Copy input to mono_buffer
    memcpy(mono_buffer, oversampled_buffer, sizeof(float) * n_frames);

    // Apply active allpass filters
    for(int i = 0; i < plugin_diffuse_stages; ++i) {
        allpass_filters[i]->processBlock(mono_buffer, mono_buffer, n_frames);
    }

    // Copy processed mono_buffer to output_buffer
    memcpy(oversampled_buffer, mono_buffer, sizeof(float) * n_frames);

    oversampling.processSamplesDown(block);
}