#pragma once
#include "visage_ui/frame.h"
#include "visage_widgets/shader_quad.h"
#include  "juce_audio_processors/juce_audio_processors.h"

template <typename T>
class Knob : public visage::Frame {
public:
    Knob(juce::AudioProcessorValueTreeState& state, const juce::String& paramID);
    ~Knob();
    void draw(visage::Canvas& canvas) override;
    void resized() override;
    void mouseDrag(const visage::MouseEvent& e) override;
    void mouseDown(const visage::MouseEvent& e) override;
    void mouseEnter(const visage::MouseEvent& e) override;
    void mouseExit(const visage::MouseEvent& e) override;
    void startAnimation(bool grow);
    float updateAnimation();

private:
    visage::Shader background_shader;
    float norm_value; // [0, 1]
    float lasty;
    float ring_thickness;
    long long last_ms;
    float ring_animation_time;
    bool animating;
    bool animation_growing;
    juce::AudioProcessorValueTreeState& apvts;
    T* param = nullptr;
    juce::String paramID;
};