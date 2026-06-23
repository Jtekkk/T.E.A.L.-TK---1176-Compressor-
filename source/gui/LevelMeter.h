#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel1176.h"

// =============================================================================
//  LevelMeter  --  thin vertical peak meter (dBFS), green->amber->red, with a
//  short peak-hold. Fed from the processor's level atomics on the UI timer.
// =============================================================================
namespace teal
{

class LevelMeter : public juce::Component
{
public:
    LevelMeter() = default;

    void setLevelDb (float db) noexcept
    {
        const float target = juce::jlimit (kFloor, 6.0f, db);
        shown = (target > shown) ? target : shown + 0.4f * (target - shown); // fast up, slow down
        if (target >= peakHold) { peakHold = target; peakAge = 0; }
        else if (++peakAge > 18)  peakHold = std::max (kFloor, peakHold - 1.5f);
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;
        auto b = getLocalBounds().toFloat();

        g.setColour (Colour (0xff0c0c0e));
        g.fillRoundedRectangle (b, 3.0f);

        auto inner = b.reduced (2.0f);
        auto norm  = [this] (float db) { return jlimit (0.0f, 1.0f, (db - kFloor) / (6.0f - kFloor)); };

        const float fillTop = inner.getBottom() - norm (shown) * inner.getHeight();
        Rectangle<float> fill (inner.getX(), fillTop, inner.getWidth(), inner.getBottom() - fillTop);

        ColourGradient grad (Colour (0xff37b24d), inner.getX(), inner.getBottom(),
                             Palette::accentRed(), inner.getX(), inner.getY(), false);
        grad.addColour (0.7,  Colour (0xffd9a441));
        g.setGradientFill (grad);
        g.fillRect (fill);

        // 0 dBFS line.
        const float zeroY = inner.getBottom() - norm (0.0f) * inner.getHeight();
        g.setColour (Palette::accentRed().withAlpha (0.6f));
        g.drawLine (inner.getX(), zeroY, inner.getRight(), zeroY, 1.0f);

        // Peak hold.
        const float py = inner.getBottom() - norm (peakHold) * inner.getHeight();
        g.setColour (Colours::white.withAlpha (0.85f));
        g.drawLine (inner.getX(), py, inner.getRight(), py, 1.4f);

        g.setColour (Palette::panelEdge());
        g.drawRoundedRectangle (b, 3.0f, 1.0f);
    }

private:
    static constexpr float kFloor = -54.0f;
    float shown    { kFloor };
    float peakHold { kFloor };
    int   peakAge  { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};

} // namespace teal
