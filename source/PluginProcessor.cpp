#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

using APVTS = juce::AudioProcessorValueTreeState;

//==============================================================================
APVTS::ParameterLayout TEAL1178AudioProcessor::createParameterLayout()
{
    using namespace juce;
    APVTS::ParameterLayout layout;

    const auto dbLabel = AudioParameterFloatAttributes().withLabel ("dB");

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { pid::input, 1 }, "Input",
        NormalisableRange<float> (-20.0f, 40.0f, 0.01f), 0.0f, dbLabel));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { pid::output, 1 }, "Output",
        NormalisableRange<float> (-20.0f, 40.0f, 0.01f), 0.0f, dbLabel));

    {
        NormalisableRange<float> r (20.0f, 800.0f, 0.1f);     // microseconds
        r.setSkewForCentre (120.0f);
        layout.add (std::make_unique<AudioParameterFloat> (
            ParameterID { pid::attack, 1 }, "Attack", r, 250.0f,
            AudioParameterFloatAttributes().withLabel ("\xc2\xb5s")));  // "µs"
    }
    {
        NormalisableRange<float> r (50.0f, 1100.0f, 1.0f);    // milliseconds
        r.setSkewForCentre (300.0f);
        layout.add (std::make_unique<AudioParameterFloat> (
            ParameterID { pid::release, 1 }, "Release", r, 400.0f,
            AudioParameterFloatAttributes().withLabel ("ms")));
    }

    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { pid::ratio, 1 }, "Ratio",
        StringArray { "4:1", "8:1", "12:1", "20:1", "All" }, 0));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { pid::mix, 1 }, "Mix",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f,
        AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { pid::os, 1 }, "Oversampling",
        StringArray { "1x", "2x", "4x", "8x" }, 2));

    layout.add (std::make_unique<AudioParameterBool> (
        ParameterID { pid::bypass, 1 }, "Bypass", false));

    return layout;
}

//==============================================================================
TEAL1178AudioProcessor::TEAL1178AudioProcessor()
    : AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    pInput   = apvts.getRawParameterValue (pid::input);
    pOutput  = apvts.getRawParameterValue (pid::output);
    pAttack  = apvts.getRawParameterValue (pid::attack);
    pRelease = apvts.getRawParameterValue (pid::release);
    pRatio   = apvts.getRawParameterValue (pid::ratio);
    pMix     = apvts.getRawParameterValue (pid::mix);
    pOs      = apvts.getRawParameterValue (pid::os);
    pBypass  = apvts.getRawParameterValue (pid::bypass);
}

//==============================================================================
bool TEAL1178AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in  = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();

    if (out != in)
        return false;

    return out == juce::AudioChannelSet::mono()
        || out == juce::AudioChannelSet::stereo();
}

//==============================================================================
void TEAL1178AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    baseSampleRate = sampleRate;

    const int ch = juce::jlimit (1, 2,
        juce::jmax (getTotalNumInputChannels(), getTotalNumOutputChannels()));

    engine.prepare (sampleRate, ch);

    dryBuffer.setSize (ch, samplesPerBlock, false, false, true);

    constexpr double rampSeconds = 0.02;
    inputGain .reset (sampleRate, rampSeconds);
    outputGain.reset (sampleRate, rampSeconds);
    mixAmount .reset (sampleRate, rampSeconds);
    inputGain .setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pInput->load()));
    outputGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pOutput->load()));
    mixAmount .setCurrentAndTargetValue (pMix->load() * 0.01f);

    using OS = juce::dsp::Oversampling<float>;
    auto makeOs = [ch, samplesPerBlock] (int log2factor)
    {
        auto o = std::make_unique<OS> ((size_t) ch, (size_t) log2factor,
                                       OS::filterHalfBandPolyphaseIIR);
        o->initProcessing ((size_t) samplesPerBlock);
        o->reset();
        return o;
    };
    os2 = makeOs (1);
    os4 = makeOs (2);
    os8 = makeOs (3);

    lastOsChoice  = -1;
    lastRatioMode = -1;
    setLatencySamples (0);
}

//==============================================================================
void TEAL1178AudioProcessor::processEngine (juce::dsp::AudioBlock<float>& block)
{
    const int n  = (int) block.getNumSamples();
    const int ch = juce::jmin ((int) block.getNumChannels(), 2);

    float* chan[2] = { nullptr, nullptr };
    for (int c = 0; c < ch; ++c)
        chan[c] = block.getChannelPointer ((size_t) c);

    double frame[2] = { 0.0, 0.0 };
    float  minGr = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        for (int c = 0; c < ch; ++c) frame[c] = (double) chan[c][i];
        engine.processFrame (frame, ch);
        for (int c = 0; c < ch; ++c) chan[c][i] = (float) frame[c];

        const float gr = (float) engine.getGainReductionDb();
        if (gr < minGr) minGr = gr;
    }

    gainReductionDb.store (minGr);
}

//==============================================================================
void TEAL1178AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int totalIn  = getTotalNumInputChannels();
    const int totalOut = getTotalNumOutputChannels();
    for (int i = totalIn; i < totalOut; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const int n     = buffer.getNumSamples();
    const int numCh = juce::jmin (buffer.getNumChannels(), 2);
    if (n == 0 || numCh == 0)
        return;

    if (pBypass->load() > 0.5f)
    {
        gainReductionDb.store (0.0f);
        return;
    }

    if (dryBuffer.getNumSamples() < n || dryBuffer.getNumChannels() < numCh)
        dryBuffer.setSize (numCh, n, false, false, true);  // defensive; rare

    // --- parameters (block rate) -------------------------------------------
    const int   osChoice  = (int) pOs->load();
    const int   ratioMode = (int) pRatio->load();
    const float attUs     = pAttack->load();
    const float relMs     = pRelease->load();

    inputGain .setTargetValue (juce::Decibels::decibelsToGain (pInput->load()));
    outputGain.setTargetValue (juce::Decibels::decibelsToGain (pOutput->load()));
    mixAmount .setTargetValue (pMix->load() * 0.01f);

    const int factor = (osChoice == 0) ? 1 : (osChoice == 1) ? 2 : (osChoice == 2) ? 4 : 8;
    engine.setSampleRate (baseSampleRate * factor);
    engine.setTimes (attUs * 1.0e-6, relMs * 1.0e-3);
    if (ratioMode != lastRatioMode)
    {
        engine.setRatioMode (static_cast<teal::Ratio> (ratioMode));
        lastRatioMode = ratioMode;
    }

    // --- capture dry, apply input drive ------------------------------------
    for (int c = 0; c < numCh; ++c)
        dryBuffer.copyFrom (c, 0, buffer, c, 0, n);

    for (int i = 0; i < n; ++i)
    {
        const float g = inputGain.getNextValue();
        for (int c = 0; c < numCh; ++c)
            buffer.getWritePointer (c)[i] *= g;
    }

    // --- nonlinear engine, oversampled -------------------------------------
    juce::dsp::AudioBlock<float> block (buffer);
    block = block.getSubsetChannelBlock (0, (size_t) numCh);

    if (osChoice == 0)
    {
        processEngine (block);
    }
    else
    {
        auto* o = (osChoice == 1) ? os2.get() : (osChoice == 2) ? os4.get() : os8.get();
        auto up = o->processSamplesUp (block);
        processEngine (up);
        o->processSamplesDown (block);
    }

    if (osChoice != lastOsChoice)
    {
        int lat = 0;
        if      (osChoice == 1) lat = (int) std::lround (os2->getLatencyInSamples());
        else if (osChoice == 2) lat = (int) std::lround (os4->getLatencyInSamples());
        else if (osChoice == 3) lat = (int) std::lround (os8->getLatencyInSamples());
        setLatencySamples (lat);
        lastOsChoice = osChoice;
    }

    // --- make-up + dry/wet mix ---------------------------------------------
    for (int i = 0; i < n; ++i)
    {
        const float mk = outputGain.getNextValue();
        const float mx = mixAmount.getNextValue();
        for (int c = 0; c < numCh; ++c)
        {
            const float wet = buffer.getReadPointer (c)[i] * mk;
            const float dry = dryBuffer.getReadPointer (c)[i];
            buffer.getWritePointer (c)[i] = mx * wet + (1.0f - mx) * dry;
        }
    }
}

//==============================================================================
juce::AudioProcessorEditor* TEAL1178AudioProcessor::createEditor()
{
    return new TEAL1178AudioProcessorEditor (*this);
}

void TEAL1178AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
    }
}

void TEAL1178AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TEAL1178AudioProcessor();
}
