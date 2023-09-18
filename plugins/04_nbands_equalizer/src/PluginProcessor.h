/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "ae2_biquad_filter.h"

//==============================================================================
/**
*/
class AE2NBandsEqualizerAudioProcessor  : public juce::AudioProcessor, juce::AudioProcessorValueTreeState::Listener
                            #if JucePlugin_Enable_ARA
                            , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    AE2NBandsEqualizerAudioProcessor();
    ~AE2NBandsEqualizerAudioProcessor() override;

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

    static constexpr int maxEqualizerBandsCount = 10; //! 最大のEQバンド数
    static constexpr int frequencyResponseBins = 512; //! 周波数応答を計算するビン数
    static constexpr float maxGaindB = 30.0; //! 最大のゲイン[dB]
    static constexpr float minGaindB = -30.0; //! 最小のゲイン[dB]
    double frequencyResponse[frequencyResponseBins]; //! 周波数応答
    float samplingRate;

    //! パラメータ変更時の動作
    void parameterChanged(const String &parameterID, float newValue) override;

private:
    static constexpr int maxProcessChannelCount = 2; //! 処理チャンネル数
    //! フィルタの種類
    enum class FilterType {
        FILTER_TYPE_LOWPASS = 0, //! ローパス
        FILTER_TYPE_HIGHPASS, //! ハイパス
        FILTER_TYPE_BANDPASS, //! バンドパス
        FILTER_TYPE_BANDELIMINATE, //! バンドエリミネート
        FILTER_TYPE_ALLPASS, //! オールパス
        FILTER_TYPE_LOWSHELF, //! ローシェルフ
        FILTER_TYPE_HIGHSHELF, //! ハイシェルフ
        NUMBER_OF_FILTER_TYPES,
    };

    //! 1バンドあたりのイコライザ
    struct Equalizer {
        struct AE2BiquadFilter filter[maxProcessChannelCount]; //! フィルタ実体
        std::atomic<float> *active = nullptr; //! 有効フラグ
        std::atomic<float> *type = nullptr; //! 種類
        std::atomic<float> *frequency = nullptr; //! 周波数パラメータ
        std::atomic<float> *qualityFactor = nullptr; //! クオリティファクタ
        std::atomic<float> *gain = nullptr; //! ゲイン
    };

    juce::AudioProcessorValueTreeState parameters;

    std::atomic<float> *bypass = nullptr;   //! バイパスフラグ
    std::atomic<float> *bandsCount = nullptr; //! 現在処理しているバンド数
    struct Equalizer bands[maxEqualizerBandsCount]; //! バンド

    //! パラメータレイアウトの作成
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout(void);
    //! フィルタ係数再計算
    void recalculateFilterCoefficients(int band);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AE2NBandsEqualizerAudioProcessor)
};
