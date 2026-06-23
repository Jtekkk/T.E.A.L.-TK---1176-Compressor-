#include "PluginEditor.h"

using namespace juce;

TEAL1176AudioProcessorEditor::TEAL1176AudioProcessorEditor (TEAL1176AudioProcessor& p)
    : AudioProcessorEditor (&p),
      proc (p),
      ratioSelector (p.apvts, pid::ratio)
{
    setLookAndFeel (&lnf);

    setupKnob (inputKnob,   "INPUT");
    setupKnob (outputKnob,  "OUTPUT");
    setupKnob (attackKnob,  "ATTACK");
    setupKnob (releaseKnob, "RELEASE");
    setupKnob (mixKnob,     "MIX");

    inputAtt   = std::make_unique<SliderAtt> (p.apvts, pid::input,   inputKnob.slider);
    outputAtt  = std::make_unique<SliderAtt> (p.apvts, pid::output,  outputKnob.slider);
    attackAtt  = std::make_unique<SliderAtt> (p.apvts, pid::attack,  attackKnob.slider);
    releaseAtt = std::make_unique<SliderAtt> (p.apvts, pid::release, releaseKnob.slider);
    mixAtt     = std::make_unique<SliderAtt> (p.apvts, pid::mix,     mixKnob.slider);

    addAndMakeVisible (ratioSelector);
    ratioLabel.setText ("RATIO", dontSendNotification);
    ratioLabel.setJustificationType (Justification::centred);
    ratioLabel.setColour (Label::textColourId, teal::Palette::textDim());
    ratioLabel.setFont (Font (FontOptions (13.0f, Font::bold)));
    addAndMakeVisible (ratioLabel);

    osBox.addItemList ({ "1x", "2x", "4x", "8x" }, 1);
    addAndMakeVisible (osBox);
    osAtt = std::make_unique<ComboAtt> (p.apvts, pid::os, osBox);
    osLabel.setText ("OVERSAMPLING", dontSendNotification);
    osLabel.setJustificationType (Justification::centred);
    osLabel.setColour (Label::textColourId, teal::Palette::textDim());
    osLabel.setFont (Font (FontOptions (13.0f, Font::bold)));
    addAndMakeVisible (osLabel);

    bypassButton.setColour (ToggleButton::textColourId,   teal::Palette::textDim());
    bypassButton.setColour (ToggleButton::tickColourId,   teal::Palette::accentRed());
    bypassButton.setColour (ToggleButton::tickDisabledColourId, teal::Palette::panelEdge());
    addAndMakeVisible (bypassButton);
    bypassAtt = std::make_unique<ButtonAtt> (p.apvts, pid::bypass, bypassButton);

    addAndMakeVisible (meter);

    setSize (780, 480);
    setResizable (false, false);
    startTimerHz (30);
}

TEAL1176AudioProcessorEditor::~TEAL1176AudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void TEAL1176AudioProcessorEditor::setupKnob (Knob& k, const String& name)
{
    k.slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle (Slider::TextBoxBelow, false, 76, 20);
    k.slider.setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);
    addAndMakeVisible (k.slider);

    k.label.setText (name, dontSendNotification);
    k.label.setJustificationType (Justification::centred);
    k.label.setColour (Label::textColourId, teal::Palette::textDim());
    k.label.setFont (Font (FontOptions (13.0f, Font::bold)));
    addAndMakeVisible (k.label);
}

void TEAL1176AudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (teal::Palette::background());

    auto r = getLocalBounds();
    auto title = r.removeFromTop (60);

    g.setColour (teal::Palette::panel());
    g.fillRect (title);
    g.setColour (teal::Palette::panelEdge());
    g.fillRect (title.getX(), title.getBottom() - 1, title.getWidth(), 2);

    auto t = title.reduced (18, 0);
    g.setColour (teal::Palette::text());
    g.setFont (Font (FontOptions (27.0f, Font::bold)));
    g.drawText ("T.E.A.L.  1176", t.removeFromLeft (300), Justification::centredLeft);

    g.setColour (teal::Palette::accent());
    g.setFont (Font (FontOptions (12.0f, Font::bold)));
    g.drawText ("FET  COMPRESSOR  /  LIMITER", t.removeFromLeft (260),
                Justification::centredLeft);

    // Subtle group panels behind the centre and side columns.
    auto content = getLocalBounds();
    content.removeFromTop (60);
    content = content.reduced (16, 12);

    auto drawPanel = [&g] (Rectangle<int> area)
    {
        g.setColour (teal::Palette::panel());
        g.fillRoundedRectangle (area.toFloat(), 8.0f);
        g.setColour (teal::Palette::panelEdge());
        g.drawRoundedRectangle (area.toFloat(), 8.0f, 1.2f);
    };
    drawPanel (content.removeFromLeft (168));
    drawPanel (content.removeFromRight (168));
    drawPanel (content.reduced (10, 0));
}

void TEAL1176AudioProcessorEditor::resized()
{
    auto placeKnob = [] (Knob& k, Rectangle<int> area)
    {
        k.label.setBounds (area.removeFromTop (18));
        k.slider.setBounds (area);
    };

    auto r = getLocalBounds();
    r.removeFromTop (60);
    auto content = r.reduced (16, 12);

    bypassButton.setBounds (getWidth() - 116, 19, 96, 24);

    auto left   = content.removeFromLeft (168);
    auto right  = content.removeFromRight (168);
    auto center = content.reduced (10, 0);

    // Left column: INPUT (top), MIX (bottom).
    placeKnob (inputKnob, left.removeFromTop (196).reduced (16, 10));
    placeKnob (mixKnob,   left.reduced (28, 12));

    // Right column: OUTPUT (top), OVERSAMPLING (bottom).
    placeKnob (outputKnob, right.removeFromTop (196).reduced (16, 10));
    {
        auto ob = right.reduced (18, 14);
        osLabel.setBounds (ob.removeFromTop (18));
        ob.removeFromTop (6);
        osBox.setBounds (ob.removeFromTop (30).reduced (6, 0));
    }

    // Centre column: meter, ATTACK / RELEASE, RATIO.
    meter.setBounds (center.removeFromTop (150).reduced (8, 6));
    auto ar = center.removeFromTop (150);
    placeKnob (attackKnob,  ar.removeFromLeft (ar.getWidth() / 2).reduced (10, 6));
    placeKnob (releaseKnob, ar.reduced (10, 6));

    auto ratioArea = center.reduced (10, 4);
    ratioLabel.setBounds (ratioArea.removeFromTop (18));
    ratioArea.removeFromTop (4);
    ratioSelector.setBounds (ratioArea.removeFromTop (40));
}

void TEAL1176AudioProcessorEditor::timerCallback()
{
    meter.setGainReductionDb (proc.gainReductionDb.load());
    ratioSelector.refresh();
}
