/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace {
    const double DISTORTION_PI = 3.14159265358979323846;

    inline float max(float a, float b)
    {
        return (a > b) ? a : b;
    }

    inline float min(float a, float b)
    {
        return (a > b) ? b : a;
    }

    inline float sign(float in)
    {
        return (in > 0) - (in < 0);
    }

    // min以上max以下にクリップする
    inline float clip(float in, float min_val, float max_val)
    {
        return min(max_val, max(min_val, in));
    }

    // 線形クリップ
    inline float linear_clip(float in)
    {
        return clip(in, -1.0f, 1.0f);
    }

    // atanクリップ
    const float twodivpi = 2.0 / DISTORTION_PI;
    inline float atan_clip(float in)
    {
        return atanf(in) * twodivpi;
    }

    // tanhクリップ
    inline float tanh_clip(float in)
    {
        return tanh(in);
    }

    // expクリップ
    inline float exp_clip(float in)
    {
        return sign(in) * (1.0 - expf(-fabs(in)));
    }

    // 線形振幅変換（何もしない）
    inline float linear_amplifiter(float in)
    {
        return in;
    }

    // 全波整流変換
    inline float fullwave_rectifier(float in)
    {
        return fabs(in);
    }

    // 半波整流変換
    inline float halfwave_rectifier(float in)
    {
        return max(0.0f, in);
    }

    // 2乗振幅変換
    inline float square_amplifiter(float in)
    {
        return in * in;
    }
}

//==============================================================================
AE2DistortionAudioProcessor::AE2DistortionAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
    , parameters(*this, nullptr, juce::Identifier("AE2Distortion"),
        {
            std::make_unique<juce::AudioParameterBool>(
                "bypass", "Bypass",
                false),
            std::make_unique<juce::AudioParameterFloat>(
                "inGain", "Input Gain",
                juce::NormalisableRange<float>(1.0f, 200.0f),
                1.0f),
            std::make_unique<juce::AudioParameterFloat>(
                "outLevel", "Output Level",
                juce::NormalisableRange<float>(0.0f, 1.0f),
                1.0f),
            std::make_unique<juce::AudioParameterChoice>(
                "clippingCurve", "Clippling Curve",
                juce::StringArray("linear", "atan", "tanh", "exp"),
                0),
            std::make_unique<juce::AudioParameterChoice>(
                "amplifierConversion", "Amplifier Conversion",
                juce::StringArray("linear", "full-wave rectifier", "half-wave rectifier", "square"),
                0),
        })
{
    // パラメータと紐づけ
    bypass = parameters.getRawParameterValue("bypass");
    inGain = parameters.getRawParameterValue("inGain");
    outLevel = parameters.getRawParameterValue("outLevel");
    clippingCurve = parameters.getRawParameterValue("clippingCurve");
    amplifierConversion = parameters.getRawParameterValue("amplifierConversion");
}

AE2DistortionAudioProcessor::~AE2DistortionAudioProcessor()
{
}

//==============================================================================
const juce::String AE2DistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AE2DistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AE2DistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AE2DistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AE2DistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AE2DistortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AE2DistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AE2DistortionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AE2DistortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void AE2DistortionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AE2DistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void AE2DistortionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AE2DistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void AE2DistortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // バイパス判定
    if (*bypass > 0.5f) {
        return;
    }

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const float gain = *inGain;
    const float level = *outLevel;
    float (*clipping)(float) = nullptr;
    float (*amplifer)(float) = nullptr;

    // クリッピング曲線の選択
    switch (static_cast<int>(*clippingCurve)) {
    case 0: clipping = linear_clip; break;
    case 1: clipping = atan_clip; break;
    case 2: clipping = tanh_clip; break;
    case 3: clipping = exp_clip; break;
    default: jassertfalse;
    }

    // 振幅変換手法の選択
    switch (static_cast<int>(*amplifierConversion)) {
    case 0: amplifer = linear_amplifiter; break;
    case 1: amplifer = fullwave_rectifier; break;
    case 2: amplifer = halfwave_rectifier; break;
    case 3: amplifer = square_amplifiter; break;
    default: jassertfalse;
    }

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // 振幅変換
        for (int smpl = 0; smpl < buffer.getNumSamples(); smpl++) {
            channelData[smpl] = amplifer(channelData[smpl]);
        }

        // ゲイン強調・閾値処理・出力ゲイン決定
        for (int smpl = 0; smpl < buffer.getNumSamples(); smpl++) {
            channelData[smpl] *= gain;
            channelData[smpl] = clipping(channelData[smpl]);
            channelData[smpl] *= level;
        }
    }
}

//==============================================================================
bool AE2DistortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AE2DistortionAudioProcessor::createEditor()
{
    return new AE2DistortionAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void AE2DistortionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AE2DistortionAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(parameters.state.getType())) {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AE2DistortionAudioProcessor();
}
