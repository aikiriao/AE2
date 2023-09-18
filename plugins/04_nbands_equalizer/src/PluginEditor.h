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
class AE2NBandsEqualizerAudioProcessorEditor  : public juce::AudioProcessorEditor, Slider::Listener, ComboBox::Listener, Button::Listener 
{
public:
    AE2NBandsEqualizerAudioProcessorEditor (AE2NBandsEqualizerAudioProcessor&, AudioProcessorValueTreeState &vts);
    ~AE2NBandsEqualizerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // ボタン変化時 
    void buttonClicked(Button *box) override;
    // スライダー変化時 
    void sliderValueChanged(Slider *slider) override;
    // コンボボックス変化時 
    void comboBoxChanged(ComboBox *box) override;

private:
    static constexpr int bandGUIWidth = 100; //! バンドあたり要素の幅 
    static constexpr int minBandsGUIWidth = 3 * bandGUIWidth; //! バンドGUIの最小幅 
    static constexpr int maxEqualizerBandsCount = AE2NBandsEqualizerAudioProcessor::maxEqualizerBandsCount;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AE2NBandsEqualizerAudioProcessor& audioProcessor;

    // 型名が長いため型エイリアスを使用
    using ButtonAttachment = AudioProcessorValueTreeState::ButtonAttachment;
    using SliderAttachment = AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = AudioProcessorValueTreeState::ComboBoxAttachment;

    AudioProcessorValueTreeState &valueTreeState;

    ToggleButton bypassButton;
    std::unique_ptr<ButtonAttachment> bypassButtonAttachment;

    //! 処理バンド数選択ボックス
    ComboBox activeFilterBox;
    std::unique_ptr<ComboBoxAttachment> activeFilterBoxAttachment;
    Label activeFilterBoxLabel;

    struct BandGUIComponent {
        //! 有効ボタン 
        ToggleButton activeButton;
        std::unique_ptr<ButtonAttachment> activeButtonAttachment;
        //! タイプ選択ボックス
        ComboBox typeBox;
        std::unique_ptr<ComboBoxAttachment> typeBoxAttachment;
        //! 周波数スライダー
        Slider frequencySlider;
        std::unique_ptr<SliderAttachment> frequencySliderAttachment;
        //! クオリティファクタスライダー
        Slider qvalueSlider;
        std::unique_ptr<SliderAttachment> qvalueSliderAttachment;
        Label qvalueSliderLabel;
        //! ゲインスライダー
        Slider gainSlider;
        std::unique_ptr<SliderAttachment> gainSliderAttachment;
        Label gainSliderLabel;
    };

    // 全バンド分のGUI要素 
    struct BandGUIComponent bandsGUI[maxEqualizerBandsCount];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AE2NBandsEqualizerAudioProcessorEditor)
};
