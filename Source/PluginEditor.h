/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "visage_windowing/windowing.h"
#include "visage_app/application_window.h"
#include <cassert>
#include "gui/plugin.h"

//==============================================================================
/**
*/
class DiffusorXAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DiffusorXAudioProcessorEditor (DiffusorXAudioProcessor&);
    ~DiffusorXAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DiffusorXAudioProcessor& audioProcessor;

    class VisageHostComponent : public juce::Component {
    public:
        VisageHostComponent(DiffusorXAudioProcessor& p) : audio_processor(p) {
            setOpaque(true);
            m_flogger = juce::FileLogger::createDefaultAppLogger("DiffusorXLogs", "DiffusorXLog.txt", "DiffusorX Log Started");
        }
        ~VisageHostComponent() override {
            // Shut down visage
            delete m_flogger;
        }

        void paint(juce::Graphics& g) override {
            
        }

        void visibilityChanged() override {
            initilizeIfReady();
        }

        void parentHierarchyChanged() override{
            initilizeIfReady();
        }
        
        void resized() override{
            if(app_){
                app_->setWindowDimensions(getWidth(), getHeight());
                //app_->redrawAll();
                //app_->setDpiScale(this->getDesktopScaleFactor());
            }
        }

        visage::ApplicationWindow* getApp() const {return app_.get();}
        void initilizeIfReady() {
            if(!isShowing()){
                return;
            }

            if(auto* peer = getPeer()){
                void* nativeHandle = peer->getNativeHandle();
                if(!app_){
                    app_ = std::make_unique<visage::ApplicationWindow>();

                    // Set onDraw
                    app_->onDraw() = [this](visage::Canvas& canvas){
                        canvas.setColor(0xffff0000);
                        canvas.fill(0, 0, app_->width() * 1.f, app_->height() * 1.f);
                        canvas.setColor(0xffff0ff00);
                        float circle_radius = app_->height() * 0.1f;
                        float x = app_->width() * 0.5f - circle_radius;
                        float y = app_->height() * 0.5f - circle_radius;
                        canvas.circle(x, y, 2.f * circle_radius);

                    };

                    app_->onResize() = [this]() {
                        plugin_frame_->setBounds(0, 0, app_->width(), app_->height());
                    };
                    const float width = getWidth();
                    const float height = getHeight();
                    float scale = 1.f;
                    const float dpi_scale = peer->getPlatformScaleFactor();
                    m_flogger->logMessage("VisageHostConmponent initilizeIfReady with width: " + juce::String(width) + " height: " + juce::String(height) + " scale: " + juce::String(scale) + " dpi_scale: " + juce::String(dpi_scale));


                    plugin_frame_ = std::make_unique<PluginUIFrame>(audio_processor);
                    app_->addChild(plugin_frame_.get());
                    app_->show(visage::Dimension::nativePixels(width), visage::Dimension::nativePixels(height), nativeHandle);

                    
                }
            }
        }

    private:
        std::unique_ptr<visage::ApplicationWindow> app_;
        std::unique_ptr<PluginUIFrame> plugin_frame_;
        juce::FileLogger* m_flogger;
        DiffusorXAudioProcessor& audio_processor;

    };

    std::unique_ptr<VisageHostComponent> visage_host_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DiffusorXAudioProcessorEditor)
};
