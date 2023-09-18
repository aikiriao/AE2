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
class AE2TemplateAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AE2TemplateAudioProcessorEditor (AE2TemplateAudioProcessor&, AudioProcessorValueTreeState &vts);
    ~AE2TemplateAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AE2TemplateAudioProcessor& audioProcessor;

    // 型名が長いため型エイリアスを使用
    using ButtonAttachment = AudioProcessorValueTreeState::ButtonAttachment;

    AudioProcessorValueTreeState &valueTreeState;

    ToggleButton bypassButton;
    std::unique_ptr<ButtonAttachment> bypassButtonAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AE2TemplateAudioProcessorEditor)
};
