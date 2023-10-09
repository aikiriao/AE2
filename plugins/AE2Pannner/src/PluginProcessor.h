/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include <cstdint>

//==============================================================================
/**
*/
class AE2PannnerAudioProcessor  : public juce::AudioProcessor, juce::AudioProcessorValueTreeState::Listener
                            #if JucePlugin_Enable_ARA
                            , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    AE2PannnerAudioProcessor();
    ~AE2PannnerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //! パラメータ変更時の動作
    void parameterChanged(const String &parameterID, float newValue) override;

private:
    juce::AudioProcessorValueTreeState parameters;

    enum class PanningMethod {
        CONSTANT_POWER_PANNING = 0,
        SIMPLE_HRTF_PANNING,
        NUMBER_OF_PANNING_METHODS,
    };

    static constexpr float minRadiusOfHead = 0.01f; //! 最小頭部実効半径[m]
    static constexpr float maxRadiusOfHead = 0.3f; //! 最大頭部実効半径[m]
    static constexpr float halfSpeakerAngle = MathConstants<float>::pi / 4.0; //! スピーカ間の角度の半分

    std::atomic<float> *bypass = nullptr;   //! バイパスフラグ
    std::atomic<float> *panningMethod = nullptr; //! パンニング手法
    std::atomic<float> *azimuthAngle = nullptr; //! 角度
    std::atomic<float> *radiusOfHead = nullptr; //! 実効頭半径

    struct AE2SimpleHRTF *hrtf[2] = { nullptr, nullptr }; //! HRTFハンドル
    uint8_t *hrtfWork[2] = { nullptr, nullptr }; //! HRTFハンドルのワーク領域
    float pannningGain[2] = { 0.0f, 0.0f }; //! 振幅パンニングのゲイン

    void calculateAmplitudeGain(float angle);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AE2PannnerAudioProcessor)
};
