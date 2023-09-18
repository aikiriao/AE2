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
class AE2AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AE2AudioProcessorEditor (AE2AudioProcessor&);
    ~AE2AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AE2AudioProcessor& audioProcessor;

    // impulse audio file
    File impulseFile;

    // impulse wav file format manager
    AudioFormatManager formatManager;

    TextButton openButton;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;

    std::unique_ptr<juce::FileChooser> chooser;

    void openButtonClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AE2AudioProcessorEditor)
};
