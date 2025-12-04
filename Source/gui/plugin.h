#pragma once

#include "visage_ui/frame.h"
#include "visualizer.h"
#include "knob.h"
#include "PluginProcessor.h"
#include "knob_container.h"
#include "about_popup.h"

class PluginUIFrame : public visage::Frame {
public:
    PluginUIFrame(DiffusorXAudioProcessor& p);
    ~PluginUIFrame();
    void draw(visage::Canvas& canvas) override;
    void resized() override;
    void mouseDown(const visage::MouseEvent& e) override;
    void openAboutMenu();
    void closeAboutMenu();

private:
    // Container frames
    std::unique_ptr<visage::Frame> visualizer_frame_container;
    std::unique_ptr<visage::Frame> parameter_frame_container;
    std::unique_ptr<visage::Frame> header_frame_container;
    std::unique_ptr<visage::Frame> main_knobs_container;
    std::unique_ptr<visage::Frame> main_plugin_frame;
    std::unique_ptr<AboutPopupFrame> about_popup_frame;

    // Content frames
    std::unique_ptr<VisualizerFrame> visualizer_frame_;

    // Knobs
    std::unique_ptr<KnobContainer<juce::AudioParameterFloat>> diffuse_freq_knob;
    std::unique_ptr<KnobContainer<juce::AudioParameterInt>> diffuse_stages_knob;
    std::unique_ptr<KnobContainer<juce::AudioParameterFloat>> diffuse_reso_knob;

    // Shaders
    visage::Shader knob_background_shader;
    long long last_about_menu_ms;

    bool about_menu_showing;

    DiffusorXAudioProcessor& audio_processor;
};