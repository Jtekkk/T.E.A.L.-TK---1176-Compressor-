#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel1176.h"

// =============================================================================
//  GainReductionMeter  --  VU-style needle showing gain reduction (0 .. -20 dB).
//  The host calls setGainReductionDb() with the engine's GR; the needle is
//  ballistically smoothed for an analogue feel.
// =============================================================================
namespace teal
{

class GainReductionMeter : public juce::Component
{
public:
    GainReductionMeter() = default;

    // grDb is <= 0. Smooths toward the target each UI tick.
    void setGainReductionDb (float grDb) noexcept
    {
        targetGr = juce::jlimit (-24.0f, 0.0f, grDb);
        // Asymmetric ballistics: snap toward more reduction, ease back.
        const float coeff = (targetGr < shownGr) ? 0.6f : 0.18f;
        shownGr += coeff * (targetGr - shownGr);
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;
        auto b = getLocalBounds().toFloat().reduced (2.0f);

        // Meter face.
        g.setColour (Colour (0xff20211f));
        g.fillRoundedRectangle (b, 6.0f);
        ColourGradient face (Colour (0xfff2ead4), b.getCentreX(), b.getY(),
                             Colour (0xffd9cfb2), b.getCentreX(), b.getBottom(), false);
        g.setGradientFill (face);
        g.fillRoundedRectangle (b.reduced (4.0f), 5.0f);
        g.setColour (Colours::black.withAlpha (0.55f));
        g.drawRoundedRectangle (b.reduced (4.0f), 5.0f, 1.4f);

        const Point<float> pivot (b.getCentreX(), b.getBottom() - 8.0f);
        const float needleLen = b.getHeight() * 0.82f;
        const float maxAng = 0.86f;   // radians each side of vertical

        // GR magnitude 0..20 maps right..left.
        auto angleFor = [maxAng] (float grMag)
        {
            const float t = jlimit (0.0f, 1.0f, grMag / 20.0f);
            return maxAng - t * (2.0f * maxAng);   // +maxAng (0 GR) -> -maxAng (20 GR)
        };

        // Scale arc + ticks.
        g.setColour (Colour (0xff36352f));
        Path arc;
        const float arcR = needleLen * 0.96f;
        arc.addCentredArc (pivot.x, pivot.y, arcR, arcR, 0.0f, -maxAng, maxAng, true);
        g.strokePath (arc, PathStrokeType (1.2f));

        g.setFont (Font (FontOptions (10.0f)));
        for (int dbMark = 0; dbMark <= 20; dbMark += 5)
        {
            const float a  = angleFor ((float) dbMark);
            const float ca = std::cos (a), sa = std::sin (a);
            const Point<float> p1 (pivot.x + sa * (arcR - 8.0f), pivot.y - ca * (arcR - 8.0f));
            const Point<float> p2 (pivot.x + sa * arcR,          pivot.y - ca * arcR);
            g.setColour (dbMark >= 15 ? Palette::accentRed().darker (0.2f) : Colour (0xff36352f));
            g.drawLine ({ p1, p2 }, dbMark % 10 == 0 ? 1.8f : 1.1f);

            const Point<float> tp (pivot.x + sa * (arcR - 20.0f), pivot.y - ca * (arcR - 20.0f));
            g.drawText (String (dbMark), Rectangle<float> (22, 12).withCentre (tp),
                        Justification::centred);
        }

        // "GR" / "VU" labels.
        g.setColour (Colour (0xff6b6657));
        g.setFont (Font (FontOptions (11.0f, Font::italic)));
        g.drawText ("GAIN REDUCTION  dB", b.reduced (10.0f).removeFromTop (16.0f),
                    Justification::centredTop);

        // Needle.
        const float a  = angleFor (-shownGr);
        const float ca = std::cos (a), sa = std::sin (a);
        const Point<float> tip (pivot.x + sa * needleLen, pivot.y - ca * needleLen);
        g.setColour (Colours::black.withAlpha (0.85f));
        g.drawLine ({ pivot, tip }, 2.2f);
        g.setColour (Palette::accentRed());
        const Point<float> tail (pivot.x + sa * (needleLen * 0.7f), pivot.y - ca * (needleLen * 0.7f));
        g.drawLine ({ tail, tip }, 2.2f);

        // Hub.
        g.setColour (Colour (0xff2a2a2a));
        g.fillEllipse (Rectangle<float> (12, 12).withCentre (pivot));
        g.setColour (Colours::black);
        g.drawEllipse (Rectangle<float> (12, 12).withCentre (pivot), 1.0f);
    }

private:
    float targetGr { 0.0f };
    float shownGr  { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainReductionMeter)
};

} // namespace teal
