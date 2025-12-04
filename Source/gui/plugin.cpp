#include "plugin.h"
#include "embedded/plugin_shaders.h"
#include "embedded/plugin_icons.h"

PluginUIFrame::PluginUIFrame(DiffusorXAudioProcessor& p) : audio_processor(p),
                knob_background_shader(resources::shaders::vs_shader_quad, resources::shaders::fs_knobs_background, visage::BlendMode::Alpha)
{

    about_menu_showing = false;
    this->setReceiveChildMouseEvents(false);
    last_about_menu_ms = visage::time::milliseconds();

    visualizer_frame_ = std::make_unique<VisualizerFrame>(audio_processor);
    visualizer_frame_container = std::make_unique<visage::Frame>();
    parameter_frame_container = std::make_unique<visage::Frame>();
    header_frame_container = std::make_unique<visage::Frame>();
    main_knobs_container = std::make_unique<visage::Frame>();
    main_plugin_frame = std::make_unique<visage::Frame>();
    about_popup_frame = std::make_unique<AboutPopupFrame>();
    about_popup_frame->layout().setDimensions(visage::Dimension::widthPercent(50), visage::Dimension::heightPercent(80));
    about_popup_frame->layout().setMarginLeft(visage::Dimension::viewMaxPercent(25));
    about_popup_frame->layout().setMarginRight(visage::Dimension::viewMaxPercent(10));
    about_popup_frame->layout().setMarginTop(visage::Dimension::heightPercent(10));

    diffuse_freq_knob = std::make_unique<KnobContainer<juce::AudioParameterFloat>>(audio_processor.getAPVTS(), "frequency", 95);
    diffuse_stages_knob = std::make_unique<KnobContainer<juce::AudioParameterInt>>(audio_processor.getAPVTS(), "diffuse_stages", 110);
    diffuse_reso_knob = std::make_unique<KnobContainer<juce::AudioParameterFloat>>(audio_processor.getAPVTS(), "resonance", 95);
    //test_knob = std::make_unique<Knob>(audio_processor.getAPVTS(), "gain");
    main_plugin_frame->layout().setDimensions(visage::Dimension::widthPercent(100), visage::Dimension::heightPercent(100));

    // Set flex settings
    main_plugin_frame->layout().setFlex(true);
    //this->layout().setFlexItemAlignment(visage::Layout::ItemAlignment::Center);
    main_plugin_frame->layout().setFlexRows(true);
    //this->layout().setFlexGrow(1.f);

    visualizer_frame_container->layout().setFlexGrow(0.5f);
    visualizer_frame_container->layout().setPaddingTop(visage::Dimension::logicalPixels(10));
    visualizer_frame_container->layout().setPaddingBottom(visage::Dimension::logicalPixels(10));
    parameter_frame_container->layout().setFlexGrow(0.5f);
    visualizer_frame_container->layout().setDimensions(visage::Dimension::viewMaxPercent(100), visage::Dimension::logicalPixels(50));
    parameter_frame_container->layout().setDimensions(visage::Dimension::viewMaxPercent(100), visage::Dimension::logicalPixels(50));
    header_frame_container->layout().setDimensions(visage::Dimension::viewMaxPercent(100), visage::Dimension::logicalPixels(30));
    main_knobs_container->layout().setDimensions(visage::Dimension::viewMaxPercent(60), visage::Dimension::heightPercent(100));
    main_knobs_container->layout().setFlex(true);
    main_knobs_container->layout().setFlexRows(false);
    main_knobs_container->layout().setFlexItemAlignment(visage::Layout::ItemAlignment::Center);
    //main_knobs_container->layout().setFlexGrow(1.f);

    
    //parameter_frame_container->layout().setPadding(visage::Dimension::logicalPixels(15));
    parameter_frame_container->layout().setFlex(true);
    parameter_frame_container->layout().setFlexItemAlignment(visage::Layout::ItemAlignment::Center);

    //test_knob->layout().setDimensions(visage::Dimension::logicalPixels(100), visage::Dimension::logicalPixels(100));
    diffuse_freq_knob->layout().setDimensions(visage::Dimension::logicalPixels(95), visage::Dimension::logicalPixels(95 + 20));
    diffuse_stages_knob->layout().setDimensions(visage::Dimension::logicalPixels(120), visage::Dimension::logicalPixels(120 + 20));
    diffuse_reso_knob->layout().setDimensions(visage::Dimension::logicalPixels(95), visage::Dimension::logicalPixels(95 + 20));

    diffuse_freq_knob->layout().setFlexGrow(1.f);
    diffuse_stages_knob->layout().setFlexGrow(1.f);
    diffuse_reso_knob->layout().setFlexGrow(1.f);


    visualizer_frame_container->onDraw() = [f = visualizer_frame_container.get()](visage::Canvas& canvas){
        canvas.setColor(0xff333333);
        canvas.fill(0, 0, f->width(), f->height());
    };
    

    parameter_frame_container->onDraw() = [f = parameter_frame_container.get(), this](visage::Canvas& canvas){
        canvas.setColor(0xffeeeeee);
        canvas.fill(0, 0, f->width(), f->height());
        canvas.setColor(0xffffffff);
        //canvas.shader(&knob_background_shader, 0.f, -f->width() / 2.f, f->width(), f->width());
    };

    header_frame_container->onDraw() = [f = header_frame_container.get(), this](visage::Canvas& canvas){
        canvas.setColor(0xff222222);
        canvas.fill(0, 0, f->width(), f->height());
        canvas.setColor(0xddeeeeee);
        canvas.svg(resources::icons::logo_svg, -164.f, 8.f, f->width(), f->height() - 16.f);

    };

    header_frame_container->onMouseMove() = [f = header_frame_container.get(), this](const visage::MouseEvent& e){
        if(e.position.x < f->width() * 0.2){
            f->setCursorStyle(visage::MouseCursor::Pointing);
        }else{
            f->setCursorStyle(visage::MouseCursor::Arrow);
        }
    };

    header_frame_container->onMouseDown() = [f = header_frame_container.get(), this](const visage::MouseEvent& e){
        if(e.position.x < f->width() * 0.2){
            this->openAboutMenu();
        }
    };

    header_frame_container->onMouseExit() = [f = header_frame_container.get(), this](const visage::MouseEvent& e){
        f->setCursorStyle(visage::MouseCursor::Arrow);
    };
    // main_knobs_container->onDraw() = [f = main_knobs_container.get()](visage::Canvas& canvas){
    //     canvas.setColor(0xffff0000);
    //     canvas.fill(0, 0, f->width(), f->height());
    // };
    
    main_knobs_container->addChild(diffuse_freq_knob.get());
    main_knobs_container->addChild(diffuse_stages_knob.get());
    main_knobs_container->addChild(diffuse_reso_knob.get());

    visualizer_frame_container->addChild(visualizer_frame_.get());

    parameter_frame_container->addChild(main_knobs_container.get());

    main_plugin_frame->addChild(header_frame_container.get());
    main_plugin_frame->addChild(visualizer_frame_container.get());
    main_plugin_frame->addChild(parameter_frame_container.get());
    
    about_popup_frame->setVisible(false);
    addChild(main_plugin_frame.get());
    addChild(about_popup_frame.get());
}

PluginUIFrame::~PluginUIFrame(){
}

void PluginUIFrame::draw(visage::Canvas& canvas) {
    canvas.setColor(0xffeeeeee);
    canvas.fill(this->x(), this->y(), this->width(), this->height());
}

void PluginUIFrame::openAboutMenu(){
    main_plugin_frame->setBlurRadius(10.f);
    this->setReceiveChildMouseEvents(true);
    about_menu_showing = true;
    last_about_menu_ms = visage::time::milliseconds();
    about_popup_frame->setVisible(true);
}

void PluginUIFrame::closeAboutMenu(){
    about_menu_showing = false;
    main_plugin_frame->setBlurRadius(0.f);
    this->setReceiveChildMouseEvents(false);
    about_popup_frame->setVisible(false);
}

void PluginUIFrame::resized() {
    // Resizing code for the plugin UI goes here
}

void PluginUIFrame::mouseDown(const visage::MouseEvent& e){
    if(about_menu_showing && (visage::time::milliseconds() - last_about_menu_ms > 100)){
        closeAboutMenu();
    }
}

