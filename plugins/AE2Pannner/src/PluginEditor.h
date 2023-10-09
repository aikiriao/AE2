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
class AE2PannnerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AE2PannnerAudioProcessorEditor (AE2PannnerAudioProcessor&, AudioProcessorValueTreeState &vts);
    ~AE2PannnerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AE2PannnerAudioProcessor& audioProcessor;

    // 型名が長いため型エイリアスを使用
    using ButtonAttachment = AudioProcessorValueTreeState::ButtonAttachment;
    using SliderAttachment = AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = AudioProcessorValueTreeState::ComboBoxAttachment;

    AudioProcessorValueTreeState &valueTreeState;

    ToggleButton bypassButton;
    std::unique_ptr<ButtonAttachment> bypassButtonAttachment;

    //! パンニング手法選択ボックス
    ComboBox panningMethodBox;
    std::unique_ptr<ComboBoxAttachment> panningMethodBoxAttachment;
    Label panningMethodBoxLabel;
    //! 水平角度スライダー
    Slider azimuthAngleSlider;
    std::unique_ptr<SliderAttachment> azimuthAngleSliderAttachment;
    Label azimuthAngleSliderLabel;
    //! 頭部半径スライダー
    Slider radiusOfHeadSlider;
    std::unique_ptr<SliderAttachment> radiusOfHeadSliderAttachment;
    Label radiusOfHeadSliderLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AE2PannnerAudioProcessorEditor)
};
