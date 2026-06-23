#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel1178.h"

// =============================================================================
//  RatioSelector  --  the 1176's mutually-exclusive ratio push-buttons, plus
//  the "All" button (all-buttons-in / British mode). Bound to a choice param.
//  Call refresh() periodically so the buttons track host/automation changes.
// =============================================================================
namespace teal
{

class RatioSelector : public juce::Component
{
public:
    RatioSelector (juce::AudioProcessorValueTreeState& s, const juce::String& paramID)
        : state (s), id (paramID)
    {
        param = state.getParameter (id);
        jassert (param != nullptr);

        const juce::StringArray labels { "4", "8", "12", "20", "ALL" };
        for (int i = 0; i < labels.size(); ++i)
        {
            auto* b = buttons.add (new juce::TextButton (labels[i]));
            b->setClickingTogglesState (true);
            b->setRadioGroupId (1976);
            b->setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff2a2b30));
            b->setColour (juce::TextButton::buttonOnColourId, Palette::accentRed());
            b->setColour (juce::TextButton::textColourOffId,  Palette::textDim());
            b->setColour (juce::TextButton::textColourOnId,   juce::Colours::white);
            b->setConnectedEdges (
                (i > 0 ? juce::Button::ConnectedOnLeft  : 0) |
                (i < labels.size() - 1 ? juce::Button::ConnectedOnRight : 0));
            const int index = i;
            b->onClick = [this, index]
            {
                if (param != nullptr)
                {
                    param->beginChangeGesture();
                    param->setValueNotifyingHost (
                        param->convertTo0to1 ((float) index));
                    param->endChangeGesture();
                }
            };
            addAndMakeVisible (b);
        }
        refresh();
    }

    void refresh()
    {
        if (param == nullptr) return;
        const int current = (int) std::lround (
            param->convertFrom0to1 (param->getValue()));
        for (int i = 0; i < buttons.size(); ++i)
            buttons[i]->setToggleState (i == current, juce::dontSendNotification);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        const int w = r.getWidth() / buttons.size();
        for (int i = 0; i < buttons.size(); ++i)
            buttons[i]->setBounds (r.removeFromLeft (i == buttons.size() - 1 ? r.getWidth() : w));
    }

private:
    juce::AudioProcessorValueTreeState& state;
    juce::String id;
    juce::RangedAudioParameter* param { nullptr };
    juce::OwnedArray<juce::TextButton> buttons;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RatioSelector)
};

} // namespace teal
