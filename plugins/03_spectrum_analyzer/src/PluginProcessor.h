/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "ae2_ring_buffer.h"

//==============================================================================
/**
*/
class AE2SpectrumAnalyzerAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    AE2SpectrumAnalyzerAudioProcessor();
    ~AE2SpectrumAnalyzerAudioProcessor() override;

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

    int currentAnalyzeChannel = 0; //! 解析対象のチャンネル
    static const int maxFFTSize = 1 << 14; //! 最大のFFTサイズ

    double sampleRate = 0.0; //! サンプリングレート

    ReadWriteLock analyzeLock; //! 解析データの排他制御
    float analyzedWave[maxFFTSize]; //! 解析対象の波形
    float analyzedSpectrum[maxFFTSize]; //! スペクトラム
    float peakSpectrum[(maxFFTSize / 2) + 1]; //! スペクトラムピーク 
    bool peakHoldEnable = false; //! ピークホールド有効？ 

    // パラメータ値から実際のFFTサイズに変換
    inline int getFFTSizeParameterInt(void) const
    {
        return 256 * (1 << (static_cast<int>(*fftSize)));
    }

private:
    //! 窓関数タイプ 
    enum WindowFunctionType
    {
        Rectangular = 0,
        Hann = 1,
        Hamming = 2,
        Blackman = 3,
    };

    juce::AudioProcessorValueTreeState parameters;

    std::atomic<float> *bypass = nullptr;   //! バイパスフラグ
    std::atomic<float> *fftSize = nullptr; //! FFTサイズ
    std::atomic<float> *windowFunction = nullptr; //! 窓関数
    std::atomic<float> *slideSizeMs = nullptr; //! スライド幅 
    std::atomic<float> *peakHold = nullptr; //! ピークホールド
    std::atomic<float> *analyzeChannel = nullptr; //! 解析チャンネル 

    std::atomic<AE2RingBuffer *> ringBuffer = NULL; //! 音声データのリングバッファ
    uint8_t *ringBufferWork = nullptr; //! リングバッファのワーク領域
    float fftWork[maxFFTSize]; //! FFT実行用のワーク領域
    int currentFFTSize = 0; //! 現在のFFTサイズ
    int currentSlideSamples = 0; //! スライド幅
    float window[maxFFTSize]; //! 窓 
    double windowSum = maxFFTSize; //! 窓関数の和 
    int windowFunctionType = Rectangular; //! 窓関数タイプ

    // パラメータ値からチャンネルインデックスに変換
    inline int getAnalyzeChannel(void) const
    {
        return static_cast<int>(*analyzeChannel) - 1; // インデックスに直すため-1
    }
    // パラメータ値からスライドサンプル数を計算
    inline int getNumSlideSamples(void) const
    {
        static const juce::StringArray slideMsList = juce::StringArray("5", "10", "20", "40", "80", "160");
        return static_cast<int>((sampleRate * slideMsList[static_cast<int>(*slideSizeMs)].getDoubleValue()) / 1000.0);
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AE2SpectrumAnalyzerAudioProcessor)
};
