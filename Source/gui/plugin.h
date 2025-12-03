#pragma once

#include "visage_ui/frame.h"
#include "visualizer.h"
#include "knob.h"
#include "PluginProcessor.h"
#include "knob_container.h"

class PluginUIFrame : public visage::Frame {
public:
    PluginUIFrame(DiffusorXAudioProcessor& p);
    ~PluginUIFrame();
    void draw(visage::Canvas& canvas) override;
    void resized() override;

private:
    // Container frames
    std::unique_ptr<visage::Frame> visualizer_frame_container;
    std::unique_ptr<visage::Frame> parameter_frame_container;
    std::unique_ptr<visage::Frame> header_frame_container;
    std::unique_ptr<visage::Frame> main_knobs_container;

    // Content frames
    std::unique_ptr<VisualizerFrame> visualizer_frame_;

    // Knobs
    std::unique_ptr<KnobContainer<juce::AudioParameterFloat>> diffuse_freq_knob;
    std::unique_ptr<KnobContainer<juce::AudioParameterInt>> diffuse_stages_knob;
    std::unique_ptr<KnobContainer<juce::AudioParameterFloat>> diffuse_reso_knob;

    // Shaders
    visage::Shader knob_background_shader;

    DiffusorXAudioProcessor& audio_processor;
};