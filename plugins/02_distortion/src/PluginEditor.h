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
class AE2DistortionAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AE2DistortionAudioProcessorEditor (AE2DistortionAudioProcessor&, AudioProcessorValueTreeState &vts);
    ~AE2DistortionAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AE2DistortionAudioProcessor& audioProcessor;

    // 型名が長いため型エイリアスを使用 
    using ButtonAttachment = AudioProcessorValueTreeState::ButtonAttachment;
    using SliderAttachment = AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = AudioProcessorValueTreeState::ComboBoxAttachment;

    AudioProcessorValueTreeState &valueTreeState;

    ToggleButton bypassButton;
    std::unique_ptr<ButtonAttachment> bypassButtonAttachment;
    Slider inGainSlider;
    std::unique_ptr<SliderAttachment> inGainSliderAttachment;
    Label inGainSliderLabel;
    Slider outLevelSlider;
    std::unique_ptr<SliderAttachment> outLevelSliderAttachment;
    Label outLevelSliderLabel;
    ComboBox clippingCurveBox;
    std::unique_ptr<ComboBoxAttachment> clippingCurveBoxAttachment;
    Label clippingCurveBoxLabel;
    ComboBox amplifiterBox;
    std::unique_ptr<ComboBoxAttachment> amplifiterBoxAttachment;
    Label amplifiterBoxLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AE2DistortionAudioProcessorEditor)
};
