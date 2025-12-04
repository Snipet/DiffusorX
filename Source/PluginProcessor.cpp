/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DiffusorXAudioProcessor::DiffusorXAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                       apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    freq_analyzer = new FreqAnalyzer(2048 * 2);
    mono_buffer = nullptr;

    // Init audio processors
    audio_processors.push_back(std::make_unique<DiffusorXMonoAudioProcessor>(apvts));
    audio_processors.push_back(std::make_unique<DiffusorXMonoAudioProcessor>(apvts));
}

juce::AudioProcessorValueTreeState::ParameterLayout DiffusorXAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // 1) Frequency - logarithmic scale
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "frequency",
        "Frequency",
        juce::NormalisableRange<float>(
            20.0f,                    // min
            22050.0f,                 // max
            0.01f,                    // step
            0.3f),                    // skew factor for logarithmic (< 1.0 = log scale)
        1000.0f,                      // default value
        "Hz",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " Hz"; },
        [](const juce::String& text) { 
            return text.trimEnd().upToFirstOccurrenceOf(" ", false, false).getFloatValue();
        }
    ));
    
    // 2) Resonance - linear scale, no label
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "resonance",
        "Resonance",
        juce::NormalisableRange<float>(0.01f, 1.0f, 0.01f, 0.75f),
        0.1f                          // default value
    ));
    
    // 3) Diffuse Stages - integer parameter, linear scale
    layout.add(std::make_unique<juce::AudioParameterInt>(
        "diffuse_stages",
        "Diffuse Stages",
        0,                            // min
        DIFFUSORX_MAX_STAGES,                          // max
        10                            // default value
    ));
    
    // 4) Bypass - boolean parameter
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "bypass",
        "Bypass",
        false                         // default value (not bypassed)
    ));
    
    return layout;
}

DiffusorXAudioProcessor::~DiffusorXAudioProcessor()
{
    delete freq_analyzer;
    delete[] mono_buffer;
}

//==============================================================================
const juce::String DiffusorXAudioProcessor::getName() const
{
    return "DiffusorX";
}

bool DiffusorXAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DiffusorXAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DiffusorXAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DiffusorXAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DiffusorXAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DiffusorXAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DiffusorXAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DiffusorXAudioProcessor::getProgramName (int index)
{
    return {};
}

void DiffusorXAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DiffusorXAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    if(mono_buffer != nullptr)
    {
        delete[] mono_buffer;
    }

    mono_buffer = new float[samplesPerBlock];

    for(auto& p : audio_processors){
        p->prepareToPlay(sampleRate, samplesPerBlock);
    }
}

void DiffusorXAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DiffusorXAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void DiffusorXAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    float mono_scale = 1.f / static_cast<float> (totalNumInputChannels);
    memset(mono_buffer, 0, sizeof(float) * buffer.getNumSamples());
    for (int channel = 0; channel < totalNumInputChannels; ++channel){
        auto* channelData = buffer.getReadPointer (channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            mono_buffer[sample] += channelData[sample] * mono_scale;
        }
    }

    // Process the mono buffer with the frequency analyzer
    freq_analyzer->processBlock(mono_buffer, buffer.getNumSamples());

    auto stereoBlock = juce::dsp::AudioBlock<float>(buffer);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto monoBlock = stereoBlock.getSingleChannelBlock(channel);
        audio_processors[channel]->processBlock(monoBlock);
    }
}

//==============================================================================
bool DiffusorXAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DiffusorXAudioProcessor::createEditor()
{
    return new DiffusorXAudioProcessorEditor (*this);
}

//==============================================================================
void DiffusorXAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DiffusorXAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if(xml && xml->hasTagName(apvts.state.getType())){
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DiffusorXAudioProcessor();
}
