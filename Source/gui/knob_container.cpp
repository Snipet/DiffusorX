#include "knob_container.h"
#include "embedded/plugin_fonts.h"

template<typename T>
KnobContainer<T>::KnobContainer(juce::AudioProcessorValueTreeState& state, const juce::String& paramID, float logical_pixel_width) : apvts(state), paramID(paramID){
    knob = std::make_unique<Knob<T>>(state, paramID);
    knob->layout().setDimensions(visage::Dimension::logicalPixels(logical_pixel_width), visage::Dimension::logicalPixels(logical_pixel_width));
    
    text_frame = std::make_unique<visage::Frame>();
    text_frame->onDraw() = [f = text_frame.get(), this, paramID](visage::Canvas& canvas){
        // canvas.setColor(0xff00ff00);
        // canvas.fill(0.f, 0.f, f->width(), f->height());
        canvas.setColor(0xcc000000);
        juce::String str = apvts.getParameter(paramID)->getName(100).toStdString();
        float shrink = 0.f;
        if(str.length() > 5) {
            shrink = (str.length() - 5) * 0.02;
        }
        const visage::Font font(f->height() * (0.85 - shrink), resources::fonts::Roboto_Condensed_Regular_ttf);
        canvas.text(apvts.getParameter(paramID)->getName(100).toStdString(), font, visage::Font::kCenter, 0.f, 0.f, f->width(), f->height());
    };

    text_frame->layout().setDimensions(visage::Dimension::widthPercent(100.f), visage::Dimension::logicalPixels(20.f));

    this->layout().setFlex(true);
    this->layout().setFlexRows(true);
    this->layout().setFlexItemAlignment(visage::Layout::ItemAlignment::Center);

    addChild(knob.get());
    addChild(text_frame.get());
}

template<typename T>
KnobContainer<T>::~KnobContainer(){

}

template<typename T>
void KnobContainer<T>::draw(visage::Canvas& canvas){
    // // Test background
    // canvas.setColor(0xfffff000);
    // canvas.fill(0.f, 0.f, width(), height());
}

template class KnobContainer<juce::AudioParameterFloat>;
template class KnobContainer<juce::AudioParameterInt>;