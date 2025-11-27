#include "visage_ui/frame.h"
#include  "juce_audio_processors/juce_audio_processors.h"
#include "knob.h"

// Container to hold the knob and display text

template<typename T>
class KnobContainer : public visage::Frame {
public:
    KnobContainer(juce::AudioProcessorValueTreeState& state, const juce::String& paramID, float logical_pixel_width);
    ~KnobContainer();
    void draw(visage::Canvas& canvas) override;
    float getTextBottom() { return text_frame->bottom();}

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::AudioParameterFloat* param = nullptr;
    juce::String paramID;
    std::unique_ptr<visage::Frame> text_frame;
    std::unique_ptr<Knob<T>> knob;
};