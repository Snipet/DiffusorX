#include "plugin.h"

PluginUIFrame::PluginUIFrame(DiffusorXAudioProcessor& p) : audio_processor(p) {
    visualizer_frame_ = std::make_unique<VisualizerFrame>(audio_processor);
    visualizer_frame_container = std::make_unique<visage::Frame>();
    parameter_frame_container = std::make_unique<visage::Frame>();
    main_knobs_container = std::make_unique<visage::Frame>();
    diffuse_freq_knob = std::make_unique<KnobContainer<juce::AudioParameterFloat>>(audio_processor.getAPVTS(), "frequency", 95);
    diffuse_stages_knob = std::make_unique<KnobContainer<juce::AudioParameterInt>>(audio_processor.getAPVTS(), "diffuse_stages", 110);
    diffuse_reso_knob = std::make_unique<KnobContainer<juce::AudioParameterFloat>>(audio_processor.getAPVTS(), "resonance", 95);
    //test_knob = std::make_unique<Knob>(audio_processor.getAPVTS(), "gain");

    // Set flex settings
    this->layout().setFlex(true);
    //this->layout().setFlexItemAlignment(visage::Layout::ItemAlignment::Center);
    this->layout().setFlexRows(true);
    //this->layout().setFlexGrow(1.f);

    visualizer_frame_container->layout().setFlexGrow(0.5f);
    parameter_frame_container->layout().setFlexGrow(0.5f);
    visualizer_frame_container->layout().setDimensions(visage::Dimension::viewMaxPercent(100), visage::Dimension::viewMinPercent(50));
    parameter_frame_container->layout().setDimensions(visage::Dimension::viewMaxPercent(100), visage::Dimension::viewMinPercent(50));
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
    
    parameter_frame_container->onDraw() = [f = parameter_frame_container.get()](visage::Canvas& canvas){
        canvas.setColor(0xffeeeeee);
        canvas.fill(0, 0, f->width(), f->height());
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

    addChild(visualizer_frame_container.get());
    addChild(parameter_frame_container.get());
}

PluginUIFrame::~PluginUIFrame(){
}

void PluginUIFrame::draw(visage::Canvas& canvas) {
    canvas.setColor(0xffeeeeee);
    canvas.fill(this->x(), this->y(), this->width(), this->height());
}

void PluginUIFrame::resized() {
    // Resizing code for the plugin UI goes here
}

