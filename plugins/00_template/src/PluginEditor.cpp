/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AE2TemplateAudioProcessorEditor::AE2TemplateAudioProcessorEditor (
        AE2TemplateAudioProcessor& p, AudioProcessorValueTreeState &vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
    // バイパスボタン
    bypassButton.setButtonText("Bypass");
    bypassButtonAttachment.reset(new ButtonAttachment(valueTreeState, "bypass", bypassButton));
    addAndMakeVisible(bypassButton);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

AE2TemplateAudioProcessorEditor::~AE2TemplateAudioProcessorEditor()
{
}

//==============================================================================
void AE2TemplateAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void AE2TemplateAudioProcessorEditor::resized()
{
    const int width = getWidth();
    const int height = getHeight();

    bypassButton.setBounds((width / 2) - 40, 0, 80, 30);
}
