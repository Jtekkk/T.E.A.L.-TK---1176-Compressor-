#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"
#include "gui/LookAndFeel1176.h"
#include "gui/GainReductionMeter.h"
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

    using APVTS  = juce::AudioProcessorValueTreeState;
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

    Knob inputKnob, outputKnob, attackKnob, releaseKnob, mixKnob;
    std::unique_ptr<SliderAtt> inputAtt, outputAtt, attackAtt, releaseAtt, mixAtt;

    teal::RatioSelector       ratioSelector;
    juce::Label               ratioLabel;

    juce::ComboBox            osBox;
    juce::Label               osLabel;
    std::unique_ptr<ComboAtt> osAtt;

    juce::ToggleButton        bypassButton { "Bypass" };
    std::unique_ptr<ButtonAtt> bypassAtt;

    teal::GainReductionMeter  meter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TEAL1176AudioProcessorEditor)
};
