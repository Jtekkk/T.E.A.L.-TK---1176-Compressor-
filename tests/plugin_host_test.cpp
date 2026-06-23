// =============================================================================
//  plugin_host_test.cpp  --  headless smoke test of the real AudioProcessor.
//
//  Instantiates the plugin via createPluginFilter(), runs sine audio through
//  every oversampling factor and every ratio, and checks the output stays
//  finite and non-silent. Also exercises the APVTS state round-trip.
//  No GUI and no audio device are opened.
// =============================================================================

#include <juce_audio_processors/juce_audio_processors.h>

#include <cmath>
#include <cstdio>
#include <memory>

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

namespace
{
constexpr double kPi = 3.14159265358979323846;

void setParam (juce::AudioProcessor& p, const juce::String& id, float norm)
{
    for (auto* raw : p.getParameters())
        if (auto* wid = dynamic_cast<juce::AudioProcessorParameterWithID*> (raw))
            if (wid->paramID == id)
            {
                raw->setValueNotifyingHost (norm);
                return;
            }
}

struct Stats { bool finite = true; float peak = 0.0f; };

Stats runSeconds (juce::AudioProcessor& p, double fs, int block, double seconds, float amp)
{
    juce::AudioBuffer<float> buf (2, block);
    juce::MidiBuffer midi;
    Stats st;
    double phase = 0.0;
    const double inc = 2.0 * kPi * 1000.0 / fs;
    const int blocks = (int) (seconds * fs / block);

    for (int b = 0; b < blocks; ++b)
    {
        for (int i = 0; i < block; ++i)
        {
            const float s = amp * (float) std::sin (phase);
            phase += inc;
            buf.setSample (0, i, s);
            buf.setSample (1, i, s);
        }
        p.processBlock (buf, midi);
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < block; ++i)
            {
                const float v = buf.getSample (c, i);
                if (! std::isfinite (v)) st.finite = false;
                st.peak = juce::jmax (st.peak, std::abs (v));
            }
    }
    return st;
}
} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI init;

    std::unique_ptr<juce::AudioProcessor> p (createPluginFilter());
    if (p == nullptr) { std::printf ("FAIL: createPluginFilter returned null\n"); return 1; }

    std::printf ("plugin: %s\n", p->getName().toRawUTF8());
    std::printf ("parameters: %d\n", p->getParameters().size());

    const double fs = 48000.0;
    const int    block = 512;
    p->setPlayConfigDetails (2, 2, fs, block);
    p->prepareToPlay (fs, block);

    bool ok = true;

    // Push hard so we definitely compress, then sweep OS factors and ratios.
    setParam (*p, "input",  0.85f);   // lots of drive
    setParam (*p, "output", 0.6f);
    setParam (*p, "mix",    1.0f);

    const char* osNames[]    = { "1x", "2x", "4x", "8x" };
    const float osNorms[]    = { 0.0f, 1.0f / 3.0f, 2.0f / 3.0f, 1.0f };
    const char* ratioNames[] = { "4:1", "8:1", "12:1", "20:1", "All" };
    const float ratioNorms[] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };

    for (int o = 0; o < 4; ++o)
    {
        setParam (*p, "os", osNorms[o]);
        for (int r = 0; r < 5; ++r)
        {
            setParam (*p, "ratio", ratioNorms[r]);
            auto st = runSeconds (*p, fs, block, 0.25, 0.5f);
            const bool pass = st.finite && st.peak > 1.0e-4f && st.peak < 16.0f;
            ok = ok && pass;
            std::printf ("  OS %-3s ratio %-5s -> peak %.3f finite=%d  %s\n",
                         osNames[o], ratioNames[r], st.peak, (int) st.finite,
                         pass ? "ok" : "FAIL");
        }
    }

    // Stereo-link off + sidechain HPF: must stay finite/non-silent.
    setParam (*p, "os", 2.0f / 3.0f);   // 4x
    setParam (*p, "ratio", 0.0f);
    setParam (*p, "link", 0.0f);
    {
        auto st = runSeconds (*p, fs, block, 0.25, 0.5f);
        const bool pass = st.finite && st.peak > 1.0e-4f && st.peak < 16.0f;
        ok = ok && pass;
        std::printf ("  link OFF        -> peak %.3f finite=%d  %s\n", st.peak, (int) st.finite, pass ? "ok" : "FAIL");
    }
    setParam (*p, "link", 1.0f);
    setParam (*p, "schpf", 0.6f);       // high-passed sidechain
    {
        auto st = runSeconds (*p, fs, block, 0.25, 0.5f);
        const bool pass = st.finite && st.peak > 1.0e-4f && st.peak < 16.0f;
        ok = ok && pass;
        std::printf ("  SC-HPF on       -> peak %.3f finite=%d  %s\n", st.peak, (int) st.finite, pass ? "ok" : "FAIL");
    }
    setParam (*p, "schpf", 0.0f);

    // Linear-phase oversampling across factors.
    setParam (*p, "osq", 1.0f);
    for (int o = 1; o < 4; ++o)
    {
        setParam (*p, "os", osNorms[o]);
        auto st = runSeconds (*p, fs, block, 0.2, 0.5f);
        const bool pass = st.finite && st.peak > 1.0e-4f && st.peak < 16.0f;
        ok = ok && pass;
        std::printf ("  OS %-3s linear   -> peak %.3f finite=%d  %s\n", osNames[o], st.peak, (int) st.finite, pass ? "ok" : "FAIL");
    }
    setParam (*p, "osq", 0.0f);
    setParam (*p, "os", 0.0f);

    // Presets: each program should apply and produce finite audio.
    {
        const int nProg = p->getNumPrograms();
        bool presetsOk = true;
        for (int i = 0; i < nProg; ++i)
        {
            p->setCurrentProgram (i);
            auto st = runSeconds (*p, fs, block, 0.1, 0.5f);
            if (! (st.finite && st.peak > 1.0e-4f)) presetsOk = false;
        }
        ok = ok && presetsOk;
        std::printf ("  %d presets       -> %s\n", nProg, presetsOk ? "ok" : "FAIL");
    }

    // Soft bypass: after the crossfade settles, output == dry input (~0.5).
    setParam (*p, "input", 0.85f);
    setParam (*p, "bypass", 1.0f);
    runSeconds (*p, fs, block, 0.1, 0.5f);          // let the ramp settle
    {
        auto st = runSeconds (*p, fs, block, 0.1, 0.5f);
        const bool pass = st.finite && std::abs (st.peak - 0.5f) < 0.02f;
        ok = ok && pass;
        std::printf ("  soft bypass     -> peak %.3f (expect ~0.5)  %s\n", st.peak, pass ? "ok" : "FAIL");
    }
    setParam (*p, "bypass", 0.0f);

    // State round-trip.
    juce::MemoryBlock mb;
    p->getStateInformation (mb);
    p->setStateInformation (mb.getData(), (int) mb.getSize());
    std::printf ("  state round-trip: %d bytes  %s\n",
                 (int) mb.getSize(), mb.getSize() > 0 ? "ok" : "FAIL");
    ok = ok && mb.getSize() > 0;

    p->releaseResources();

    std::printf ("%s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
