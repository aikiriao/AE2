/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class AE2SpectrumAnalyzerAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    AE2SpectrumAnalyzerAudioProcessorEditor (AE2SpectrumAnalyzerAudioProcessor&, AudioProcessorValueTreeState &vts);
    ~AE2SpectrumAnalyzerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void mouseDown(const MouseEvent &) override;

    // 軸のスケール
    enum ScaleType {
        Linear = 1,
        Log = 2,
    };

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AE2SpectrumAnalyzerAudioProcessor& audioProcessor;

    // 型名が長いため型エイリアスを使用
    using ButtonAttachment = AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = AudioProcessorValueTreeState::ComboBoxAttachment;

    AudioProcessorValueTreeState &valueTreeState;

    ToggleButton bypassButton;
    std::unique_ptr<ButtonAttachment> bypassButtonAttachment;
    ComboBox analyzeChannelBox;
    std::unique_ptr<ComboBoxAttachment> analyzeChannelBoxAttachment;
    Label analyzeChannelBoxLabel;
    ComboBox fftSizeBox;
    std::unique_ptr<ComboBoxAttachment> fftSizeBoxAttachment;
    Label fftSizeBoxLabel;
    ComboBox windowFunctionBox;
    std::unique_ptr<ComboBoxAttachment> windowFunctionBoxAttachment;
    Label windowFunctionBoxLabel;
    ComboBox slideSizeMsBox;
    std::unique_ptr<ComboBoxAttachment> slideSizeMsBoxAttachment;
    Label slideSizeMsBoxLabel;
    ToggleButton peakHoldButton;
    std::unique_ptr<ButtonAttachment> peakHoldButtonAttachment;

    ScaleType freqScaleType = Log;
    int minDisplayFrequency = 10;
    int maxDisplayFrequency = 12000;

    void drawFrame(juce::Graphics &g);
    void drawSpectrum(juce::Graphics &g, const juce::Rectangle<int> &area, const float *spec,
        const int fftSize, const double sampleRate, const float mindB, const float maxdB, const int minFrequency, const int maxFrequency, const int scale);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AE2SpectrumAnalyzerAudioProcessorEditor)
};
