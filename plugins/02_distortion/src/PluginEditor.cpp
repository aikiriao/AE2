/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AE2DistortionAudioProcessorEditor::AE2DistortionAudioProcessorEditor (
    AE2DistortionAudioProcessor& p, AudioProcessorValueTreeState &vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
    // バイパスボタン
    bypassButton.setButtonText("Bypass");
    bypassButtonAttachment.reset(new ButtonAttachment(valueTreeState, "bypass", bypassButton));
    addAndMakeVisible(bypassButton);

    // 入力ゲインのスライダー
    inGainSlider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    inGainSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    inGainSliderAttachment.reset(new SliderAttachment(valueTreeState, "inGain", inGainSlider));
    addAndMakeVisible(inGainSlider);
    // 入力ゲインのラベル
    inGainSliderLabel.setText("Input Gain", juce::dontSendNotification);
    inGainSliderLabel.setFont(Font(13.0f, Font::plain));
    inGainSliderLabel.setJustificationType(Justification::centredTop);
    inGainSliderLabel.attachToComponent(&inGainSlider, false);
    addAndMakeVisible(inGainSliderLabel);

    // 出力レベルのスライダー
    outLevelSlider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    outLevelSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    outLevelSliderAttachment.reset(new SliderAttachment(valueTreeState, "outLevel", outLevelSlider));
    addAndMakeVisible(outLevelSlider);
    // 出力レベルのラベル
    outLevelSliderLabel.setText("Output Level", juce::dontSendNotification);
    outLevelSliderLabel.setFont(Font(13.0f, Font::plain));
    outLevelSliderLabel.setJustificationType(Justification::centredTop);
    outLevelSliderLabel.attachToComponent(&outLevelSlider, false);
    addAndMakeVisible(outLevelSliderLabel);

    // クリッピング曲線選択ボックス
    clippingCurveBox.addItemList(juce::StringArray("linear", "atan", "tanh", "exp"), 1);
    clippingCurveBoxAttachment.reset(new ComboBoxAttachment(valueTreeState, "clippingCurve", clippingCurveBox));
    addAndMakeVisible(clippingCurveBox);
    // 出力レベルのラベル
    clippingCurveBoxLabel.setText("Clipping Curve", juce::dontSendNotification);
    clippingCurveBoxLabel.setFont(Font(13.0f, Font::plain));
    clippingCurveBoxLabel.attachToComponent(&clippingCurveBox, true);
    addAndMakeVisible(clippingCurveBoxLabel);

    // 振幅変換手法選択ボックス
    amplifiterBox.addItemList(juce::StringArray("linear", "full-wave", "half-wave", "square"), 1);
    amplifiterBoxAttachment.reset(new ComboBoxAttachment(valueTreeState, "amplifierConversion", amplifiterBox));
    addAndMakeVisible(amplifiterBox);
    // 振幅変換手法選択ボックスのラベル
    amplifiterBoxLabel.setText("Ampl. Conversion", juce::dontSendNotification);
    amplifiterBoxLabel.setFont(Font(13.0f, Font::plain));
    amplifiterBoxLabel.attachToComponent(&amplifiterBox, true);
    addAndMakeVisible(amplifiterBoxLabel);

    // ゲインのdB表記 
    inGainSlider.textFromValueFunction = [](double value)
        {
            double db = 20.0 * log10(value);
            return juce::String(db) + "dB";
        };

    // ゲインのdB表記を振幅に直す 
    inGainSlider.valueFromTextFunction = [](const String &text)
        {
            double db = text.removeCharacters("dB").getDoubleValue();
            return pow(10.0, db / 20.0);
        };

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (200, 230);
}

AE2DistortionAudioProcessorEditor::~AE2DistortionAudioProcessorEditor()
{
}

//==============================================================================
void AE2DistortionAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AE2DistortionAudioProcessorEditor::resized()
{
    const int width = getWidth();
    const int height = getHeight();

    bypassButton.setBounds((width / 2) - 40, 0, 80, 30);
    inGainSlider.setBounds(0, 50, 100, 100);
    outLevelSlider.setBounds(width / 2, 50, 100, 100);
    clippingCurveBox.setBounds(width / 2, 160, 100, 30);
    amplifiterBox.setBounds(width / 2, 190, 100, 30);
}
