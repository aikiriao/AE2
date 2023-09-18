/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AE2SpectrumAnalyzerAudioProcessorEditor::AE2SpectrumAnalyzerAudioProcessorEditor (
        AE2SpectrumAnalyzerAudioProcessor& p, AudioProcessorValueTreeState &vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
    // バイパスボタン
    bypassButton.setButtonText("Bypass");
    bypassButtonAttachment.reset(new ButtonAttachment(valueTreeState, "bypass", bypassButton));
    addAndMakeVisible(bypassButton);

    // 処理チャンネル選択ボックス
    const AudioChannelSet set = audioProcessor.getChannelLayoutOfBus(true, 0);
    for (int ch = 0; ch < audioProcessor.getChannelCountOfBus(true, 0); ch++) {
        const String chName = set.getAbbreviatedChannelTypeName(set.getTypeOfChannel(ch));
        analyzeChannelBox.addItem(chName, ch + 1);
    }
    analyzeChannelBoxAttachment.reset(new ComboBoxAttachment(valueTreeState, "analyzeChannel", analyzeChannelBox));
    addAndMakeVisible(analyzeChannelBox);
    // 処理チャンネル選択ボックスのラベル
    analyzeChannelBoxLabel.setText("CH", juce::dontSendNotification);
    analyzeChannelBoxLabel.setFont(Font(13.0f, Font::plain));
    analyzeChannelBoxLabel.attachToComponent(&analyzeChannelBox, true);
    addAndMakeVisible(analyzeChannelBoxLabel);

    // FFTサイズ選択ボックス
    fftSizeBox.addItemList(juce::StringArray("256", "512", "1024", "2048", "4096", "8192", "16384"), 1);
    fftSizeBoxAttachment.reset(new ComboBoxAttachment(valueTreeState, "fftSize", fftSizeBox));
    addAndMakeVisible(fftSizeBox);
    // FFTサイズ選択ボックスのラベル
    fftSizeBoxLabel.setText("FFT Size", juce::dontSendNotification);
    fftSizeBoxLabel.setFont(Font(13.0f, Font::plain));
    fftSizeBoxLabel.attachToComponent(&fftSizeBox, true);
    addAndMakeVisible(fftSizeBoxLabel);

    // 窓関数選択ボックス
    windowFunctionBox.addItemList(juce::StringArray("rectangular", "hann", "hamming", "blackman"), 1);
    windowFunctionBoxAttachment.reset(new ComboBoxAttachment(valueTreeState, "windowFunction", windowFunctionBox));
    addAndMakeVisible(windowFunctionBox);
    // 窓関数選択ボックスのラベル
    windowFunctionBoxLabel.setText("Window", juce::dontSendNotification);
    windowFunctionBoxLabel.setFont(Font(13.0f, Font::plain));
    windowFunctionBoxLabel.attachToComponent(&windowFunctionBox, true);
    addAndMakeVisible(windowFunctionBoxLabel);

    // スライド幅選択ボックス
    slideSizeMsBox.addItemList(juce::StringArray("5", "10", "20", "40", "80", "160"), 1);
    slideSizeMsBoxAttachment.reset(new ComboBoxAttachment(valueTreeState, "slideSizeMs", slideSizeMsBox));
    addAndMakeVisible(slideSizeMsBox);
    // スライド幅選択ボックスのラベル
    slideSizeMsBoxLabel.setText("Slide", juce::dontSendNotification);
    slideSizeMsBoxLabel.setFont(Font(13.0f, Font::plain));
    slideSizeMsBoxLabel.attachToComponent(&slideSizeMsBox, true);
    addAndMakeVisible(slideSizeMsBoxLabel);

    // ピークホールドボタン
    peakHoldButton.setButtonText("Peak");
    peakHoldButtonAttachment.reset(new ButtonAttachment(valueTreeState, "peakHold", peakHoldButton));
    addAndMakeVisible(peakHoldButton);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);

    startTimerHz(60);
}

AE2SpectrumAnalyzerAudioProcessorEditor::~AE2SpectrumAnalyzerAudioProcessorEditor()
{
}

//==============================================================================
void AE2SpectrumAnalyzerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    drawFrame(g);
}

void AE2SpectrumAnalyzerAudioProcessorEditor::resized()
{
    const int width = getWidth();
    const int height = getHeight();

    bypassButton.setBounds(0, 5, 80, 20);
    analyzeChannelBox.setBounds(100, 5, 55, 20);
    fftSizeBox.setBounds(210, 5, 85, 20);
    windowFunctionBox.setBounds(350, 5, 100, 20);
    slideSizeMsBox.setBounds(490, 5, 70, 20);
    peakHoldButton.setBounds(600, 5, 80, 20);
}

void AE2SpectrumAnalyzerAudioProcessorEditor::timerCallback()
{
    repaint();
}

void AE2SpectrumAnalyzerAudioProcessorEditor::drawFrame(juce::Graphics &g)
{
    const int width = getWidth();
    const int height = getHeight();
    const int numWaveXTicks = 5;
    const int numSpectrumYTicks = 5;
    const float fontSize = 12.0f;
    const int fftSize = audioProcessor.getFFTSizeParameterInt();
    const int scopeSize = fftSize / 2;
    const float maxdB = 0.0f;
    const float mindB = -120.0f;
    const float minDisplayFreqencyLog = log10f(minDisplayFrequency);
    const float maxDisplayFreqencyLog = log10f(maxDisplayFrequency);
    const juce::Colour frameColor = juce::Colours::red;
    const juce::Colour lineColor = juce::Colour(0, 0xD4, 0);

    const Rectangle<int> waveArea({ 20, 30 }, { width - 5, (height / 4) - 10 });
    const Rectangle<int> spectrumArea({ 20, height / 4 }, { width - 5, height - 10});

    // グリッド線
    g.setColour(frameColor);
    const float dashPattern[2] = {2.0f, 2.0f};
    // 振幅0の線 
    g.drawDashedLine(
        { static_cast<float>(waveArea.getTopLeft().x), static_cast<float>(waveArea.getCentreY()),
        static_cast<float>(waveArea.getTopRight().x), static_cast<float>(waveArea.getCentreY()) },
        dashPattern, 2);
    // スペクトルの縦線 
    for (int i = 1; i < numWaveXTicks; i++) {
        const float lineX = waveArea.getTopLeft().x + (i * waveArea.getWidth()) / numWaveXTicks;
        g.drawDashedLine(
            { lineX, static_cast<float>(waveArea.getTopLeft().y), lineX, static_cast<float>(waveArea.getBottomRight().y) },
            dashPattern, 2);
    }
    // スペクトルの横線 
    for (int i = 1; i < numSpectrumYTicks; i++) {
        const float lineY = spectrumArea.getTopLeft().y + (i * spectrumArea.getHeight()) / numSpectrumYTicks;
        g.drawDashedLine(
            { static_cast<float>(spectrumArea.getTopLeft().x), lineY, static_cast<float>(spectrumArea.getBottomRight().x), lineY },
            dashPattern, 2);
    }

    // x軸グリッドとラベル
    if (freqScaleType == Linear) {
        for (int hz = (minDisplayFrequency / 1000) * 1000 + 1000;
            hz < maxDisplayFrequency;
            hz += 1000) {
            const float gridX = (float)juce::jmap((float)hz,
                (float)minDisplayFrequency, (float)maxDisplayFrequency, (float)spectrumArea.getTopLeft().x, (float)spectrumArea.getTopRight().x);
            g.setColour(frameColor);
            g.drawDashedLine(
                { gridX, static_cast<float>(spectrumArea.getTopLeft().y), gridX, static_cast<float>(spectrumArea.getBottomRight().y) },
                dashPattern, 2);
            if ((hz % 2000) == 0) {
                const float stringY = spectrumArea.getBottomRight().y;
                g.setFont(fontSize);
                g.setColour(lineColor);
                g.drawText(String(hz / 1000) + "k", gridX - fontSize, stringY, 2 * fontSize, 10, Justification::centred);
            }
        }
    } else if (freqScaleType == Log) {
        int deltaHz = minDisplayFrequency;
        int hz = minDisplayFrequency + deltaHz; // 最低周波数は右枠で表示済み 
        while (hz < maxDisplayFrequency) {
            const float gridX = (float)juce::jmap(log10f(hz),
                minDisplayFreqencyLog, maxDisplayFreqencyLog, (float)spectrumArea.getTopLeft().x, (float)spectrumArea.getTopRight().x);
            g.setColour(frameColor);
            g.drawDashedLine(
                { gridX, static_cast<float>(spectrumArea.getTopLeft().y), gridX, static_cast<float>(spectrumArea.getBottomRight().y) },
                dashPattern, 2);
            if (hz >= (10 * deltaHz)) {
                const float stringY = spectrumArea.getBottomRight().y;
                int displayHz = hz;
                juce::String suffix = "";
                if (hz >= 1000) {
                    displayHz /= 1000;
                    suffix += "k";
                }
                g.setFont(fontSize);
                g.setColour(lineColor);
                g.drawText(String(displayHz) + suffix, gridX - fontSize, stringY, 2 * fontSize, 10, Justification::centred);
                deltaHz *= 10;
            }
            hz += deltaHz;
        }
    } else {
        jassertfalse;
    }

    // グラフ軸ラベル(ticks)
    g.setColour(lineColor);
    g.setFont(fontSize);
    g.drawText( "1", 0,    waveArea.getTopLeft().y - 6, waveArea.getTopLeft().x - 3, 10, Justification::centredRight);
    g.drawText( "0", 0,      waveArea.getCentreY() - 6, waveArea.getTopLeft().x - 3, 10, Justification::centredRight);
    g.drawText("-1", 0, waveArea.getBottomLeft().y - 6, waveArea.getTopLeft().x - 3, 10, Justification::centredRight);
    for (int i = 1; i < numWaveXTicks; i++) {
        const float stringX = waveArea.getTopLeft().x + (i * waveArea.getWidth()) / numWaveXTicks - (2 * fontSize);
        const float stringY = waveArea.getBottomRight().y;
        const float ms = (i * fftSize * 1000.0) / (audioProcessor.sampleRate * numWaveXTicks);
        g.drawText(String(ms, 1), stringX, stringY, 4 * fontSize, 10, Justification::centred);
    }
    for (int i = 1; i < numSpectrumYTicks; i++) {
        const float stringY = spectrumArea.getTopLeft().y + (i * spectrumArea.getHeight()) / numSpectrumYTicks - 6;
        const int dB = -(i * (maxdB - mindB)) / numSpectrumYTicks;
        g.drawText(String(dB), 0, stringY, spectrumArea.getTopLeft().x - 3, 10, Justification::centred);
    }

    audioProcessor.analyzeLock.enterRead();

    // 解析結果
    // 波形
    g.setColour(lineColor);
    {
        const float *wave = audioProcessor.analyzedWave;
        const float rightEdge = waveArea.getTopRight().x;
        const float leftEdge = waveArea.getTopLeft().x;
        const float topEdge = waveArea.getTopLeft().y;
        const float bottomEdge = waveArea.getBottomLeft().y;
        float waveX, waveY;
        juce::Path path;
        constexpr int drawPointCounts = 2048; // 描画する最大点数（負荷が高いため点数制限） 
        const int stride = jmax(1, fftSize / drawPointCounts); // インデックスの飛ばし幅 
        waveX = jmap(0.0f, 0.0f, fftSize - 1.0f, leftEdge, rightEdge);
        waveY = jlimit(topEdge, bottomEdge, jmap(wave[0], -1.0f, 1.0f, bottomEdge, topEdge));
        path.startNewSubPath(waveX, waveY);
        for (int i = 1; i < fftSize; i += stride) {
            waveX = jmap((float)i, 0.0f, fftSize - 1.0f, leftEdge, rightEdge);
            waveY = jlimit(topEdge, bottomEdge, jmap(wave[i], -1.0f, 1.0f, bottomEdge, topEdge));
            path.lineTo(waveX, waveY);
        }
        g.strokePath(path, juce::PathStrokeType(1.0f));
    }

    // スペクトル
    g.setColour(lineColor);
    drawSpectrum(g, spectrumArea, audioProcessor.analyzedSpectrum, fftSize,
        audioProcessor.sampleRate, mindB, maxdB, minDisplayFrequency, maxDisplayFrequency, freqScaleType);
    // スペクトルピーク
    if (audioProcessor.peakHoldEnable) {
        g.setColour(frameColor);
        drawSpectrum(g, spectrumArea, audioProcessor.peakSpectrum, fftSize,
            audioProcessor.sampleRate, mindB, maxdB, minDisplayFrequency, maxDisplayFrequency, freqScaleType);
    }

    audioProcessor.analyzeLock.exitRead();

    // 外枠
    // 最後に書くことで解析結果が枠の上に書かれることを防ぐ
    g.setColour(frameColor);
    g.drawRect(waveArea, 1.5f);
    g.drawRect(spectrumArea, 1.5f);
}

void AE2SpectrumAnalyzerAudioProcessorEditor::drawSpectrum(juce::Graphics &g, const juce::Rectangle<int> &area, const float *spec,
    const int fftSize, const double sampleRate, const float mindB, const float maxdB,
    const int minDisplayFrequency, const int maxDisplayFrequency, const int scale)
{
    const int rightEdge = area.getTopRight().x;
    const int leftEdge = area.getTopLeft().x;
    const int topEdge = area.getTopLeft().y;
    const int bottomEdge = area.getBottomLeft().y;
    const int scopeSize = fftSize / 2;
    const float minDisplayFrequencyLog = log10f(minDisplayFrequency);
    const float maxDisplayFrequencyLog = log10f(maxDisplayFrequency);

    float fromX, fromY;
    float specX, specY;
    int i;

    // 最低周波数に達するまでインデックスを読み飛ばし
    for (i = 1; i < scopeSize; i++) {
        const float hz = (i * sampleRate) / fftSize;
        if (hz >= minDisplayFrequency) {
            break;
        }
    }


    // 最低周波数成分のプロット
    {
        const float hz = (i * sampleRate) / fftSize;
        if (scale == Linear) {
            fromX = (float)juce::jmap(hz, (float)minDisplayFrequency, (float)maxDisplayFrequency, (float)leftEdge, (float)rightEdge);
        } else if (scale == Log) {
            const float hz = (i * sampleRate) / fftSize;
            fromX = (float)juce::jmap(log10f(hz), minDisplayFrequencyLog, maxDisplayFrequencyLog, (float)leftEdge, (float)rightEdge);
        }
        else {
            jassertfalse;
        }
        fromY = juce::jmap(juce::jlimit(mindB, maxdB, spec[i]), mindB, maxdB, (float)bottomEdge, (float)topEdge);
    }

    // 最低周波数以上の成分のプロット
    for (; i < scopeSize; i++) {
        const float hz = (i * sampleRate) / fftSize;
        if (hz > maxDisplayFrequency) {
            break;
        }
        if (scale == Linear) {
            specX = (float)juce::jmap(hz, (float)minDisplayFrequency, (float)maxDisplayFrequency, (float)leftEdge, (float)rightEdge);
        } else if (scale == Log) {
            specX = (float)juce::jmap(log10f(hz), minDisplayFrequencyLog, maxDisplayFrequencyLog, (float)leftEdge, (float)rightEdge);
        } else {
            jassertfalse;
        }
        specY = juce::jmap(juce::jlimit(mindB, maxdB, spec[i]), mindB, maxdB, (float)bottomEdge, (float)topEdge);

        // drawLineでは16384点以上の描画で60fpsに間に合わなかったので高速化
        // drawVerticalLineが高速であることを利用し、横軸の変化分が小さければdrawVerticalLineに置き換える
        if (fabs(specX - fromX) > 0.2f) {
            g.drawLine(fromX, fromY, specX, specY);
        } else {
            g.drawVerticalLine((fromX + specX) * 0.5f, jmin(fromY, specY), jmax(fromY, specY));
        }

        fromX = specX; fromY = specY;
    }
}

void AE2SpectrumAnalyzerAudioProcessorEditor::mouseDown(const MouseEvent &e)
{
    if (e.mods.isRightButtonDown())
    {
        PopupMenu scaleMenu;
        scaleMenu.addItem( 1, "Linear", true, freqScaleType == Linear);
        scaleMenu.addItem( 2,    "Log", true, freqScaleType == Log);

        PopupMenu minRangeMenu;
        static const int minFreqList[5] = { 0, 1, 10, 100, 1000 };
        for (int i = 0; i < 5; i++) {
            const int freq = minFreqList[i];
            minRangeMenu.addItem(3 + i , String(freq), !((freq == 0) && (freqScaleType == Log)), minDisplayFrequency == freq);
        }

        PopupMenu maxRangeMenu;
        static const int maxFreqList[7] = { 4000, 8000, 12000, 16000, 20000, 24000, 40000 };
        const int nyquistFrequency = static_cast<int>(audioProcessor.sampleRate / 2.0);
        for (int i = 0; i < 7; i++) {
            const int freq = maxFreqList[i];
            maxRangeMenu.addItem(8 + i, String(freq),
                freq <= nyquistFrequency, maxDisplayFrequency == freq);
        }
        maxRangeMenu.addItem(15, "Nyquist", true, maxDisplayFrequency == nyquistFrequency);

        PopupMenu rangeMenu;
        rangeMenu.addSubMenu("Min", minRangeMenu);
        rangeMenu.addSubMenu("Max", maxRangeMenu);

        PopupMenu mainMenu;
        mainMenu.addSubMenu("Frequency axis scale", scaleMenu);
        mainMenu.addSubMenu("Frequency range", rangeMenu);

        mainMenu.showMenuAsync(PopupMenu::Options(),
            [this](int result)
            {
                switch (result) {
                case 0: break;
                case 1: case 2:
                    if (result == 1) {
                        freqScaleType = Linear;
                    } else if (result == 2) {
                        freqScaleType = Log;
                        // 対数軸で直流成分を表示すると見ずらくなるため、0を回避 
                        if (minDisplayFrequency == 0) {
                            minDisplayFrequency = 10;
                        }
                    }
                    break;
                case 3: case 4: case 5: case 6: case 7:
                    minDisplayFrequency = minFreqList[result - 3];
                    break;
                case 8: case 9: case 10:  case 11: case 12:  case 13: case 14:
                    maxDisplayFrequency = maxFreqList[result - 8];
                    break;
                case 15:
                    maxDisplayFrequency = static_cast<int>(audioProcessor.sampleRate / 2.0);;
                    break;
                default: jassertfalse;
                }
            });
    }
}
