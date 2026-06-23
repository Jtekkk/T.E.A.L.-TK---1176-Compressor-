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
    setupKnob (scHpfKnob,   "SC HPF");

    inputAtt   = std::make_unique<SliderAtt> (p.apvts, pid::input,   inputKnob.slider);
    outputAtt  = std::make_unique<SliderAtt> (p.apvts, pid::output,  outputKnob.slider);
    attackAtt  = std::make_unique<SliderAtt> (p.apvts, pid::attack,  attackKnob.slider);
    releaseAtt = std::make_unique<SliderAtt> (p.apvts, pid::release, releaseKnob.slider);
    mixAtt     = std::make_unique<SliderAtt> (p.apvts, pid::mix,     mixKnob.slider);
    scHpfAtt   = std::make_unique<SliderAtt> (p.apvts, pid::scHpf,   scHpfKnob.slider);

    addAndMakeVisible (ratioSelector);
    ratioLabel.setText ("RATIO", dontSendNotification);
    ratioLabel.setJustificationType (Justification::centred);
    ratioLabel.setColour (Label::textColourId, teal::Palette::textDim());
    ratioLabel.setFont (Font (FontOptions (13.0f, Font::bold)));
    addAndMakeVisible (ratioLabel);

    osBox.addItemList ({ "1x", "2x", "4x", "8x" }, 1);
    addAndMakeVisible (osBox);
    osAtt = std::make_unique<ComboAtt> (p.apvts, pid::os, osBox);

    osQualityBox.addItemList ({ "Low Latency", "Linear Phase" }, 1);
    addAndMakeVisible (osQualityBox);
    osQualityAtt = std::make_unique<ComboAtt> (p.apvts, pid::osQuality, osQualityBox);

    osLabel.setText ("OVERSAMPLING", dontSendNotification);
    osLabel.setJustificationType (Justification::centred);
    osLabel.setColour (Label::textColourId, teal::Palette::textDim());
    osLabel.setFont (Font (FontOptions (12.0f, Font::bold)));
    addAndMakeVisible (osLabel);

    for (auto* b : { &linkButton, &extScButton })
    {
        b->setColour (ToggleButton::textColourId, teal::Palette::text());
        b->setColour (ToggleButton::tickColourId, teal::Palette::accent());
        b->setColour (ToggleButton::tickDisabledColourId, teal::Palette::panelEdge());
        addAndMakeVisible (b);
    }
    linkAtt  = std::make_unique<ButtonAtt> (p.apvts, pid::link,  linkButton);
    extScAtt = std::make_unique<ButtonAtt> (p.apvts, pid::extSc, extScButton);

    bypassButton.setColour (ToggleButton::textColourId, teal::Palette::textDim());
    bypassButton.setColour (ToggleButton::tickColourId, teal::Palette::accentRed());
    bypassButton.setColour (ToggleButton::tickDisabledColourId, teal::Palette::panelEdge());
    addAndMakeVisible (bypassButton);
    bypassAtt = std::make_unique<ButtonAtt> (p.apvts, pid::bypass, bypassButton);

    // Preset selector.
    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (proc.getProgramName (i), i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, dontSendNotification);
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        if (id > 0) proc.setCurrentProgram (id - 1);
    };
    addAndMakeVisible (presetBox);
    for (auto* b : { &prevPreset, &nextPreset })
    {
        b->setColour (TextButton::buttonColourId, Colour (0xff2a2b30));
        b->setColour (TextButton::textColourOffId, teal::Palette::text());
        addAndMakeVisible (b);
    }
    prevPreset.onClick = [this] { stepPreset (-1); };
    nextPreset.onClick = [this] { stepPreset (+1); };

    addAndMakeVisible (meter);
    addAndMakeVisible (inputMeter);
    addAndMakeVisible (outputMeter);

    setSize (880, 560);
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
    k.slider.setTextBoxStyle (Slider::TextBoxBelow, false, 74, 20);
    k.slider.setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);
    addAndMakeVisible (k.slider);

    k.label.setText (name, dontSendNotification);
    k.label.setJustificationType (Justification::centred);
    k.label.setColour (Label::textColourId, teal::Palette::textDim());
    k.label.setFont (Font (FontOptions (13.0f, Font::bold)));
    addAndMakeVisible (k.label);
}

void TEAL1176AudioProcessorEditor::stepPreset (int delta)
{
    const int n = proc.getNumPrograms();
    if (n <= 0) return;
    const int idx = jlimit (0, n - 1, proc.getCurrentProgram() + delta);
    proc.setCurrentProgram (idx);
    presetBox.setSelectedId (idx + 1, dontSendNotification);
}

void TEAL1176AudioProcessorEditor::refreshPresetBox()
{
    const int want = proc.getCurrentProgram() + 1;
    if (presetBox.getSelectedId() != want)
        presetBox.setSelectedId (want, dontSendNotification);
}

void TEAL1176AudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (teal::Palette::background());

    auto full = getLocalBounds();
    auto title = full.removeFromTop (64);

    g.setColour (teal::Palette::panel());
    g.fillRect (title);
    g.setColour (teal::Palette::panelEdge());
    g.fillRect (title.getX(), title.getBottom() - 2, title.getWidth(), 2);

    auto t = title.reduced (18, 0);
    g.setColour (teal::Palette::text());
    g.setFont (Font (FontOptions (26.0f, Font::bold)));
    g.drawText ("T.E.A.L.  1176", t.removeFromLeft (210), Justification::centredLeft);
    g.setColour (teal::Palette::accent());
    g.setFont (Font (FontOptions (11.0f, Font::bold)));
    g.drawText ("FET  COMPRESSOR / LIMITER", t.removeFromLeft (180).withTrimmedTop (2),
                Justification::centredLeft);

    // Column + strip panels.
    auto content = getLocalBounds();
    content.removeFromTop (64);
    auto bottom = content.removeFromBottom (78);
    content = content.reduced (14, 10);

    auto panel = [&g] (Rectangle<int> a)
    {
        g.setColour (teal::Palette::panel());
        g.fillRoundedRectangle (a.toFloat(), 8.0f);
        g.setColour (teal::Palette::panelEdge());
        g.drawRoundedRectangle (a.toFloat(), 8.0f, 1.2f);
    };
    panel (content.removeFromLeft (212));
    panel (content.removeFromRight (212));
    panel (content.reduced (10, 0));
    panel (bottom.reduced (14, 8));
}

void TEAL1176AudioProcessorEditor::resized()
{
    auto placeKnob = [] (Knob& k, Rectangle<int> area)
    {
        k.label.setBounds (area.removeFromTop (18));
        k.slider.setBounds (area);
    };

    auto full = getLocalBounds();
    full.removeFromTop (64);
    auto bottom = full.removeFromBottom (78);
    auto content = full.reduced (14, 10);

    // --- title-bar widgets --------------------------------------------------
    bypassButton.setBounds (getWidth() - 130, 20, 110, 24);
    {
        auto pb = Rectangle<int> (440, 18, 235, 28);
        prevPreset.setBounds (pb.removeFromLeft (28));
        pb.removeFromLeft (4);
        nextPreset.setBounds (pb.removeFromRight (28));
        pb.removeFromRight (4);
        presetBox.setBounds (pb);
    }

    // --- columns ------------------------------------------------------------
    auto left   = content.removeFromLeft (212).reduced (10, 8);
    auto right  = content.removeFromRight (212).reduced (10, 8);
    auto centre = content.reduced (18, 4);

    // Left: INPUT (+ meter), MIX.
    {
        auto top = left.removeFromTop (210);
        inputMeter.setBounds (top.removeFromRight (14).reduced (0, 16));
        top.removeFromRight (6);
        placeKnob (inputKnob, top);
        placeKnob (mixKnob, left.reduced (18, 6));
    }

    // Right: OUTPUT (+ meter), SC HPF.
    {
        auto top = right.removeFromTop (210);
        outputMeter.setBounds (top.removeFromLeft (14).reduced (0, 16));
        top.removeFromLeft (6);
        placeKnob (outputKnob, top);
        placeKnob (scHpfKnob, right.reduced (18, 6));
    }

    // Centre: GR meter, ATTACK/RELEASE, RATIO.
    meter.setBounds (centre.removeFromTop (150).reduced (4, 4));
    auto ar = centre.removeFromTop (150);
    placeKnob (attackKnob,  ar.removeFromLeft (ar.getWidth() / 2).reduced (12, 6));
    placeKnob (releaseKnob, ar.reduced (12, 6));
    auto ratioArea = centre.reduced (10, 4);
    ratioLabel.setBounds (ratioArea.removeFromTop (18));
    ratioArea.removeFromTop (4);
    ratioSelector.setBounds (ratioArea.removeFromTop (40));

    // --- bottom strip: toggles + oversampling -------------------------------
    auto bs = bottom.reduced (26, 18);
    auto lhs = bs.removeFromLeft (bs.getWidth() / 2);
    linkButton.setBounds  (lhs.removeFromLeft (150).withSizeKeepingCentre (150, 26));
    lhs.removeFromLeft (10);
    extScButton.setBounds (lhs.removeFromLeft (130).withSizeKeepingCentre (130, 26));

    auto rhs = bs;
    osLabel.setBounds (rhs.removeFromLeft (110).withSizeKeepingCentre (110, 22));
    rhs.removeFromLeft (8);
    osBox.setBounds (rhs.removeFromLeft (90).withSizeKeepingCentre (90, 28));
    rhs.removeFromLeft (10);
    osQualityBox.setBounds (rhs.removeFromLeft (150).withSizeKeepingCentre (150, 28));
}

void TEAL1176AudioProcessorEditor::timerCallback()
{
    meter.setGainReductionDb (proc.gainReductionDb.load());
    inputMeter.setLevelDb (proc.inputLevelDb.load());
    outputMeter.setLevelDb (proc.outputLevelDb.load());
    ratioSelector.refresh();
    refreshPresetBox();
}
