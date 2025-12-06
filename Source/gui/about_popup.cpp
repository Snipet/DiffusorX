#include "about_popup.h"
#include "embedded/plugin_icons.h"
#include "embedded/plugin_fonts.h"

#define FUNK_AUDIO_LOGO_SIZE 50

AboutTextFrame::AboutTextFrame(const visage::String& str, const visage::Font::Justification& justification) : str(str), justification(justification){

}

AboutTextFrame::~AboutTextFrame(){

}

void AboutTextFrame::draw(visage::Canvas& canvas){
    canvas.setColor(0xffeeeeee);
    visage::Font font(height() * 0.6, resources::fonts::Roboto_Condensed_Regular_ttf);
    canvas.text(str, font, justification, 0.f, 0.f, width(), height());
}


AboutPopupFrame::AboutPopupFrame(): funk_audio_logo_svg(resources::icons::funk_audio_logo_rendered_svg.data, resources::icons::funk_audio_logo_rendered_svg.size){
    this->layout().setPaddingBottom(visage::Dimension::logicalPixels(5));
    logo_frame = std::make_unique<visage::Frame>();
    version_frame = std::make_unique<AboutTextFrame>("Version: 0.5.0", visage::Font::Justification::kCenter);
    dsp_frame = std::make_unique<AboutTextFrame>("DSP: Sean Funk");
    graphics_frame = std::make_unique<AboutTextFrame>("Graphics: Sean Funk");
    libraries_frame = std::make_unique<AboutTextFrame>("Libraries:");
    kissfft_frame = std::make_unique<AboutTextFrame>("   - kissfft");
    visage_frame = std::make_unique<AboutTextFrame>("   - visage");
    juce_frame = std::make_unique<AboutTextFrame>("   - JUCE");
    funk_audio_logo_container = std::make_unique<visage::Frame>();
    funk_audio_logo = std::make_unique<visage::Frame>();
    logo_frame->layout().setDimensions(visage::Dimension::widthPercent(100), visage::Dimension::logicalPixels(40));
    dsp_frame->layout().setDimensions(visage::Dimension::widthPercent(100), visage::Dimension::logicalPixels(20));
    graphics_frame->layout().setDimensions(visage::Dimension::widthPercent(100), visage::Dimension::logicalPixels(20));
    libraries_frame->layout().setDimensions(visage::Dimension::widthPercent(100), visage::Dimension::logicalPixels(20));
    kissfft_frame->layout().setDimensions(visage::Dimension::widthPercent(100), visage::Dimension::logicalPixels(20));
    visage_frame->layout().setDimensions(visage::Dimension::widthPercent(100), visage::Dimension::logicalPixels(20));
    juce_frame->layout().setDimensions(visage::Dimension::widthPercent(100), visage::Dimension::logicalPixels(20));
    funk_audio_logo_container->layout().setDimensions(visage::Dimension::widthPercent(100), visage::Dimension::logicalPixels(FUNK_AUDIO_LOGO_SIZE));
    funk_audio_logo_container->layout().setFlex(true);
    funk_audio_logo_container->layout().setFlexRows(true);
    funk_audio_logo_container->layout().setFlexItemAlignment(visage::Layout::ItemAlignment::Center);
    funk_audio_logo->layout().setDimensions(visage::Dimension::logicalPixels(FUNK_AUDIO_LOGO_SIZE), visage::Dimension::logicalPixels(FUNK_AUDIO_LOGO_SIZE));

    version_frame->layout().setDimensions(visage::Dimension::widthPercent(100), visage::Dimension::logicalPixels(15));
    version_frame->layout().setMarginBottom(visage::Dimension::logicalPixels(20));
    funk_audio_logo_svg.setDimensions(FUNK_AUDIO_LOGO_SIZE, FUNK_AUDIO_LOGO_SIZE);


    logo_frame->onDraw() = [f = logo_frame.get(), this](visage::Canvas& canvas){

        canvas.setColor(0xffeeeeee);
        canvas.svg(resources::icons::logo_center_svg, 0.f, 0.f, f->width(), f->height());
    };

    funk_audio_logo->onDraw() = [f = funk_audio_logo.get(), this](visage::Canvas& canvas){
        canvas.setColor(0xffffffff);
        canvas.svg(funk_audio_logo_svg, 0.f, 0.f, f->width(), f->height());
    };

    // version_frame->onDraw() = [f = version_frame.get(), this](visage::Canvas& canvas){

    //     canvas.setColor(0xffeeeeee);
    //     visage::Font font(f->height() * 0.6, resources::fonts::Roboto_Condensed_Regular_ttf);

    //     canvas.text("Version 0.5.0", font, visage::Font::Justification::kCenter, 0.f, 0.f, f->width(), f->height());
    // };
    
    // dsp_frame->onDraw() = [f = dsp_frame.get(), this](visage::Canvas& canvas){

    //     canvas.setColor(0xffeeeeee);
    //     visage::Font font(f->height() * 0.6, resources::fonts::Roboto_Condensed_Regular_ttf);
    //     canvas.text("DSP: Sean Funk", font, visage::Font::Justification::kLeft, 0.f, 0.f, f->width(), f->height());
    // };

    // graphics_frame->onDraw() = [f = graphics_frame.get(), this](visage::Canvas& canvas){

    //     canvas.setColor(0xffeeeeee);
    //     visage::Font font(f->height() * 0.6, resources::fonts::Roboto_Condensed_Regular_ttf);
    //     canvas.text("Graphics: Sean Funk", font, visage::Font::Justification::kLeft, 0.f, 0.f, f->width(), f->height());
    // };

    layout().setFlex(true);
    layout().setPaddingLeft(visage::Dimension::logicalPixels(10));
    layout().setPaddingRight(visage::Dimension::logicalPixels(10));
    layout().setPaddingBottom(visage::Dimension::logicalPixels(8));
    layout().setPaddingTop(visage::Dimension::logicalPixels(4));
    layout().setFlexRows(true);

    funk_audio_logo_container->addChild(funk_audio_logo.get());

    this->addChild(logo_frame.get());
    this->addChild(version_frame.get());
    this->addChild(dsp_frame.get());
    this->addChild(graphics_frame.get());
    this->addChild(libraries_frame.get());
    this->addChild(kissfft_frame.get());
    this->addChild(visage_frame.get());
    this->addChild(juce_frame.get());
    this->addChild(funk_audio_logo_container.get());
}

AboutPopupFrame::~AboutPopupFrame(){

}

void AboutPopupFrame::draw(visage::Canvas& canvas){

    canvas.setColor(0xff151515);
    canvas.roundedRectangle(0, 0, width(), height(), width() * 0.04);
}