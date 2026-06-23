#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"
#include "gui/LookAndFeel1176.h"
#include "gui/GainReductionMeter.h"
#include "gui/LevelMeter.h"
#include "gui/RatioSelector.h"

class TEAL1176AudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit TEAL1176AudioProcessorEditor (TEAL1176AudioProcessor&);
    ~TEAL1176AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshPresetBox();
    void stepPreset (int delta);

    using APVTS     = juce::AudioProcessorValueTreeState;
    using SliderAtt = APVTS::SliderAttachment;
    using ComboAtt  = APVTS::ComboBoxAttachment;
    using ButtonAtt = APVTS::ButtonAttachment;

    struct Knob
    {
        juce::Slider slider;
        juce::Label  label;
    };
    void setupKnob (Knob& k, const juce::String& name);

    TEAL1176AudioProcessor& proc;
    teal::LookAndFeel1176   lnf;

    Knob inputKnob, outputKnob, attackKnob, releaseKnob, mixKnob, scHpfKnob;
    std::unique_ptr<SliderAtt> inputAtt, outputAtt, attackAtt, releaseAtt, mixAtt, scHpfAtt;

    teal::RatioSelector       ratioSelector;
    juce::Label               ratioLabel;

    juce::ComboBox            osBox, osQualityBox;
    juce::Label               osLabel;
    std::unique_ptr<ComboAtt> osAtt, osQualityAtt;

    juce::ToggleButton        linkButton  { "Stereo Link" };
    juce::ToggleButton        extScButton { "Ext SC" };
    juce::ToggleButton        bypassButton { "Bypass" };
    std::unique_ptr<ButtonAtt> linkAtt, extScAtt, bypassAtt;

    juce::ComboBox            presetBox;
    juce::TextButton          prevPreset { "<" }, nextPreset { ">" };

    teal::GainReductionMeter  meter;
    teal::LevelMeter          inputMeter, outputMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TEAL1176AudioProcessorEditor)
};
