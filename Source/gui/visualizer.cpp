#include "visualizer.h"
#include <cmath>
#include <complex>
#include "embedded/plugin_fonts.h"

VisualizerFrame::VisualizerFrame(DiffusorXAudioProcessor& processor) : audio_processor(processor) {
    layout().setFlex(true);
    layout().setMargin(visage::Dimension::logicalPixels(5));
    freq_graph_data.setNumPoints(512);
    allpass_graph_data.setNumPoints(512);
    temp_allpass_graph_data = new double[allpass_graph_data.numPoints()];
    freq_graph_data.clear();
    allpass_graph_data.clear();
    for(int i = 0; i < freq_graph_data.numPoints(); ++i) {
        freq_graph_data[i] = std::sin(static_cast<double>(i) * 0.1f) * 0.5f + 0.5f;
    }
}

VisualizerFrame::~VisualizerFrame() {
    delete[] temp_allpass_graph_data;
}

void VisualizerFrame::draw(visage::Canvas& canvas) {
    canvas.setColor(0xff333333);
    canvas.roundedRectangle(0.f, 0.f, this->width(), this->height(), this->width()*0.01);

    calculateFreqGraphData();
    visage::Brush graph_brush = visage::Brush::linear(visage::Gradient(0xffffffff, 0x00000000),
                                                      visage::Point(0.f, 0.f),
                                                      visage::Point(0.f, static_cast<double>(this->height())));
    canvas.setColor(graph_brush);
    //canvas.graphLine(graph_data, 0.f, 0.f, this->width(), this->height(), 4.f);
    canvas.graphFill(freq_graph_data, 0.f, 0.f, this->width(), this->height(), height());

    canvas.setColor(0xffffffff);
    canvas.graphLine(freq_graph_data, 0.f, 0.f, this->width(), this->height(), 2.f);

    calculateAllpassGraphData();
    calculateGroupDelayData();

    canvas.setColor(0x44ffffff);
    canvas.graphFill(allpass_graph_data, 0.f, 0.f, this->width(), this->height(), height());

    canvas.setColor(0xffffffff);
    canvas.graphLine(allpass_graph_data, 0.f, 0.f, this->width(), this->height(), 1.f);

    // visage::String min_text = visage::String("Min: ") + visage::String(min_val, 2);
    // visage::String max_text = visage::String("Max: ") + visage::String(max_val, 2);

    // canvas.setColor(0xffff0000);
    // canvas.text(min_text, visage::Font(12.f, resources::fonts::Roboto_Condensed_Bold_ttf), visage::Font::Justification::kLeft, 0.f, 0.f, 100.f, 20.f);
    // canvas.text(max_text, visage::Font(12.f, resources::fonts::Roboto_Condensed_Bold_ttf), visage::Font::Justification::kLeft, 0.f, 20.f, 100.f, 20.f);


    // For testing purposes. Logic for redrawing the visualizer when parameters change will be added later.
    redraw();
}

void VisualizerFrame::calculateFreqGraphData() {
    float* freq_data = audio_processor.getFreqAnalyzerData();
    int fft_size = audio_processor.getFreqAnalyzer()->getBufferSize();
    int sample_rate = audio_processor.getSampleRate();
    int num_points = freq_graph_data.numPoints();
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
        freq_graph_data[i] = 1.f - magnitude;
    }
}

void VisualizerFrame::calculateAllpassGraphData() {
    double sample_rate = audio_processor.getSampleRate() * 1.0;
    double oversampling_sample_rate = sample_rate * 4.0;
    if (sample_rate <= 0.f) return;
    
    double freq_param = audio_processor.getAPVTS().getRawParameterValue("frequency")->load();
    double reso_param = std::pow(audio_processor.getAPVTS().getRawParameterValue("resonance")->load(), 2) * 2.5f + 0.3;
    double omega_c = 2.f * double_Pi * freq_param / (oversampling_sample_rate);
    double alpha = std::sin(omega_c) / (2.f * reso_param);

    double a0 = 1.f + alpha;
    double a1 = -2.f * std::cos(omega_c) / a0;
    double a2 = (1.f - alpha) / a0;
    double b0 = a2;
    double b1 = a1;
    double b2 = 1.f;

    int num_points = allpass_graph_data.numPoints();
    
    // First pass: calculate raw phase
    for(int i = 0; i < num_points; ++i) {
        double current_freq = linearToLogFrequency(static_cast<double>(i) / static_cast<double>(num_points - 1), 20.f, sample_rate / 2.f);
        double omega = 2.f * double_Pi * current_freq / (oversampling_sample_rate);
        std::complex<double> z = std::exp(std::complex<double>(0.f, omega));
        std::complex<double> z_inv = 1.0 / z;
        std::complex<double> z_inv2 = z_inv * z_inv;
        
        std::complex<double> num = b0 + b1 * z_inv + b2 * z_inv2;
        std::complex<double> den = 1.0 + a1 * z_inv + a2 * z_inv2;
        std::complex<double> H = num / den;
        
        temp_allpass_graph_data[i] = std::arg(H);  // Keep in radians for now
    }
    
    // Phase unwrapping
    for(int i = 1; i < num_points; ++i) {
        double phase_diff = temp_allpass_graph_data[i] - temp_allpass_graph_data[i - 1];
        
        // If phase jumps more than π, unwrap it
        if (phase_diff > double_Pi) {
            temp_allpass_graph_data[i] -= 2.0 * double_Pi;
        } else if (phase_diff < -double_Pi) {
            temp_allpass_graph_data[i] += 2.0 * double_Pi;
        }
    }
    
    // Now convert to display format
    for (int i = 0; i < num_points; ++i) {
        double normalized_phase = (-temp_allpass_graph_data[i] / double_Pi + 1.f) / 2.f;
        temp_allpass_graph_data[i] = normalized_phase;
    }
}

void VisualizerFrame::calculateGroupDelayData() {
    double sample_rate = audio_processor.getSampleRate() * 1.0;
    double oversampling_sample_rate = sample_rate * 4.0;
    if (sample_rate <= 0.f) return;
    
    int num_points = allpass_graph_data.numPoints();
    
    // Calculate group delay using finite differences
    min_val = std::numeric_limits<double>::max();
    max_val = std::numeric_limits<double>::lowest();
    for(int i = 0; i < num_points; ++i) {
        double group_delay = 0.f;
        
        if (i == 0) {
            // Forward difference at start
            double freq_curr = linearToLogFrequency(static_cast<double>(i) / static_cast<double>(num_points - 1), 20.f, sample_rate / 2.f);
            double freq_next = linearToLogFrequency(static_cast<double>(i + 1) / static_cast<double>(num_points - 1), 20.f, sample_rate / 2.f);
            double omega_curr = 2.f * double_Pi * freq_curr / oversampling_sample_rate;
            double omega_next = 2.f * double_Pi * freq_next / oversampling_sample_rate;
            
            double phase_curr = temp_allpass_graph_data[i] * 2.f * double_Pi;  // Convert back to radians
            double phase_next = temp_allpass_graph_data[i + 1] * 2.f * double_Pi;
            
            group_delay = -(phase_next - phase_curr) / (omega_next - omega_curr);
            
        } else if (i == num_points - 1) {
            // Backward difference at end
            double freq_prev = linearToLogFrequency(static_cast<double>(i - 1) / static_cast<double>(num_points - 1), 20.f, sample_rate / 2.f);
            double freq_curr = linearToLogFrequency(static_cast<double>(i) / static_cast<double>(num_points - 1), 20.f, sample_rate / 2.f);
            double omega_prev = 2.f * double_Pi * freq_prev / oversampling_sample_rate;
            double omega_curr = 2.f * double_Pi * freq_curr / oversampling_sample_rate;
            
            double phase_prev = temp_allpass_graph_data[i - 1] * 2.f * double_Pi;
            double phase_curr = temp_allpass_graph_data[i] * 2.f * double_Pi;
            
            group_delay = -(phase_curr - phase_prev) / (omega_curr - omega_prev);
            
        } else {
            // Central difference for interior points (more accurate)
            double freq_prev = linearToLogFrequency(static_cast<double>(i - 1) / static_cast<double>(num_points - 1), 20.f, sample_rate / 2.f);
            double freq_next = linearToLogFrequency(static_cast<double>(i + 1) / static_cast<double>(num_points - 1), 20.f, sample_rate / 2.f);
            double omega_prev = 2.f * double_Pi * freq_prev / oversampling_sample_rate;
            double omega_next = 2.f * double_Pi * freq_next / oversampling_sample_rate;
            
            double phase_prev = temp_allpass_graph_data[i - 1] * 2.f * double_Pi;
            double phase_next = temp_allpass_graph_data[i + 1] * 2.f * double_Pi;
            
            group_delay = -(phase_next - phase_prev) / (omega_next - omega_prev);
        }
        
        // Group delay is in samples - you might want to normalize for display
        if (group_delay < min_val) min_val = group_delay;
        if (group_delay > max_val) max_val = group_delay;
        allpass_graph_data[i] = group_delay;
    }

    // Normalize group delay for display
    double range = max_val - min_val;
    for (int i = 0; i < allpass_graph_data.numPoints(); ++i) {
        allpass_graph_data[i] = (allpass_graph_data[i] - min_val) / range;
    }
}

void VisualizerFrame::resized() {
}