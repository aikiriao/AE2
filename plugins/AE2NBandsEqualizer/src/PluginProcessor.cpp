/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AE2NBandsEqualizerAudioProcessor::AE2NBandsEqualizerAudioProcessor()
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
    , parameters(*this, nullptr, juce::Identifier("AE2NBandsEqualizer"), createParameterLayout())
    , samplingRate(44100.0f)
{
    // パラメータと紐づけ
    bypass = parameters.getRawParameterValue("bypass");
    parameters.addParameterListener("bypass", this);
    bandsCount = parameters.getRawParameterValue("nbands");
    parameters.addParameterListener("nbands", this);

    for (int i = 0; i < maxEqualizerBandsCount; i++) {
        // 全フィルタバッファをクリア
        for (int ch = 0; ch < maxProcessChannelCount; ch++) {
            AE2BiquadFilter_ClearBuffer(&bands[i].filter[ch]);
        }
        const String active_id = String("active_") + String(i);
        bands[i].active = parameters.getRawParameterValue(active_id);
        parameters.addParameterListener(active_id, this);
        const String type_id = String("type_") + String(i);
        bands[i].type = parameters.getRawParameterValue(type_id);
        parameters.addParameterListener(type_id, this);
        const String frequency_id = String("frequency_") + String(i);
        bands[i].frequency = parameters.getRawParameterValue(frequency_id);
        parameters.addParameterListener(frequency_id, this);
        const String qualityfactor_id = String("qualityfactor_") + String(i);
        bands[i].qualityFactor = parameters.getRawParameterValue(String("qualityfactor_") + String(i));
        parameters.addParameterListener(qualityfactor_id, this);
        const String gain_id = String("gain_") + String(i);
        bands[i].gain = parameters.getRawParameterValue(String("gain_") + String(i));
        parameters.addParameterListener(gain_id, this);
    }
}

AudioProcessorValueTreeState::ParameterLayout AE2NBandsEqualizerAudioProcessor::createParameterLayout(void)
{
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));
    layout.add(std::make_unique<juce::AudioParameterInt>(
        "nbands", "Number of bands", 1, maxEqualizerBandsCount, 1));

    for (int i = 0; i < maxEqualizerBandsCount; i++) {
        layout.add(std::make_unique<AudioParameterBool>(
            String("active_") + String(i), String("Active") + String(i), true));
        layout.add(std::make_unique<AudioParameterInt>(
            String("type_") + String(i), String("Type") + String(i),
            0, static_cast<int>(FilterType::NUMBER_OF_FILTER_TYPES) - 1,
            static_cast<int>(FilterType::FILTER_TYPE_LOWPASS)));
        layout.add(std::make_unique<AudioParameterFloat>(
            String("frequency_") + String(i), String("Frequency") + String(i),
            10.0f, 18000.0f, 1000.0f));
        layout.add(std::make_unique<AudioParameterFloat>(
            String("qualityfactor_") + String(i), String("Quality Factor") + String(i),
            1.0 / sqrt(2.0), 30.0f, 1.0 / sqrt(2.0)));
        layout.add(std::make_unique<AudioParameterFloat>(
            String("gain_") + String(i), String("Gain in dB") + String(i),
            minGaindB, maxGaindB, 0.0));
    }

    return layout;
};


AE2NBandsEqualizerAudioProcessor::~AE2NBandsEqualizerAudioProcessor()
{
}

//==============================================================================
const juce::String AE2NBandsEqualizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AE2NBandsEqualizerAudioProcessor::acceptsMidi() const
{
    #if JucePlugin_WantsMidiInput
    return true;
    #else
    return false;
    #endif
}

bool AE2NBandsEqualizerAudioProcessor::producesMidi() const
{
    #if JucePlugin_ProducesMidiOutput
    return true;
    #else
    return false;
    #endif
}

bool AE2NBandsEqualizerAudioProcessor::isMidiEffect() const
{
    #if JucePlugin_IsMidiEffect
    return true;
    #else
    return false;
    #endif
}

double AE2NBandsEqualizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AE2NBandsEqualizerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AE2NBandsEqualizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AE2NBandsEqualizerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AE2NBandsEqualizerAudioProcessor::getProgramName (int index)
{
    return {};
}

void AE2NBandsEqualizerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AE2NBandsEqualizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // サンプリングレートを記録
    samplingRate = sampleRate;

    // サンプリングレート変更に伴って全フィルタを再計算
    for (int i = 0; i < *bandsCount; i++) {
        recalculateFilterCoefficients(i);
        for (int ch = 0; ch < maxProcessChannelCount; ch++) {
            AE2BiquadFilter_ClearBuffer(&bands[i].filter[ch]);
        }
    }
}

void AE2NBandsEqualizerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AE2NBandsEqualizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void AE2NBandsEqualizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // フィルタ適用
        for (int i = 0; i < *bandsCount; i++) {
            if (*bands[i].active > 0.5) {
                AE2BiquadFilter_Process(&bands[i].filter[channel], channelData, buffer.getNumSamples());
            }
        }
    }
}

//==============================================================================
bool AE2NBandsEqualizerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AE2NBandsEqualizerAudioProcessor::createEditor()
{
    return new AE2NBandsEqualizerAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void AE2NBandsEqualizerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AE2NBandsEqualizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(parameters.state.getType())) {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

// フィルタ係数再計算
void AE2NBandsEqualizerAudioProcessor::recalculateFilterCoefficients(int band)
{
    for (int ch = 0; ch < maxProcessChannelCount; ch++) {
        struct Equalizer &eq = bands[band];
        const float linearGain = pow(10.0, *eq.gain / 20.0);

        switch (static_cast<FilterType>(static_cast<int>(*eq.type))) {
        case FilterType::FILTER_TYPE_LOWPASS:
            AE2BiquadFilter_SetLowPass(&eq.filter[ch], samplingRate, *eq.frequency, *eq.qualityFactor, linearGain);
            break;
        case FilterType::FILTER_TYPE_HIGHPASS:
            AE2BiquadFilter_SetHighPass(&eq.filter[ch], samplingRate, *eq.frequency, *eq.qualityFactor, linearGain);
            break;
        case FilterType::FILTER_TYPE_BANDPASS:
            AE2BiquadFilter_SetBandPass(&eq.filter[ch], samplingRate, *eq.frequency, *eq.qualityFactor, linearGain);
            break;
        case FilterType::FILTER_TYPE_BANDELIMINATE:
            AE2BiquadFilter_SetBandEliminate(&eq.filter[ch], samplingRate, *eq.frequency, *eq.qualityFactor, linearGain);
            break;
        case FilterType::FILTER_TYPE_ALLPASS:
            AE2BiquadFilter_SetAllPass(&eq.filter[ch], samplingRate, *eq.frequency, *eq.qualityFactor, linearGain);
            break;
        case FilterType::FILTER_TYPE_LOWSHELF:
            AE2BiquadFilter_SetLowShelf(&eq.filter[ch], samplingRate, *eq.frequency, *eq.qualityFactor, linearGain);
            break;
        case FilterType::FILTER_TYPE_HIGHSHELF:
            AE2BiquadFilter_SetHighShelf(&eq.filter[ch], samplingRate, *eq.frequency, *eq.qualityFactor, linearGain);
            break;
        default:
            jassertfalse;
        }
    }

    // 周波数応答の再計算
    for (int i = 0; i < frequencyResponseBins; i++) {
        frequencyAmplitudeResponse[i] = frequencyPhaseResponse[i] = 0.0;
    }
    for (int i = 0; i < *bandsCount; i++) {
        if (*bands[i].active > 0.5) {
            const struct AE2BiquadFilter *f = &bands[i].filter[0];
            for (int j = 0; j < frequencyResponseBins; j++) {
                const double omega0 = (MathConstants<double>::pi * j) / frequencyResponseBins;
                // TODO: sin,cosはテーブルにしておくといいかも
                const double sinw0 = sin(omega0);
                const double cosw0 = cos(omega0);
                const double re_denom = cosw0 + f->a1 + f->a2 * cosw0;
                const double im_denom = sinw0 - f->a2 * sinw0;
                const double re_numer = f->b0 * cosw0 + f->b1 + f->b2 * cosw0;
                const double im_numer = f->b0 * sinw0 - f->b2 * sinw0;
                frequencyAmplitudeResponse[j]
                    += 10.0 * (log10(re_numer * re_numer + im_numer * im_numer) - log10(re_denom * re_denom + im_denom * im_denom));
                frequencyPhaseResponse[j]
                    += atan2(re_denom * im_numer - re_numer * im_denom, re_numer * re_denom + im_numer * im_denom);
            }
        }
    }
    // (-pi, pi]の範囲に丸め込む
    for (int j = 0; j < frequencyResponseBins; j++) {
        frequencyPhaseResponse[j] = fmod(frequencyPhaseResponse[j], MathConstants<double>::pi);
    }
}

void AE2NBandsEqualizerAudioProcessor::parameterChanged(const String &parameterID, float newValue)
{
    // バンド数が変わったら全フィルタ係数を再計算
    if (parameterID == "nbands") {
        for (int i = 0; i < newValue; i++) {
            recalculateFilterCoefficients(i);
        }
        return;
    }

    // "_"の後にある数字からバンドを特定
    int start;
    if ((start = parameterID.indexOf("_")) != -1) {
        const int band = parameterID.substring(start + 1).getIntValue();
        recalculateFilterCoefficients(band);
    }

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AE2NBandsEqualizerAudioProcessor();
}
