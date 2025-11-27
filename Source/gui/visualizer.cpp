#include "visualizer.h"
#include <cmath>
#include <complex>

VisualizerFrame::VisualizerFrame(DiffusorXAudioProcessor& processor) : audio_processor(processor) {
    layout().setFlex(true);
    layout().setMargin(visage::Dimension::logicalPixels(5));
    freq_graph_data.setNumPoints(512);
    allpass_graph_data.setNumPoints(512);
    freq_graph_data.clear();
    allpass_graph_data.clear();
    for(int i = 0; i < freq_graph_data.numPoints(); ++i) {
        freq_graph_data[i] = std::sin(static_cast<float>(i) * 0.1f) * 0.5f + 0.5f;
    }
    max_magnitude = 1.f;
}

VisualizerFrame::~VisualizerFrame() {
}

void VisualizerFrame::draw(visage::Canvas& canvas) {
    canvas.setColor(0xff333333);
    canvas.roundedRectangle(0.f, 0.f, this->width(), this->height(), this->width()*0.01);

    calculateFreqGraphData();
    visage::Brush graph_brush = visage::Brush::linear(visage::Gradient(0xffffffff, 0x00000000),
                                                      visage::Point(0.f, 0.f),
                                                      visage::Point(0.f, static_cast<float>(this->height())));
    canvas.setColor(graph_brush);
    //canvas.graphLine(graph_data, 0.f, 0.f, this->width(), this->height(), 4.f);
    canvas.graphFill(freq_graph_data, 0.f, 0.f, this->width(), this->height(), height());

    canvas.setColor(0xffffffff);
    canvas.graphLine(freq_graph_data, 0.f, 0.f, this->width(), this->height(), 2.f);

    calculateAllpassGraphData();
    canvas.setColor(0xff00ffff);
    canvas.graphLine(allpass_graph_data, 0.f, 0.f, this->width(), this->height(), 2.f);

    redraw();
}

void VisualizerFrame::calculateFreqGraphData() {
    float* freq_data = audio_processor.getFreqAnalyzerData();
    int fft_size = audio_processor.getFreqAnalyzer()->getBufferSize();
    int sample_rate = audio_processor.getSampleRate();
    int num_points = freq_graph_data.numPoints();
    max_magnitude = 0.f;
    for(int i = 0; i < num_points; ++i) {
        float norm_value = static_cast<float>(i) / static_cast<float>(num_points - 1);
        float freq = linearToLogFrequency(norm_value, 20.f, static_cast<float>(sample_rate) / 2.f);
        float bin = getBinForFrequency(freq, sample_rate, fft_size);
        int bin_index_lower = static_cast<int>(std::floor(bin));
        int bin_index_upper = bin_index_lower + 1;
        if(bin_index_upper >= fft_size / 2){
            bin_index_upper = fft_size / 2 - 1;
        }
        float fraction = bin - static_cast<float>(bin_index_lower);
        float magnitude = 0.f;
        if(bin_index_lower >= 0 && bin_index_lower < fft_size / 2) {
            float mag_lower = freq_data[bin_index_lower];
            float mag_upper = freq_data[bin_index_upper];
            magnitude = std::abs(mag_lower + fraction * (mag_upper - mag_lower));
        }
        if(magnitude > max_magnitude) {
            max_magnitude = magnitude;
        }
        freq_graph_data[i] = 1.f - magnitude;
    }
}

void VisualizerFrame::calculateAllpassGraphData() {
    float sample_rate = audio_processor.getSampleRate();
    float freq_param = audio_processor.getAPVTS().getRawParameterValue("frequency")->load();
    float reso_param = audio_processor.getAPVTS().getRawParameterValue("resonance")->load() * 5.f + 0.5f;
    float omega_c = 2.f * float_Pi * freq_param / (sample_rate * 2.f);
    float alpha = std::sin(omega_c) / (2.f * reso_param);

    float a0 = 1.f + alpha;
    float a1 = -2.f * std::cos(omega_c) / a0;
    float a2 = (1.f - alpha) / a0;
    float b0 = a2;
    float b1 = a1;
    float b2 = 1.f;

    int num_points = allpass_graph_data.numPoints();
    for(int i = 0; i < num_points; ++i) {
        float current_freq = linearToLogFrequency(static_cast<float>(i) / static_cast<float>(num_points - 1), 20.f, sample_rate / 2.f);
        float omega = 2.f * float_Pi * current_freq / (sample_rate * 2.f);
        std::complex<float> z = std::exp(std::complex<float>(0.f, omega));
        std::complex<float> z_inv = 1.0f / z;
        std::complex<float> z_inv2 = z_inv * z_inv;
        
        std::complex<float> num = b0 + b1 * z_inv + b2 * z_inv2;
        std::complex<float> den = 1.0f + a1 * z_inv + a2 * z_inv2;
        std::complex<float> H = num / den;
        float phase = std::arg(H);
        phase = (phase + float_Pi * 2.f) / (float_Pi * 2.f) - 0.5f; // Normalize phase to [0, 1]
        if(omega > omega_c) {
            phase -= 1.f;
        }
        phase = 1.f - phase; // Invert for visualization
        allpass_graph_data[i] = phase - 0.5f;
    }
}

void VisualizerFrame::resized() {
}