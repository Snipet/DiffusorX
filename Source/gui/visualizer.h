#pragma once
#include "visage_ui/frame.h"
#include "PluginProcessor.h"

class OutputAnalyzerThread : public juce::Thread{
public:
    OutputAnalyzerThread(DiffusorXAudioProcessor& processor) : juce::Thread("OutputAnalyzerThread"), audio_processor(processor){
        output_analyzer = std::make_unique<DiffusorXOutputAnalyzer>(2048 * 16, 2048 * 16, audio_processor.getAPVTS());
    }
    ~OutputAnalyzerThread() override{
        stopThread(4000);
    }

    DiffusorXOutputAnalyzer* getAnalyzer() {return output_analyzer.get();}

    void run() override;

private:
    std::unique_ptr<DiffusorXOutputAnalyzer> output_analyzer;
    DiffusorXAudioProcessor& audio_processor;
};

class VisualizerFrame : public visage::Frame {
public:
    VisualizerFrame(DiffusorXAudioProcessor& processor);
    ~VisualizerFrame();
    void draw(visage::Canvas& canvas) override;
    void resized() override;

private:
    visage::GraphData freq_graph_data;
    visage::GraphData allpass_graph_data;
    visage::GraphData freq_response_graph_data; // Frequency response of plugin output 
    visage::GraphData prev_freq_response_graph_data;
    double* temp_allpass_graph_data = nullptr;
    DiffusorXAudioProcessor& audio_processor;
    OutputAnalyzerThread output_analyzer_thread;
    float max_magnitude;
    double max_val;
    double min_val;

    float last_frequency_cutoff;
    float last_resonance;
    int freq_response_calc_interval;
    int freq_response_calc_tick;

    void calculateFreqGraphData();
    void calculateAllpassGraphData();
    void calculateGroupDelayData();
    void calculateFrequencyResponseGraphData();

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

    float logFrequencyToLinear(float frequency, float minFreq, float maxFreq){
        // Clamp frequency to [minFreq, maxFreq] range for safety
        frequency = std::max(minFreq, std::min(maxFreq, frequency));
        
        float logMin = std::log(minFreq);
        float logMax = std::log(maxFreq);
        float logFreq = std::log(frequency);
        
        float normalizedValue = (logFreq - logMin) / (logMax - logMin);
        
        return normalizedValue;
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

    float amplitudeToDb(float amplitude) {
        if (amplitude <= 0.0f) {
            amplitude = 0.00001;
        }
        
        float db = 20.0f * std::log10(amplitude);
        //db = std::max(-60.f, db);
        
        return db;
    }
};