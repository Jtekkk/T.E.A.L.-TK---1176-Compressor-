#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// =============================================================================
//  LookAndFeel1176  --  blackface-1176-flavoured knobs and combo boxes.
// =============================================================================
namespace teal
{

struct Palette
{
    static juce::Colour background() { return juce::Colour (0xff17181b); }
    static juce::Colour panel()      { return juce::Colour (0xff232428); }
    static juce::Colour panelEdge()  { return juce::Colour (0xff3a3c42); }
    static juce::Colour text()       { return juce::Colour (0xffe9e7e1); }
    static juce::Colour textDim()    { return juce::Colour (0xff9a9aa0); }
    static juce::Colour accent()     { return juce::Colour (0xffd9a441); }  // warm amber
    static juce::Colour accentRed()  { return juce::Colour (0xffd2503e); }  // VU needle / limit
    static juce::Colour knob()       { return juce::Colour (0xff2c2d31); }
    static juce::Colour knobEdge()   { return juce::Colour (0xff09090b); }
};

class LookAndFeel1176 : public juce::LookAndFeel_V4
{
public:
    LookAndFeel1176()
    {
        setColour (juce::Slider::textBoxTextColourId,       Palette::text());
        setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff121215));
        setColour (juce::Label::textColourId,               Palette::text());
        setColour (juce::ComboBox::backgroundColourId,      juce::Colour (0xff121215));
        setColour (juce::ComboBox::textColourId,            Palette::text());
        setColour (juce::ComboBox::outlineColourId,         Palette::panelEdge());
        setColour (juce::ComboBox::arrowColourId,           Palette::accent());
        setColour (juce::PopupMenu::backgroundColourId,     Palette::panel());
        setColour (juce::PopupMenu::highlightedBackgroundColourId, Palette::accent().withAlpha (0.3f));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider&) override
    {
        using namespace juce;
        auto bounds = Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (5.0f);
        const auto radius = jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const auto angle  = startAngle + sliderPos * (endAngle - startAngle);

        // Tick marks around the dial.
        g.setColour (Palette::textDim().withAlpha (0.55f));
        const int   ticks = 11;
        for (int i = 0; i < ticks; ++i)
        {
            const float a  = startAngle + (float) i / (float) (ticks - 1) * (endAngle - startAngle);
            const float r1 = radius + 2.0f;
            const float r2 = radius + 6.0f;
            const Point<float> p1 (centre.x + r1 * std::sin (a), centre.y - r1 * std::cos (a));
            const Point<float> p2 (centre.x + r2 * std::sin (a), centre.y - r2 * std::cos (a));
            g.drawLine ({ p1, p2 }, 1.4f);
        }

        // Knob body.
        const auto knobR = radius - 1.0f;
        const auto knobBounds = Rectangle<float> (knobR * 2, knobR * 2).withCentre (centre);
        ColourGradient grad (Palette::knob().brighter (0.18f), centre.x, centre.y - knobR,
                             Palette::knob().darker (0.5f),     centre.x, centre.y + knobR, false);
        g.setGradientFill (grad);
        g.fillEllipse (knobBounds);
        g.setColour (Palette::knobEdge());
        g.drawEllipse (knobBounds, 2.0f);
        g.setColour (Colours::black.withAlpha (0.25f));
        g.drawEllipse (knobBounds.reduced (2.5f), 1.0f);

        // Indicator.
        Path pointer;
        const float pw = 3.4f;
        pointer.addRoundedRectangle (-pw * 0.5f, -knobR + 4.0f, pw, knobR * 0.62f, 1.6f);
        pointer.applyTransform (AffineTransform::rotation (angle).translated (centre));
        g.setColour (Palette::accent());
        g.fillPath (pointer);

        // Centre cap.
        g.setColour (Palette::knob().brighter (0.05f));
        g.fillEllipse (Rectangle<float> (knobR * 0.5f, knobR * 0.5f).withCentre (centre));
        g.setColour (Palette::knobEdge());
        g.drawEllipse (Rectangle<float> (knobR * 0.5f, knobR * 0.5f).withCentre (centre), 1.0f);
    }

    juce::Label* createSliderTextBox (juce::Slider& s) override
    {
        auto* l = LookAndFeel_V4::createSliderTextBox (s);
        l->setJustificationType (juce::Justification::centred);
        l->setFont (juce::Font (juce::FontOptions (13.0f)));
        return l;
    }

    juce::Font getLabelFont (juce::Label& l) override
    {
        return l.getFont().withHeight (juce::jlimit (12.0f, 16.0f, l.getFont().getHeight()));
    }
};

} // namespace teal
