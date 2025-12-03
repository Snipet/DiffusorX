#include "output_analyzer.h"

DiffusorXOutputAnalyzer::DiffusorXOutputAnalyzer(size_t buffer_size, size_t fft_size, juce::AudioProcessorValueTreeState& state) : buffer_size(buffer_size), apvts(state), buffer(1, buffer_size), fft_size(fft_size){
    magnitude_spectrum_1 = new float[fft_size / 2 + 1];
    magnitude_spectrum_2 = new float[fft_size / 2 + 1];
    audio_processor = std::make_unique<DiffusorXMonoAudioProcessor>(apvts);
    fft_input = new float[fft_size];
    fft_output = new kiss_fft_cpx[fft_size / 2 + 1];
    fft_cfg = kiss_fftr_alloc(fft_size, 0, nullptr, nullptr);
    output_magnitude_spectrum_ptr = magnitude_spectrum_1;
}

DiffusorXOutputAnalyzer::~DiffusorXOutputAnalyzer(){
    if(magnitude_spectrum_1){
        delete[] magnitude_spectrum_1;
    }

    if(magnitude_spectrum_2){
        delete[] magnitude_spectrum_2;
    }

    if(fft_input){
        delete[] fft_input;
    }

    if(fft_output){
        delete[] fft_output;
    }

    if(fft_cfg) {
        kiss_fft_free(fft_cfg);
    }
}

void DiffusorXOutputAnalyzer::generateMagnitudeSpectrum(){
    audio_processor->prepareToPlay(44100.f, buffer_size);
    buffer.clear();
    buffer.setSample(0, 1, 1.f); // Impulse
    // buffer.setSample(0, 1, 1.f); // Impulse
    // buffer.setSample(0, 2, 1.f); // Impulse
    auto block = juce::dsp::AudioBlock<float>(buffer);

    audio_processor->processBlock(block);

    float* temp_magnitude_spectrum;
    if(output_magnitude_spectrum_ptr == magnitude_spectrum_1){
        temp_magnitude_spectrum = magnitude_spectrum_2;
    }else{
        temp_magnitude_spectrum = magnitude_spectrum_1;
    }

    
    // // Highpass the block
    // juce::dsp::ProcessSpec spec;
    // spec.sampleRate = 44100.f;
    // spec.maximumBlockSize = buffer_size;
    // spec.numChannels = 1;

    // highpass_chain.prepare(spec);
    // auto coeffs = *juce::dsp::IIR::Coefficients<float>::makeHighPass(44100.f, 100.0f);
    // *highpass_chain.get<0>().coefficients = coeffs;
    // *highpass_chain.get<1>().coefficients = coeffs;
    // *highpass_chain.get<2>().coefficients = coeffs;
    // *highpass_chain.get<3>().coefficients = coeffs;
    // auto context = juce::dsp::ProcessContextReplacing<float>(block);
    // highpass_chain.process(context);


    float* audio_result = block.getChannelPointer(0);
    for(int i = 0; i < fft_size / 2 + 1; ++i){
        temp_magnitude_spectrum[i] = 0;
    }

    // size_t hop_size = fft_size / 2;
    // size_t n_hop_frames = buffer_size / hop_size - 1;
    // const float fft_scale = 1.f / (2.f);
    // for(int i = 0; i < n_hop_frames; ++i){
    //     size_t sample_offset = i * hop_size;
    //     for(int s = 0; s < fft_size; ++s){
    //         fft_input[s] = audio_result[sample_offset + s] * hannWindow(s, fft_size);
    //     }

    //     kiss_fftr(fft_cfg, fft_input, fft_output);
    //     for(int k = 0; k < fft_size / 2 + 1; ++k) {
    //         magnitude_spectrum[k] += sqrtf(fft_output[k].r * fft_output[k].r + fft_output[k].i * fft_output[k].i) * fft_scale;
    //     }
    // }




    kiss_fftr(fft_cfg, audio_result, fft_output);
    const float scale = 1.0f / 2.f;
    for(int k = 0; k < fft_size / 2 + 1; ++k) {
        temp_magnitude_spectrum[k] = sqrtf(fft_output[k].r * fft_output[k].r + fft_output[k].i * fft_output[k].i) * scale;
    }

    output_magnitude_spectrum_ptr = temp_magnitude_spectrum;
}

