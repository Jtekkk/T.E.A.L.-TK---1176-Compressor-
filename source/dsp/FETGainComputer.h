#pragma once

// =============================================================================
//  FETGainComputer.h  --  1176-style feedback detector / gain computer
//
//  Pure C++ (no JUCE).
//
//  Models the sidechain described in the README:
//    * Detection is FEEDBACK: the level is sensed AFTER gain reduction. The
//      caller feeds in the (rectified) post-GR signal from the previous sample,
//      which breaks the delay-free loop with ZERO audio latency (README §6).
//    * Ratio is the sidechain loop gain:  ratio R = 1 + k  (README §B.1), so
//      the achieved reduction is  G = -k/(1+k) * (level - threshold).
//    * Higher ratios raise the threshold (README §6.1).
//    * The smoothing one-pole is split into separate attack (charge) and
//      release (discharge) time constants. It is deliberately NOT
//      over-smoothed: the residual full-wave-rectifier ripple at 2*f that
//      survives at low frequency + fast settings is what produces the
//      program-dependent LF "grit" (README §C.3 / §2.2). Do not add an extra
//      control-side low-pass or you delete the sound.
//    * A soft knee approximates the structurally-soft, program-dependent knee
//      that the real feedback loop produces "for free" (README §B.2).
// =============================================================================

#include <cmath>
#include <algorithm>

namespace teal
{

struct FETGainComputer
{
    // --- configuration -------------------------------------------------------
    double fs          { 48000.0 };
    double attSec      { 0.00025 };   // 250 us
    double relSec      { 0.4 };       // 400 ms
    double thresholdDb { -18.0 };
    double k           { 3.0 };       // ratio - 1  (4:1 -> 3)
    double kneeDb      { 6.0 };

    // --- state ---------------------------------------------------------------
    double env      { 0.0 };          // linear envelope of the rectified detector
    double attCoeff { 0.0 };
    double relCoeff { 0.0 };
    double lastGrDb { 0.0 };          // most recent gain reduction (<= 0), for metering

    void prepare (double sampleRate) noexcept
    {
        fs = sampleRate;
        setTimes (attSec, relSec);
        reset();
    }

    // Recompute coefficients for a new sample rate WITHOUT clearing the
    // envelope (used when the oversampling factor changes mid-stream).
    void setSampleRate (double sampleRate) noexcept
    {
        fs = sampleRate;
        setTimes (attSec, relSec);
    }

    void reset() noexcept { env = 0.0; lastGrDb = 0.0; }

    void setTimes (double attackSeconds, double releaseSeconds) noexcept
    {
        attSec = std::max (1.0e-6, attackSeconds);
        relSec = std::max (1.0e-6, releaseSeconds);
        attCoeff = 1.0 - std::exp (-1.0 / (attSec * fs));
        relCoeff = 1.0 - std::exp (-1.0 / (relSec * fs));
    }

    void setRatio (double loopGainK, double thrDb) noexcept
    {
        k           = loopGainK;
        thresholdDb = thrDb;
    }

    // Soft-knee "amount over threshold" in dB (>= 0).
    static inline double softOver (double overDb, double W) noexcept
    {
        if (W <= 0.0) return overDb > 0.0 ? overDb : 0.0;
        if (overDb <= -0.5 * W) return 0.0;
        if (overDb >=  0.5 * W) return overDb;
        const double t = overDb + 0.5 * W;
        return (t * t) / (2.0 * W);
    }

    // detectorRect = full-wave-rectified, (stereo-linked) post-GR signal from
    // the previous sample. Returns the linear divider gain in (0, 1].
    inline double process (double detectorRect) noexcept
    {
        const double c = (detectorRect > env) ? attCoeff : relCoeff;
        env += c * (detectorRect - env);                       // ripple intentionally retained

        const double envDb = 20.0 * std::log10 (std::max (1.0e-9, env));
        const double over  = softOver (envDb - thresholdDb, kneeDb);

        // Control law is G = -k * over. Because the detector senses the
        // gain-reduced (post-GR) signal, the closed loop turns this into a
        // steady-state slope of 1/(1+k), i.e. ratio R = 1+k (README §B.1).
        // (Do NOT pre-divide by (1+k) here -- the feedback already does it.)
        const double grDb = -k * over;

        lastGrDb = grDb;
        return std::pow (10.0, grDb * 0.05);
    }
};

} // namespace teal
