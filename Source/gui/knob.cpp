#include "knob.h"
#include "embedded/plugin_shaders.h"
#include "embedded/plugin_fonts.h"
#include <cassert>

using namespace visage::dimension;

template <typename T>
Knob<T>::Knob(juce::AudioProcessorValueTreeState& state, const juce::String& paramID) : background_shader(resources::shaders::vs_shader_quad, resources::shaders::fs_knob_background2, visage::BlendMode::Alpha), apvts(state), paramID(paramID) {
    lasty = -1.f;
    ring_animation_time = 40.f; // ms
    animating = false;
    last_ms = visage::time::milliseconds();
    ring_thickness = 0.f;
    animation_growing = false;

    param = dynamic_cast<T*>(apvts.getParameter(paramID));
    jassert(param);

    norm_value = apvts.getRawParameterValue(paramID)->load();
}
template<typename T>
Knob<T>::~Knob() {
}

template<typename T>
void Knob<T>::draw(visage::Canvas& canvas) {
    float p_value = apvts.getRawParameterValue(paramID)->load();
    norm_value = param->convertTo0to1(p_value);
    unsigned int p_int = static_cast<unsigned int>(norm_value * 255.f);
    float animation_value = updateAnimation();
    unsigned int thickness = static_cast<unsigned int>(animation_value * 255.f);
    unsigned int color_input = p_int;
    color_input = color_input | (thickness << 8);
    canvas.setColor(color_input);
    canvas.shader(&background_shader, 0.f, 0.f, width(), height());

    juce::String param_string = param->getCurrentValueAsText();
    int length = param_string.length();
    float shrink = 0.f;
    if (length > 4){
        shrink = (length - 4) * 0.01;
    }

    float base_font_size = width() * (0.17f - shrink); // If width is 100 logical pixels, font size is 10 logical pixels
    const visage::Font knob_font(base_font_size, resources::fonts::Roboto_Condensed_Regular_ttf);
    unsigned int text_color = 0xffffffff;
    char alpha = 190 + animation_value * (255 - 190);
    text_color = (text_color & (alpha << 24)) | 0x00ffffff;
    canvas.setColor(text_color);
    canvas.text(param_string.toStdString(), knob_font, visage::Font::kCenter, 0.f, 0.f, width(), height());


    redraw();
}

template<typename T>
void Knob<T>::resized() {
}


template<typename T>
void Knob<T>::startAnimation(bool grow) {
    animation_growing = grow;
    last_ms = visage::time::milliseconds();
    ring_thickness = grow ? 0.f : ring_thickness;
    animating = true;
}

template<typename T>
float Knob<T>::updateAnimation()
{
    long long ms = visage::time::milliseconds();
    float delta = float(ms - last_ms) / ring_animation_time;
    ring_thickness = animation_growing ? std::min(ring_thickness + delta, 1.f) : std::max(ring_thickness - delta, 0.f);
    last_ms = ms;
    if (ring_thickness >= 1.f && animation_growing)
    {
        animating = false;
    }

    if (ring_thickness == 0.f  && !animation_growing)
    {
        animating = false;
    }

    return ring_thickness;
}

template<typename T>
void Knob<T>::mouseDown(const visage::MouseEvent& e) {
    lasty = e.position.y;
    // if (e.repeatClickCount() == 2 && e.isLeftButton())
    // {
    //     float paramDefault = param_ptr->GetDefault(true);
    //     norm_value = paramDefault;
    //     plug_state.plug->SetParameterValue(param, paramDefault);
    // }

    // if (e.isRightButton())
    // {
    //     visage::PopupMenu menu;
    //     menu.addOption(0, "Reset to Default");
    //     menu.addOption(1, "Learn MIDI");
    //     menu.show(this->topParentFrame(), e.window_position);
    // }

}
template<typename T>
void Knob<T>::mouseEnter(const visage::MouseEvent& e) {
    startAnimation(true);
    redraw();
}

template<typename T>
void Knob<T>::mouseExit(const visage::MouseEvent& e) { startAnimation(false); redraw();}

template<typename T>
void Knob<T>::mouseDrag(const visage::MouseEvent& e) {
    if (lasty == -1.f)
    {
        lasty = e.position.y;
    }
    float dy = lasty - e.position.y;
    norm_value = norm_value += dy * (e.isCtrlDown() ? 0.002 : 0.008);
    norm_value = std::min(norm_value, 1.f);
    norm_value = std::max(norm_value, 0.f);
    lasty = e.position.y;

    // Set plugin parameter [todo]
    param->setValueNotifyingHost(norm_value);

    redraw();
}

template class Knob<juce::AudioParameterFloat>;
template class Knob<juce::AudioParameterInt>;