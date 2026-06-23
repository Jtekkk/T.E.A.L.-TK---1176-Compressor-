#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

#include "ParamIDs.h"
#include "dsp/CompressorEngine.h"

// =============================================================================
//  TEAL 1176 -- FET feedback compressor / limiter (1176 model)
//
//  Architecture (see source/dsp and the README "one-page build recipe"):
//    base rate : capture dry, apply smoothed input drive
//    OS rate   : nonlinear engine (input iron -> FET divider -> FET shaper ->
//                output iron -> DC block), feedback detection with retained
//                ripple, zero added audio latency (feedback => no lookahead)
//    base rate : apply smoothed make-up + dry/wet mix
//
//  Oversampling uses min-phase polyphase-IIR halfband filters for ~zero latency.
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

    int getNumPrograms() override                          { return 1; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const juce::String getProgramName (int) override       { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // Current gain reduction (<= 0 dB) for the editor's meter.
    std::atomic<float> gainReductionDb { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Run the engine over a block at whatever rate the block is in.
    void processEngine (juce::dsp::AudioBlock<float>& block);

    teal::CompressorEngine engine;
    juce::AudioBuffer<float> dryBuffer;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> inputGain, outputGain, mixAmount;

    std::unique_ptr<juce::dsp::Oversampling<float>> os2, os4, os8;
    double baseSampleRate { 48000.0 };
    int    lastOsChoice   { -1 };
    int    lastRatioMode  { -1 };

    // Cached atomic parameter pointers (read once per block).
    std::atomic<float>* pInput   { nullptr };
    std::atomic<float>* pOutput  { nullptr };
    std::atomic<float>* pAttack  { nullptr };
    std::atomic<float>* pRelease { nullptr };
    std::atomic<float>* pRatio   { nullptr };
    std::atomic<float>* pMix     { nullptr };
    std::atomic<float>* pOs      { nullptr };
    std::atomic<float>* pBypass  { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TEAL1176AudioProcessor)
};
