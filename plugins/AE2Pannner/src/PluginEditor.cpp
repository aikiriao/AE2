/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AE2PannnerAudioProcessorEditor::AE2PannnerAudioProcessorEditor (
        AE2PannnerAudioProcessor& p, AudioProcessorValueTreeState &vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
    // バイパスボタン
    bypassButton.setButtonText("Bypass");
    bypassButtonAttachment.reset(new ButtonAttachment(valueTreeState, "bypass", bypassButton));
    addAndMakeVisible(bypassButton);

    // パンニング手法
    panningMethodBox.addItemList(StringArray("Amplitude", "SimpleHRTF"), 1);
    panningMethodBoxAttachment.reset(
        new ComboBoxAttachment(valueTreeState, "panningMethod", panningMethodBox));
    panningMethodBoxLabel.setText("Method", juce::dontSendNotification);
    panningMethodBoxLabel.setFont(Font(13.0f, Font::plain));
    panningMethodBoxLabel.attachToComponent(&panningMethodBox, true);
    addAndMakeVisible(panningMethodBox);
    addAndMakeVisible(panningMethodBoxLabel);

    // パンニング方向スライダー
    azimuthAngleSlider.setSliderStyle(Slider::RotaryHorizontalDrag);
    azimuthAngleSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    azimuthAngleSliderAttachment.reset(
        new SliderAttachment(valueTreeState, "azimuthAngle", azimuthAngleSlider));
    azimuthAngleSliderLabel.setText("Azimuth Angle", juce::dontSendNotification);
    azimuthAngleSliderLabel.setFont(Font(13.0f, Font::plain));
    azimuthAngleSliderLabel.attachToComponent(&azimuthAngleSlider, false);
    azimuthAngleSliderLabel.setJustificationType(Justification::centredTop);
    addAndMakeVisible(azimuthAngleSlider);
    addAndMakeVisible(azimuthAngleSliderLabel);
    auto range = NormalisableRange<double>(-180.0, 180.0,
        [](auto rangeStart, auto rangeEnd, auto normalised)
        { return jmap(normalised, rangeEnd, rangeStart); },
        [](auto rangeStart, auto rangeEnd, auto value)
        { return jmap(value, rangeEnd, rangeStart, 0.0, 1.0); });
    azimuthAngleSlider.setNormalisableRange(range);

    // 頭部実行半径スライダー
    radiusOfHeadSlider.setSliderStyle(Slider::LinearHorizontal);
    radiusOfHeadSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    radiusOfHeadSliderAttachment.reset(
        new SliderAttachment(valueTreeState, "radiusOfHead", radiusOfHeadSlider));
    radiusOfHeadSliderLabel.setText("Head Radius", juce::dontSendNotification);
    radiusOfHeadSliderLabel.setFont(Font(13.0f, Font::plain));
    radiusOfHeadSliderLabel.attachToComponent(&radiusOfHeadSlider, true);
    // radiusOfHeadSliderLabel.setJustificationType(Justification::centredTop);
    addAndMakeVisible(radiusOfHeadSlider);
    addAndMakeVisible(radiusOfHeadSliderLabel);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (170, 300);
}

AE2PannnerAudioProcessorEditor::~AE2PannnerAudioProcessorEditor()
{
}

//==============================================================================
void AE2PannnerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AE2PannnerAudioProcessorEditor::resized()
{
    const int width = getWidth();
    const int height = getHeight();

    bypassButton.setBounds(0, 0, 80, 30);
    panningMethodBox.setBounds(50, 30, 120, 30);
    azimuthAngleSlider.setBounds(10, 90, 150, 150);
    radiusOfHeadSlider.setBounds(50, 240, 120, 50);
}
