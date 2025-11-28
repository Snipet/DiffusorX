#pragma once
#include "visage_ui/frame.h"
#include "PluginProcessor.h"

class VisualizerFrame : public visage::Frame {
public:
    VisualizerFrame(DiffusorXAudioProcessor& processor);
    ~VisualizerFrame();
    void draw(visage::Canvas& canvas) override;
    void resized() override;

private:
    visage::GraphData freq_graph_data;
    visage::GraphData allpass_graph_data;
    double* temp_allpass_graph_data = nullptr;
    DiffusorXAudioProcessor& audio_processor;
    float max_magnitude;
    double max_val;
    double min_val;

    void calculateFreqGraphData();
    void calculateAllpassGraphData();
    void calculateGroupDelayData();

    float linearToLogFrequency(float normalizedValue, float minFreq, float maxFreq)
    {
        // Clamp input to [0, 1] range for safety
        normalizedValue = std::max(0.0f, std::min(1.0f, normalizedValue));
        
        // Calculate the logarithmic ratio
        // frequency = minFreq * (maxFreq/minFreq)^normalizedValue
        float logMin = std::log(minFreq);
        float logMax = std::log(maxFreq);
        
        float logFreq = logMin + normalizedValue * (logMax - logMin);
        
        return std::exp(logFreq);
    }

    float getBinForFrequency(float frequency, int sampleRate, int fftSize)
    {
        // Ensure frequency is within valid range
        if (frequency < 0.0f || frequency > sampleRate / 2.0f)
            return 0.f; // Invalid frequency
        
        // Calculate the bin index
        float binIndex = (frequency / (sampleRate / 2.0f)) * (fftSize / 2);
        
        return binIndex;
    }
};