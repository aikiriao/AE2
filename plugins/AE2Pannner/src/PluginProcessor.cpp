/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "ae2_simple_hrtf.h"

namespace {
    // 角度を[-pi, pi)に丸め込む
    float wrapAngle(float angle)
    {
        return angle - 2.0 * MathConstants<float>::pi * floor((angle + MathConstants<float>::pi) / (2.0 * MathConstants<float>::pi));
    }
}

//==============================================================================
AE2PannnerAudioProcessor::AE2PannnerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                    #if ! JucePlugin_IsMidiEffect
                    #if ! JucePlugin_IsSynth
                        .withInput  ("Input",  juce::AudioChannelSet::mono(), true)
                    #endif
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                    #endif
                    )
#endif
    , parameters(*this, nullptr, juce::Identifier("AE2Pannner"),
        {
            std::make_unique<juce::AudioParameterBool>(
                "bypass", "Bypass",
                false),
            std::make_unique<juce::AudioParameterChoice>(
                "panningMethod", "Panning Method",
                juce::StringArray("Amplitude", "SimpleHRTF"),
                static_cast<int>(PanningMethod::CONSTANT_POWER_PANNING)),
            std::make_unique<juce::AudioParameterFloat>(
                "azimuthAngle", "Azimuth Angle in Degree",
                -180.0, 180.0, 0.0),
            std::make_unique<juce::AudioParameterFloat>(
                "radiusOfHead", "Radius of Head in m",
                minRadiusOfHead, maxRadiusOfHead, AE2SIMPLEHRTF_DEFAULT_RADIUS_OF_HEAD),
        })
{
    // パラメータと紐づけ
    bypass = parameters.getRawParameterValue("bypass");
    panningMethod = parameters.getRawParameterValue("panningMethod");
    azimuthAngle = parameters.getRawParameterValue("azimuthAngle");
    radiusOfHead = parameters.getRawParameterValue("radiusOfHead");

    parameters.addParameterListener("azimuthAngle", this);
    parameters.addParameterListener("radiusOfHead", this);
}

AE2PannnerAudioProcessor::~AE2PannnerAudioProcessor()
{
}

//==============================================================================
const juce::String AE2PannnerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AE2PannnerAudioProcessor::acceptsMidi() const
{
    #if JucePlugin_WantsMidiInput
    return true;
    #else
    return false;
    #endif
}

bool AE2PannnerAudioProcessor::producesMidi() const
{
    #if JucePlugin_ProducesMidiOutput
    return true;
    #else
    return false;
    #endif
}

bool AE2PannnerAudioProcessor::isMidiEffect() const
{
    #if JucePlugin_IsMidiEffect
    return true;
    #else
    return false;
    #endif
}

double AE2PannnerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AE2PannnerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AE2PannnerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AE2PannnerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AE2PannnerAudioProcessor::getProgramName (int index)
{
    return {};
}

void AE2PannnerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AE2PannnerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    {
        int32_t hrtfWorkSize;
        struct AE2SimpleHRTFConfig config;
        config.delay_fade_time_ms = 10.0;
        config.max_num_process_samples = samplesPerBlock;
        config.max_radius_of_head = maxRadiusOfHead;
        config.sampling_rate = sampleRate;
        hrtfWorkSize = AE2SimpleHRTF_CalculateWorkSize(&config);
        jassert(hrtfWorkSize >= 0);

        // ハンドル作成
        for (int ch = 0; ch < 2; ch++) {
            // 一度作成していた場合は作り直す
            if (hrtf[ch] != nullptr) {
                AE2SimpleHRTF_Destroy(hrtf[ch]);
                delete[] hrtfWork[ch];
            }
            hrtfWork[ch] = new uint8_t[hrtfWorkSize];
            hrtf[ch] = AE2SimpleHRTF_Create(&config, hrtfWork[ch], hrtfWorkSize);
            jassert(hrtf[ch] != NULL);
        }
    }
}

void AE2PannnerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AE2PannnerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()) {
        return false;
    }

    // This checks if the input layout matches the output layout
    #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    #endif

    return true;
    #endif
}
#endif

void AE2PannnerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

    // モノラルチャンネルのデータを全チャンネルにコピー
    for (int channel = 0; channel < totalNumOutputChannels; channel++) {
        buffer.copyFrom(channel, 0, buffer.getReadPointer(0), buffer.getNumSamples());
    }

    switch (static_cast<PanningMethod>(static_cast<int>(*panningMethod))) {
    case PanningMethod::CONSTANT_POWER_PANNING:
        // ゲイン計算
        calculateAmplitudeGain((*azimuthAngle) * MathConstants<float>::pi / 180.0f);
        // 各チャンネルにゲイン適用
        for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
            auto *channelData = buffer.getWritePointer(channel);
            for (int smpl = 0; smpl < buffer.getNumSamples(); smpl++) {
                channelData[smpl] *= pannningGain[channel];
            }
        }
        break;
    case PanningMethod::SIMPLE_HRTF_PANNING:
        // 各チャンネルでHRTF適用
        for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
            auto *channelData = buffer.getWritePointer(channel);
            AE2SimpleHRTF_Process(hrtf[channel], channelData, buffer.getNumSamples());
        }
        break;
    default:
        jassertfalse;
    }
}

//==============================================================================
bool AE2PannnerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AE2PannnerAudioProcessor::createEditor()
{
    return new AE2PannnerAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void AE2PannnerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AE2PannnerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(parameters.state.getType())) {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

void AE2PannnerAudioProcessor::parameterChanged(const String &parameterID, float newValue)
{
    if (hrtf[0] != nullptr && hrtf[1] != nullptr) {
        if (parameterID == "azimuthAngle") {
            const float radianAngle = (*azimuthAngle) * MathConstants<float>::pi / 180.0f;
            AE2SimpleHRTF_SetAzimuthAngle(hrtf[0], wrapAngle(MathConstants<float>::pi / 2.0 - radianAngle));
            AE2SimpleHRTF_SetAzimuthAngle(hrtf[1], wrapAngle(MathConstants<float>::pi / 2.0 + radianAngle));
        } else if (parameterID == "radiusOfHead") {
            for (int ch = 0; ch < 2; ch++) {
                AE2SimpleHRTF_SetHeadRadius(hrtf[ch], *radiusOfHead);
            }
        }
    }
}

// 振幅パンニングのゲイン計算
void AE2PannnerAudioProcessor::calculateAmplitudeGain(float angle)
{
    float tmpGain[2];
    tmpGain[1] = 1.0f; // 後に正規化するため仮に1とする
    tmpGain[0] = -(tan(angle) + tan(halfSpeakerAngle)) / (tan(angle) - tan(halfSpeakerAngle));
    const float norm = sqrt(tmpGain[0] * tmpGain[0] + tmpGain[1] * tmpGain[1]);
    pannningGain[0] = tmpGain[0] / norm;
    pannningGain[1] = tmpGain[1] / norm;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AE2PannnerAudioProcessor();
}
