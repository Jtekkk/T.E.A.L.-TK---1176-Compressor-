#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

using APVTS = juce::AudioProcessorValueTreeState;

//==============================================================================
namespace
{
struct Preset
{
    const char* name;
    float input, output, attackUs, releaseMs;
    int   ratio;        // 0..4 -> 4:1/8:1/12:1/20:1/All
    float mix, scHpf;
    bool  link;
};

const Preset kPresets[] = {
    { "Init / Clean",        0.0f, 0.0f, 250.0f, 400.0f, 0, 100.0f,  20.0f, true },
    { "Dr. Pepper (Vocal)", 10.0f, 4.0f, 300.0f, 350.0f, 0, 100.0f,  20.0f, true },
    { "Vocal Bite",         14.0f, 6.0f,  80.0f, 200.0f, 0, 100.0f,  70.0f, true },
    { "Bass Control",       10.0f, 4.0f, 200.0f, 250.0f, 0, 100.0f,  60.0f, true },
    { "Drum Smash (All)",   18.0f, 8.0f,  50.0f, 150.0f, 4, 100.0f,  20.0f, true },
    { "Parallel Crush",     24.0f, 0.0f,  30.0f, 100.0f, 4,  40.0f,  20.0f, true },
    { "Brick Limit",        16.0f, 6.0f,  20.0f, 100.0f, 3, 100.0f,  20.0f, true },
    { "Smooth Leveler",      6.0f, 3.0f, 500.0f, 600.0f, 0, 100.0f,  20.0f, true },
};
constexpr int kNumPresets = (int) (sizeof (kPresets) / sizeof (kPresets[0]));

void setParamValue (APVTS& s, const char* id, float realValue)
{
    if (auto* p = s.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (realValue));
}
} // namespace

//==============================================================================
APVTS::ParameterLayout TEAL1176AudioProcessor::createParameterLayout()
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
        NormalisableRange<float> r (20.0f, 800.0f, 0.1f);
        r.setSkewForCentre (120.0f);
        layout.add (std::make_unique<AudioParameterFloat> (
            ParameterID { pid::attack, 1 }, "Attack", r, 250.0f,
            AudioParameterFloatAttributes().withLabel ("\xc2\xb5s")));
    }
    {
        NormalisableRange<float> r (50.0f, 1100.0f, 1.0f);
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

    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { pid::osQuality, 1 }, "OS Quality",
        StringArray { "Low Latency", "Linear Phase" }, 0));

    {
        NormalisableRange<float> r (20.0f, 500.0f, 1.0f);
        r.setSkewForCentre (100.0f);
        layout.add (std::make_unique<AudioParameterFloat> (
            ParameterID { pid::scHpf, 1 }, "SC HPF", r, 20.0f,
            AudioParameterFloatAttributes().withLabel ("Hz")));
    }

    layout.add (std::make_unique<AudioParameterBool> (
        ParameterID { pid::link, 1 }, "Stereo Link", true));
    layout.add (std::make_unique<AudioParameterBool> (
        ParameterID { pid::extSc, 1 }, "Ext SC", false));
    layout.add (std::make_unique<AudioParameterBool> (
        ParameterID { pid::bypass, 1 }, "Bypass", false));

    return layout;
}

//==============================================================================
TEAL1176AudioProcessor::TEAL1176AudioProcessor()
    : AudioProcessor (BusesProperties()
          .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output",    juce::AudioChannelSet::stereo(), true)
          .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), false)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    pInput   = apvts.getRawParameterValue (pid::input);
    pOutput  = apvts.getRawParameterValue (pid::output);
    pAttack  = apvts.getRawParameterValue (pid::attack);
    pRelease = apvts.getRawParameterValue (pid::release);
    pRatio   = apvts.getRawParameterValue (pid::ratio);
    pMix     = apvts.getRawParameterValue (pid::mix);
    pOs      = apvts.getRawParameterValue (pid::os);
    pOsQ     = apvts.getRawParameterValue (pid::osQuality);
    pScHpf   = apvts.getRawParameterValue (pid::scHpf);
    pLink    = apvts.getRawParameterValue (pid::link);
    pExtSc   = apvts.getRawParameterValue (pid::extSc);
    pBypass  = apvts.getRawParameterValue (pid::bypass);
}

//==============================================================================
bool TEAL1176AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto main = layouts.getMainOutputChannelSet();
    if (main != juce::AudioChannelSet::mono() && main != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainInputChannelSet() != main)
        return false;

    if (layouts.inputBuses.size() > 1)
    {
        const auto sc = layouts.inputBuses.getReference (1);
        if (! sc.isDisabled()
            && sc != juce::AudioChannelSet::mono()
            && sc != juce::AudioChannelSet::stereo())
            return false;
    }
    return true;
}

//==============================================================================
void TEAL1176AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    baseSampleRate = sampleRate;

    const int ch = juce::jlimit (1, 2, getMainBusNumOutputChannels());

    engine.prepare (sampleRate, ch);
    engine.setLinked (pLink->load() > 0.5f);
    engine.setSidechainHpf (pScHpf->load());

    dryBuffer.setSize (ch, samplesPerBlock, false, false, true);

    constexpr double ramp = 0.02;
    inputGain .reset (sampleRate, ramp);
    outputGain.reset (sampleRate, ramp);
    mixAmount .reset (sampleRate, ramp);
    bypassRamp.reset (sampleRate, 0.01);
    inputGain .setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pInput->load()));
    outputGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pOutput->load()));
    mixAmount .setCurrentAndTargetValue (pMix->load() * 0.01f);
    bypassRamp.setCurrentAndTargetValue (pBypass->load() > 0.5f ? 0.0f : 1.0f);

    auto makeOs = [ch, samplesPerBlock] (int log2factor, OS::FilterType ft)
    {
        auto o = std::make_unique<OS> ((size_t) ch, (size_t) log2factor, ft);
        o->initProcessing ((size_t) samplesPerBlock);
        o->reset();
        return o;
    };
    os2  = makeOs (1, OS::filterHalfBandPolyphaseIIR);
    os4  = makeOs (2, OS::filterHalfBandPolyphaseIIR);
    os8  = makeOs (3, OS::filterHalfBandPolyphaseIIR);
    os2f = makeOs (1, OS::filterHalfBandFIREquiripple);
    os4f = makeOs (2, OS::filterHalfBandFIREquiripple);
    os8f = makeOs (3, OS::filterHalfBandFIREquiripple);

    lastOsKey     = -1;
    lastRatioMode = -1;
    setLatencySamples (0);
}

//==============================================================================
void TEAL1176AudioProcessor::processEngine (juce::dsp::AudioBlock<float>& block,
                                            const float* sc0, const float* sc1,
                                            int scNumCh, int factor)
{
    const int n  = (int) block.getNumSamples();
    const int ch = juce::jmin ((int) block.getNumChannels(), 2);

    float* chan[2] = { nullptr, nullptr };
    for (int c = 0; c < ch; ++c)
        chan[c] = block.getChannelPointer ((size_t) c);

    const bool useSc = scNumCh > 0;
    const int  scCh  = juce::jmin (scNumCh, 2);

    double frame[2] = { 0.0, 0.0 };
    double scf[2]   = { 0.0, 0.0 };
    float  minGr = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        for (int c = 0; c < ch; ++c) frame[c] = (double) chan[c][i];

        const double* scPtr = nullptr;
        if (useSc)
        {
            const int bi = (factor > 1) ? i / factor : i;
            scf[0] = sc0 != nullptr ? (double) sc0[bi] : 0.0;
            scf[1] = sc1 != nullptr ? (double) sc1[bi] : scf[0];
            scPtr  = scf;
        }

        engine.processFrame (frame, ch, scPtr, scCh);

        for (int c = 0; c < ch; ++c) chan[c][i] = (float) frame[c];

        const float gr = (float) engine.getGainReductionDb();
        if (gr < minGr) minGr = gr;
    }

    lastBlockGr = minGr;
}

//==============================================================================
void TEAL1176AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int n     = buffer.getNumSamples();
    const int numCh = juce::jmin (getMainBusNumOutputChannels(), 2);
    if (n == 0 || numCh == 0)
        return;

    if (dryBuffer.getNumSamples() < n || dryBuffer.getNumChannels() < numCh)
        dryBuffer.setSize (numCh, n, false, false, true);

    // --- parameters --------------------------------------------------------
    const int   osChoice  = (int) pOs->load();
    const int   osQ       = (int) pOsQ->load();
    const int   ratioMode = (int) pRatio->load();
    const float attUs     = pAttack->load();
    const float relMs     = pRelease->load();
    const bool  linkOn    = pLink->load()   > 0.5f;
    const float scHz      = pScHpf->load();
    const bool  extOn     = pExtSc->load()  > 0.5f;
    const bool  bypassed  = pBypass->load() > 0.5f;

    inputGain .setTargetValue (juce::Decibels::decibelsToGain (pInput->load()));
    outputGain.setTargetValue (juce::Decibels::decibelsToGain (pOutput->load()));
    mixAmount .setTargetValue (pMix->load() * 0.01f);
    bypassRamp.setTargetValue (bypassed ? 0.0f : 1.0f);

    const int factor = (osChoice == 0) ? 1 : (osChoice == 1) ? 2 : (osChoice == 2) ? 4 : 8;
    engine.setSampleRate (baseSampleRate * factor);
    engine.setTimes (attUs * 1.0e-6, relMs * 1.0e-3);
    engine.setLinked (linkOn);
    engine.setSidechainHpf (scHz);
    if (ratioMode != lastRatioMode)
    {
        engine.setRatioMode (static_cast<teal::Ratio> (ratioMode));
        lastRatioMode = ratioMode;
    }

    // --- external sidechain ------------------------------------------------
    auto* scBus = getBus (true, 1);
    const bool scConnected = scBus != nullptr && scBus->isEnabled();
    const bool useExt = extOn && scConnected;
    engine.setExternalSidechain (useExt);

    const float* sc0 = nullptr;
    const float* sc1 = nullptr;
    int scNumCh = 0;
    if (useExt)
    {
        auto scBuffer = getBusBuffer (buffer, true, 1);
        scNumCh = juce::jmin (scBuffer.getNumChannels(), 2);
        if (scNumCh > 0) sc0 = scBuffer.getReadPointer (0);
        if (scNumCh > 1) sc1 = scBuffer.getReadPointer (1);
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
    auto block = juce::dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, (size_t) numCh);

    OS* osPtr = nullptr;
    if (osChoice != 0)
        osPtr = (osQ == 0) ? (osChoice == 1 ? os2.get()  : osChoice == 2 ? os4.get()  : os8.get())
                           : (osChoice == 1 ? os2f.get() : osChoice == 2 ? os4f.get() : os8f.get());

    if (osPtr == nullptr)
    {
        processEngine (block, sc0, sc1, scNumCh, 1);
    }
    else
    {
        auto up = osPtr->processSamplesUp (block);
        processEngine (up, sc0, sc1, scNumCh, factor);
        osPtr->processSamplesDown (block);
    }

    const int osKey = osChoice * 2 + osQ;
    if (osKey != lastOsKey)
    {
        setLatencySamples (osPtr != nullptr ? (int) std::lround (osPtr->getLatencyInSamples()) : 0);
        lastOsKey = osKey;
    }

    gainReductionDb.store (bypassed ? 0.0f : lastBlockGr);

    // --- make-up + dry/wet mix + soft bypass -------------------------------
    for (int i = 0; i < n; ++i)
    {
        const float mk = outputGain.getNextValue();
        const float mx = mixAmount.getNextValue();
        const float bp = bypassRamp.getNextValue();
        for (int c = 0; c < numCh; ++c)
        {
            const float dry  = dryBuffer.getReadPointer (c)[i];
            const float wet  = buffer.getReadPointer (c)[i] * mk;
            const float comp = mx * wet + (1.0f - mx) * dry;
            buffer.getWritePointer (c)[i] = bp * comp + (1.0f - bp) * dry;
        }
    }
}

//==============================================================================
int TEAL1176AudioProcessor::getNumPrograms()              { return kNumPresets; }
int TEAL1176AudioProcessor::getCurrentProgram()           { return currentProgram; }
void TEAL1176AudioProcessor::setCurrentProgram (int i)    { applyProgram (i); }

const juce::String TEAL1176AudioProcessor::getProgramName (int i)
{
    return (i >= 0 && i < kNumPresets) ? juce::String (kPresets[i].name) : juce::String();
}

void TEAL1176AudioProcessor::applyProgram (int index)
{
    index = juce::jlimit (0, kNumPresets - 1, index);
    const auto& pr = kPresets[index];
    setParamValue (apvts, pid::input,   pr.input);
    setParamValue (apvts, pid::output,  pr.output);
    setParamValue (apvts, pid::attack,  pr.attackUs);
    setParamValue (apvts, pid::release, pr.releaseMs);
    setParamValue (apvts, pid::ratio,   (float) pr.ratio);
    setParamValue (apvts, pid::mix,     pr.mix);
    setParamValue (apvts, pid::scHpf,   pr.scHpf);
    setParamValue (apvts, pid::link,    pr.link ? 1.0f : 0.0f);
    currentProgram = index;
}

//==============================================================================
juce::AudioProcessorEditor* TEAL1176AudioProcessor::createEditor()
{
    return new TEAL1176AudioProcessorEditor (*this);
}

void TEAL1176AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
    }
}

void TEAL1176AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TEAL1176AudioProcessor();
}
