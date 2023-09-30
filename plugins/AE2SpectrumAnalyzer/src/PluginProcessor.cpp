/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cfloat>

#include "ae2_fft.h"
#include "ae2_window_function.h"

//==============================================================================
AE2SpectrumAnalyzerAudioProcessor::AE2SpectrumAnalyzerAudioProcessor()
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
    , parameters(*this, nullptr, juce::Identifier("AE2AE2SpectrumAnalyzer"),
        {
            std::make_unique<juce::AudioParameterBool>(
                "bypass", "Bypass",
                false),
            std::make_unique<juce::AudioParameterInt>(
                "analyzeChannel", "Analyzing Channel",
                1, getChannelCountOfBus(true, 0),
                1),
            std::make_unique<juce::AudioParameterChoice>(
                "fftSize", "FFT size",
                juce::StringArray("256", "512", "1024", "2048", "4096", "8192", "16384"),
                0),
            std::make_unique<juce::AudioParameterChoice>(
                "windowFunction", "Window Function",
                juce::StringArray("rectangular", "hann", "hamming", "blackman"),
                0),
            std::make_unique<juce::AudioParameterChoice>(
                "slideSizeMs", "Slide size in ms",
                juce::StringArray("5", "10", "20", "40", "80", "160"),
                0),
            std::make_unique<juce::AudioParameterBool>(
                "peakHold", "Peak Hold",
                false),
        })
{
    // パラメータと紐づけ
    bypass = parameters.getRawParameterValue("bypass");
    analyzeChannel = parameters.getRawParameterValue("analyzeChannel");
    fftSize = parameters.getRawParameterValue("fftSize");
    windowFunction = parameters.getRawParameterValue("windowFunction");
    slideSizeMs = parameters.getRawParameterValue("slideSizeMs");
    peakHold = parameters.getRawParameterValue("peakHold");

    // いったん矩形窓を作成
    AE2WindowFunction_MakeWindow(AE2WINDOWFUNCTION_RECTANGULAR, window, maxFFTSize);
    windowFunctionType = Rectangular;
}

AE2SpectrumAnalyzerAudioProcessor::~AE2SpectrumAnalyzerAudioProcessor()
{
    if (ringBuffer != NULL) {
        AE2RingBuffer_Destroy(ringBuffer);
        delete[] ringBufferWork;
    }
}

//==============================================================================
const juce::String AE2SpectrumAnalyzerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AE2SpectrumAnalyzerAudioProcessor::acceptsMidi() const
{
    #if JucePlugin_WantsMidiInput
    return true;
    #else
    return false;
    #endif
}

bool AE2SpectrumAnalyzerAudioProcessor::producesMidi() const
{
    #if JucePlugin_ProducesMidiOutput
    return true;
    #else
    return false;
    #endif
}

bool AE2SpectrumAnalyzerAudioProcessor::isMidiEffect() const
{
    #if JucePlugin_IsMidiEffect
    return true;
    #else
    return false;
    #endif
}

double AE2SpectrumAnalyzerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AE2SpectrumAnalyzerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AE2SpectrumAnalyzerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AE2SpectrumAnalyzerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AE2SpectrumAnalyzerAudioProcessor::getProgramName (int index)
{
    return {};
}

void AE2SpectrumAnalyzerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AE2SpectrumAnalyzerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // すでにバッファ作成済みの場合は破棄
    if (ringBuffer != NULL) {
        AE2RingBuffer_Destroy(ringBuffer);
        delete[] ringBufferWork;
    }

    // サンプリングレートの保存
    this->sampleRate = sampleRate;

    // リングバッファの作成
    AE2RingBufferConfig config;
    const int maxGetSamples = jmax(maxFFTSize, currentSlideSamples);
    config.max_ndata = samplesPerBlock + maxGetSamples;
    config.max_required_ndata = maxGetSamples;
    config.data_unit_size = sizeof(float);
    const int ringBufferSize = AE2RingBuffer_CalculateWorkSize(&config);
    jassert(ringBufferSize >= 0);
    ringBufferWork = new uint8_t[ringBufferSize];
    ringBuffer = AE2RingBuffer_Create(&config, ringBufferWork, ringBufferSize);
    jassert(ringBuffer != NULL);

    // スライド幅の更新
    currentSlideSamples = getNumSlideSamples();
}

void AE2SpectrumAnalyzerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AE2SpectrumAnalyzerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void AE2SpectrumAnalyzerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // バイパス判定
    if (*bypass > 0.5f) {
        return;
    }

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // FFTサイズ・窓関数の変更があったら窓関数を更新
    if ((currentFFTSize != getFFTSizeParameterInt())
        || (windowFunctionType != static_cast<int>(*windowFunction))
        || (currentSlideSamples != getNumSlideSamples())) {
        currentFFTSize = getFFTSizeParameterInt();
        windowFunctionType = static_cast<int>(*windowFunction);
        currentSlideSamples = getNumSlideSamples();

        // 窓関数の作成
        switch (windowFunctionType) {
        case Rectangular: AE2WindowFunction_MakeWindow(AE2WINDOWFUNCTION_RECTANGULAR, window, currentFFTSize); break;
        case Hann:        AE2WindowFunction_MakeWindow(AE2WINDOWFUNCTION_HANN, window, currentFFTSize);        break;
        case Hamming:     AE2WindowFunction_MakeWindow(AE2WINDOWFUNCTION_HAMMING, window, currentFFTSize);     break;
        case Blackman:    AE2WindowFunction_MakeWindow(AE2WINDOWFUNCTION_BLACKMAN, window, currentFFTSize);    break;
        default: jassertfalse;
        }

        // 窓関数和の計算
        windowSum = 0.0;
        for (int i = 0; i < currentFFTSize; i++) {
            windowSum += window[i];
        }

        // ピークをリセット
        for (int bin = 0; bin <= maxFFTSize / 2; bin++) {
            peakSpectrum[bin] = -FLT_MAX;
        }
    }

    // 分析チャンネルの切り替え
    if (currentAnalyzeChannel != getAnalyzeChannel()) {
        currentAnalyzeChannel = getAnalyzeChannel();
    }

    const auto *channelData = buffer.getWritePointer (currentAnalyzeChannel);
    const int sampleCounts = buffer.getNumSamples();
    int processSamples = 0;
    while (processSamples < sampleCounts) {
        // 挿入するサンプル数
        const int putSamples = jmin(sampleCounts - processSamples, currentSlideSamples);
        // データをリングバッファに挿入
        AE2RingBuffer_Put(ringBuffer, &channelData[processSamples], putSamples);

        // FFTサイズ分のデータが溜まったらリングバッファから取り出しFFT実行
        if (AE2RingBuffer_GetRemainNumData(ringBuffer) >= jmax(currentFFTSize, currentSlideSamples)) {
            const float regularization_factor = 2.0 / windowSum; // 正規化定数
            void *pdata;

            analyzeLock.enterWrite();

            // スライド幅だけ取り出しで解析
            AE2RingBuffer_Get(ringBuffer, &pdata, currentSlideSamples);
            memcpy(analyzedSpectrum, pdata, currentFFTSize * sizeof(float));
            memcpy(analyzedWave, pdata, currentFFTSize * sizeof(float));

            // 正規化・窓かけ
            for (int smpl = 0; smpl < currentFFTSize; smpl++) {
                analyzedSpectrum[smpl] *= (regularization_factor * window[smpl]);
            }
            // FFT
            AE2FFT_RealFFT(currentFFTSize, -1, analyzedSpectrum, fftWork);
            // パワーを計算
            analyzedSpectrum[0] = analyzedSpectrum[0] * analyzedSpectrum[0]; // 直流成分
            const float nypuistPower = analyzedSpectrum[1] * analyzedSpectrum[1]; // ナイキスト周波数成分
            for (int bin = 1; bin < currentFFTSize / 2; bin++) {
                analyzedSpectrum[bin]
                    = AE2FFTCOMPLEX_REAL(analyzedSpectrum, bin) * AE2FFTCOMPLEX_REAL(analyzedSpectrum, bin)
                    + AE2FFTCOMPLEX_IMAG(analyzedSpectrum, bin) * AE2FFTCOMPLEX_IMAG(analyzedSpectrum, bin);
            }
            analyzedSpectrum[currentFFTSize / 2] = nypuistPower;
            // dBに変換
            // TODO: 外観に関する部分なのでEditorがやるべき
            for (int bin = 0; bin <= currentFFTSize / 2; bin++) {
                analyzedSpectrum[bin] = 10.0 * log10f(analyzedSpectrum[bin]);
            }
            // ピークホールド状態の変化
            if (peakHoldEnable != (*peakHold > 0.5f)) {
                peakHoldEnable = (*peakHold > 0.5f);
                // 有効になったときはピークをリセット
                if (peakHoldEnable) {
                    for (int bin = 0; bin <= maxFFTSize / 2; bin++) {
                        peakSpectrum[bin] = -FLT_MAX;
                    }
                }
            }
            // ピーク値の更新
            if (peakHoldEnable) {
                for (int bin = 0; bin <= currentFFTSize / 2; bin++) {
                    peakSpectrum[bin] = jmax(peakSpectrum[bin], analyzedSpectrum[bin]);
                }
            }

            analyzeLock.exitWrite();
        }
        // 進捗を進める
        processSamples += putSamples;
    }
}

//==============================================================================
bool AE2SpectrumAnalyzerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AE2SpectrumAnalyzerAudioProcessor::createEditor()
{
    return new AE2SpectrumAnalyzerAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void AE2SpectrumAnalyzerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AE2SpectrumAnalyzerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
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
    return new AE2SpectrumAnalyzerAudioProcessor();
}
