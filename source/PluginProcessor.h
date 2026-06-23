#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

#include "ParamIDs.h"
#include "dsp/CompressorEngine.h"

// =============================================================================
//  TEAL 1176 -- FET feedback compressor / limiter (1176 model)
//
//    base rate : capture dry, apply smoothed input drive
//    OS rate   : nonlinear engine (input iron -> FET divider -> FET shaper ->
//                output iron), feedback (or external feedforward) detection
//    base rate : make-up + dry/wet mix + click-free soft bypass
//
//  Oversampling: 1x-8x, selectable low-latency IIR or linear-phase FIR.
//  Optional stereo-link, sidechain high-pass, and external sidechain input.
// =============================================================================

class TEAL1176AudioProcessor : public juce::AudioProcessor
{
public:
    TEAL1176AudioProcessor();
    ~TEAL1176AudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                        { return true; }

    const juce::String getName() const override            { return JucePlugin_Name; }
    bool acceptsMidi() const override                      { return false; }
    bool producesMidi() const override                     { return false; }
    bool isMidiEffect() const override                     { return false; }
    double getTailLengthSeconds() const override           { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // Metering for the editor (read on the UI timer).
    std::atomic<float> gainReductionDb { 0.0f };
    std::atomic<float> inputLevelDb  { -100.0f };
    std::atomic<float> outputLevelDb { -100.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void processEngine (juce::dsp::AudioBlock<float>& block,
                        const float* sc0, const float* sc1, int scNumCh, int factor);
    void applyProgram (int index);

    using OS = juce::dsp::Oversampling<float>;

    teal::CompressorEngine engine;
    juce::AudioBuffer<float> dryBuffer;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> inputGain, outputGain, mixAmount, bypassRamp;

    std::unique_ptr<OS> os2,  os4,  os8;     // low-latency IIR
    std::unique_ptr<OS> os2f, os4f, os8f;    // linear-phase FIR
    double baseSampleRate { 48000.0 };
    int    lastOsKey      { -1 };            // encodes (osChoice, quality) for latency updates
    int    lastRatioMode  { -1 };
    int    currentProgram { 0 };
    float  lastBlockGr     { 0.0f };

    std::atomic<float>* pInput    { nullptr };
    std::atomic<float>* pOutput   { nullptr };
    std::atomic<float>* pAttack   { nullptr };
    std::atomic<float>* pRelease  { nullptr };
    std::atomic<float>* pRatio    { nullptr };
    std::atomic<float>* pMix      { nullptr };
    std::atomic<float>* pOs       { nullptr };
    std::atomic<float>* pOsQ      { nullptr };
    std::atomic<float>* pScHpf    { nullptr };
    std::atomic<float>* pLink     { nullptr };
    std::atomic<float>* pExtSc    { nullptr };
    std::atomic<float>* pBypass   { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TEAL1176AudioProcessor)
};
