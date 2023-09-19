/*
==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cfloat>

//==============================================================================
AE2NBandsEqualizerAudioProcessorEditor::AE2NBandsEqualizerAudioProcessorEditor (
        AE2NBandsEqualizerAudioProcessor& p, AudioProcessorValueTreeState &vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
    // バイパスボタン
    bypassButton.setButtonText("Bypass");
    bypassButtonAttachment.reset(new ButtonAttachment(valueTreeState, "bypass", bypassButton));
    addAndMakeVisible(bypassButton);

    // 処理バンド数選択ボックス
    StringArray bandsLabel;
    for (int i = 1; i <= maxEqualizerBandsCount; i++) {
        bandsLabel.add(String(i));
    }
    activeFilterBox.addItemList(bandsLabel, 1);
    activeFilterBox.addListener(this);
    activeFilterBoxAttachment.reset(new ComboBoxAttachment(valueTreeState, "nbands", activeFilterBox));
    activeFilterBoxLabel.setText("Bands", juce::dontSendNotification);
    activeFilterBoxLabel.setFont(Font(13.0f, Font::plain));
    activeFilterBoxLabel.attachToComponent(&activeFilterBox, true);
    addAndMakeVisible(activeFilterBox);
    addAndMakeVisible(activeFilterBoxLabel);

    // 各バンドのパラメータ
    for (int i = 0; i < maxEqualizerBandsCount; i++) {
        struct BandGUIComponent &bandGUI = bandsGUI[i];
        bandGUI.activeButton.setButtonText("Band" + String(i + 1));
        bandGUI.activeButtonAttachment.reset(
            new ButtonAttachment(valueTreeState, String("active_") + String(i), bandGUI.activeButton));
        bandGUI.activeButton.addListener(this);
        addAndMakeVisible(bandGUI.activeButton);

        bandGUI.typeBox.addItemList(
            StringArray("LPF", "HPF", "BPF", "BEF", "APF", "LSF", "HSF"), 1);
        bandGUI.typeBoxAttachment.reset(
            new ComboBoxAttachment(valueTreeState, String("type_") + String(i), bandGUI.typeBox));
        bandGUI.typeBox.addListener(this);
        addAndMakeVisible(bandGUI.typeBox);

        bandGUI.frequencySlider.setSliderStyle(Slider::RotaryHorizontalDrag);
        bandGUI.frequencySlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
        bandGUI.frequencySlider.addListener(this);
        bandGUI.frequencySliderAttachment.reset(
            new SliderAttachment(valueTreeState, String("frequency_") + String(i), bandGUI.frequencySlider));
        bandGUI.frequencySlider.setTextValueSuffix("Hz");
        addAndMakeVisible(bandGUI.frequencySlider);

        bandGUI.qvalueSlider.setSliderStyle(Slider::LinearVertical);
        bandGUI.qvalueSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
        bandGUI.qvalueSlider.addListener(this);
        bandGUI.qvalueSliderAttachment.reset(
            new SliderAttachment(valueTreeState, String("qualityfactor_") + String(i), bandGUI.qvalueSlider));
        bandGUI.qvalueSliderLabel.setText("Q", juce::dontSendNotification);
        bandGUI.qvalueSliderLabel.setFont(Font(13.0f, Font::plain));
        bandGUI.qvalueSliderLabel.attachToComponent(&bandGUI.qvalueSlider, false);
        bandGUI.qvalueSliderLabel.setJustificationType(Justification::centredTop);
        addAndMakeVisible(bandGUI.qvalueSlider);
        addAndMakeVisible(bandGUI.qvalueSliderLabel);

        bandGUI.gainSlider.setSliderStyle(Slider::LinearVertical);
        bandGUI.gainSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
        bandGUI.gainSlider.addListener(this);
        bandGUI.gainSliderAttachment.reset(
            new SliderAttachment(valueTreeState, String("gain_") + String(i), bandGUI.gainSlider));
        bandGUI.gainSliderLabel.setText("Gain", juce::dontSendNotification);
        bandGUI.gainSliderLabel.setFont(Font(13.0f, Font::plain));
        bandGUI.gainSliderLabel.attachToComponent(&bandGUI.gainSlider, false);
        bandGUI.gainSliderLabel.setJustificationType(Justification::centredTop);
        addAndMakeVisible(bandGUI.gainSlider);
        addAndMakeVisible(bandGUI.gainSliderLabel);
    }

    // ウィンドウサイズ初期化
    const int activeBandCounts = static_cast<int>(*valueTreeState.getRawParameterValue("nbands"));
    setSize (jmax(minBandsGUIWidth, activeBandCounts * bandGUIWidth), 570);
}

AE2NBandsEqualizerAudioProcessorEditor::~AE2NBandsEqualizerAudioProcessorEditor()
{
}

//==============================================================================
void AE2NBandsEqualizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    const int width = getWidth();
    const Rectangle<float> frequencyResponseArea(5, 35, width - 10, 200);
    const double *ampli = audioProcessor.frequencyAmplitudeResponse;
    const double *phase = audioProcessor.frequencyPhaseResponse;

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // 周波数特性の描画
    {
        const float mindB = -60.0f;
        const float maxdB = 30.0f;
        const float minHz = 50.0f;
        const float maxHz = audioProcessor.samplingRate / 2.0;
        const float minHzLog = log10(minHz);
        const float maxHzLog = log10(maxHz);
        const float minPhase = -180.0f;
        const float maxPhase = 180.0f;
        const juce::Colour ampliLineColor = juce::Colour(0, 0xD4, 0);
        const juce::Colour phaseLineColor = juce::Colour(0, 0, 0xD4);
        const juce::Colour frameColor = juce::Colours::red;
        const juce::Colour backColor = juce::Colours::black;
        const float fontSize = 12.0f;
        const float leftEdge = frequencyResponseArea.getBottomLeft().x;
        const float rightEdge = frequencyResponseArea.getBottomRight().x;
        const float topEdge = frequencyResponseArea.getTopLeft().y;
        const float bottomEdge = frequencyResponseArea.getBottomLeft().y;
        const float numBins = audioProcessor.frequencyResponseBins;

        // 背景を黒で塗る
        g.setColour(backColor);
        g.fillRect(frequencyResponseArea);

        // 目盛りの描画
        const float dashPattern[2] = { 2.0f, 2.0f };
        // スペクトルの横線
        for (int dB = mindB; dB <= maxdB; dB += 10) {
            const float lineY = jmap(static_cast<float>(dB), mindB, maxdB, bottomEdge, topEdge);
            g.setColour(frameColor);
            g.drawDashedLine({ leftEdge, lineY, rightEdge, lineY }, dashPattern, 2);
            g.setFont(fontSize);
            g.setColour(juce::Colours::white);
            g.drawText(String(dB), leftEdge, lineY - 5, 2 * fontSize, 10, Justification::centred);
        }
        // 位相のテキスト
        g.drawText(String(-180), rightEdge - 2 * fontSize,                 bottomEdge - 5, 2 * fontSize, 10, Justification::centred);
        g.drawText(String(   0), rightEdge - 2 * fontSize, (bottomEdge + topEdge) / 2 - 5, 2 * fontSize, 10, Justification::centred);
        g.drawText(String( 180), rightEdge - 2 * fontSize,                    topEdge - 5, 2 * fontSize, 10, Justification::centred);

        // スペクトルの縦線
        int hz = minHz;
        int hz_unit = static_cast<int>(pow(10.0, floor(log10(minHz))));
        while (hz < maxHz) {
            const float lineX = jmap(log10f(hz), minHzLog, maxHzLog, leftEdge, rightEdge);
            g.setColour(frameColor);
            g.drawDashedLine({ lineX, bottomEdge, lineX, topEdge }, dashPattern, 2);
            if (hz % (hz_unit * 5) == 0) {
                int displayHz = hz;
                juce::String suffix = "";
                if (hz >= 1000) {
                    displayHz /= 1000;
                    suffix += "k";
                }
                g.setFont(fontSize);
                g.setColour(juce::Colours::white);
                g.drawText(String(displayHz) + suffix, lineX - fontSize, bottomEdge, 2 * fontSize, 10, Justification::centred);
            }
            if (hz >= (hz_unit * 10)) {
                hz_unit *= 10;
            }
            hz += hz_unit;
        }

        // 特性のグラフを描画
        {
            float fromX = jmap(log10f(FLT_MIN), minHzLog, maxHzLog, leftEdge, rightEdge);
            float amplFromY = jmap(static_cast<float>(ampli[0]), mindB, maxdB, bottomEdge, topEdge);
            float phaseFromY = jmap(static_cast<float>(phase[0] * 180.0f / MathConstants<float>::pi), minPhase, maxPhase, bottomEdge, topEdge);
            for (int i = 1; i < numBins; i++) {
                float binHz = i * audioProcessor.samplingRate / (2 * numBins);
                float toX = jmap(log10f(binHz), minHzLog, maxHzLog, leftEdge, rightEdge);
                float amplToY = jmap(static_cast<float>(ampli[i]), mindB, maxdB, bottomEdge, topEdge);
                float phaseToY = jmap(static_cast<float>(phase[i] * 180.0f / MathConstants<float>::pi), minPhase, maxPhase, bottomEdge, topEdge);
                // 位相を先に描く（おそらく振幅の方が重要）
                if (frequencyResponseArea.contains(Point<float>({ fromX, phaseFromY }))
                    && frequencyResponseArea.contains(Point<float>({ toX, phaseToY }))) {
                    g.setColour(phaseLineColor);
                    g.drawLine(fromX, phaseFromY, toX, phaseToY, 1.5f);
                }
                // 振幅特性
                if (frequencyResponseArea.contains(Point<float>({ fromX, amplFromY }))
                    && frequencyResponseArea.contains(Point<float>({ toX, amplToY }))) {
                    g.setColour(ampliLineColor);
                    g.drawLine(fromX, amplFromY, toX, amplToY, 1.5f);
                }
                fromX = toX;
                amplFromY = amplToY;
                phaseFromY = phaseToY;
            }
        }
    }
}

void AE2NBandsEqualizerAudioProcessorEditor::resized()
{
    const int width = getWidth();
    const int bandTopY = 250;

    bypassButton.setBounds(0, 0, 80, 30);
    activeFilterBox.setBounds(width - 60, 0, 60, 30);

    // イコライザ要素の描画
    for (int i = 0; i < activeFilterBox.getSelectedId(); i++) {
        struct BandGUIComponent &bandGUI = bandsGUI[i];
        const int bandX = i * bandGUIWidth;
        bandGUI.activeButton.setBounds(bandX + 10, bandTopY, 80, 30);
        bandGUI.typeBox.setBounds(bandX + 10, bandTopY + 30, 80, 30);
        bandGUI.frequencySlider.setBounds(bandX, bandTopY + 60, 100, 100);
        bandGUI.qvalueSlider.setBounds(bandX, bandTopY + 190, 50, 130);
        bandGUI.gainSlider.setBounds(bandX + 50, bandTopY + 190, 50, 130);
    }

    // 表示/非表示の切り替え　
    for (int i = 0; i < maxEqualizerBandsCount; i++) {
        struct BandGUIComponent &bandGUI = bandsGUI[i];
        const bool visible = i < activeFilterBox.getSelectedId();
        bandGUI.activeButton.setVisible(visible);
        bandGUI.typeBox.setVisible(visible);
        bandGUI.frequencySlider.setVisible(visible);
        bandGUI.gainSlider.setVisible(visible);
        bandGUI.qvalueSlider.setVisible(visible);
    }
}

void AE2NBandsEqualizerAudioProcessorEditor::buttonClicked(Button *box)
{
    // 周波数特性が変わるため再描画
    repaint();
}

void AE2NBandsEqualizerAudioProcessorEditor::sliderValueChanged(Slider *slider)
{
    // 周波数特性が変わるため再描画
    repaint();
}

void AE2NBandsEqualizerAudioProcessorEditor::comboBoxChanged(ComboBox *box)
{
    const int numActiveBands = activeFilterBox.getSelectedId();

    // 有効なバンド数が変化
    if (box == &activeFilterBox) {
        setSize(juce::jmax(minBandsGUIWidth, numActiveBands * bandGUIWidth), getHeight());
        resized();
        repaint();
        return;
    }

    // バンドの種類変更
    for (int i = 0; i < numActiveBands; i++) {
        if (box == &bandsGUI[i].typeBox) {
            repaint();
            return;
        }
    }

    // ここにはこないはず
    jassertfalse;
}
