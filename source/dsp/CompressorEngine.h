#pragma once

// =============================================================================
//  CompressorEngine.h  --  full per-sample 1176 signal chain (pure C++, no JUCE)
//
//  Audio path (run at the oversampled rate by the host processor):
//
//     x -> [input iron] -> [ * g ] -> [FET nonlinearity] -> [output iron] -> [DC] -> y
//                             ^
//                             |  g from the feedback loop
//                             |
//     detector tap:  x * g  (the LINEAR gain-reduced signal, previous sample)
//
//  * Input drive and output make-up are applied OUTSIDE the engine, at base
//    rate, by the plugin processor (they are linear and commute with the
//    oversampling, which keeps the costly nonlinear core lean and the gain
//    stages zipper-free). Make-up sits after the whole chain, like the 1176
//    Output pot, so it does not affect detection.
//  * Detection feeds back from the LINEAR gain-reduced signal (x*g) rather than
//    the saturated audio, so gain reduction scales properly with input drive
//    (sensing the post-saturation audio would let the iron's ceiling clamp the
//    control loop and cap GR). This is the behavioural split the README
//    recommends: model "how much GR" and "how much grit" separately. The
//    program-dependent ratio creep and LF grit still emerge from the retained
//    detector ripple and the signal-dependent multiply; the drive-dependent
//    grit comes from the input iron saturating harder as you push Input.
//  * Detection is stereo-LINKED (max across channels), so the stereo image
//    does not wander -- the hardware does this with the 1176SA link.
//  * The biased-tanh stages add the even-harmonic "iron + FET" colour.
// =============================================================================

#include <algorithm>
#include <cmath>

#include "ADAAShaper.h"
#include "FETGainComputer.h"

namespace teal
{

// Ratio button positions. "All" == the 1176 "all-buttons-in" / British mode.
enum class Ratio { R4 = 0, R8, R12, R20, All };

struct CompressorEngine
{
    static constexpr int kMaxCh = 2;

    FETGainComputer gr;
    BiasedTanh      inIron  [kMaxCh];
    BiasedTanh      fet     [kMaxCh];
    BiasedTanh      outIron [kMaxCh];
    DCBlocker       dcb     [kMaxCh];
    double          prevDet [kMaxCh] { 0.0, 0.0 };   // previous linear gain-reduced sample
    int             numCh { 2 };

    // -------------------------------------------------------------------------
    void prepare (double fs, int channels) noexcept
    {
        numCh = std::max (1, std::min (channels, kMaxCh));
        gr.prepare (fs);
        for (int c = 0; c < kMaxCh; ++c)
        {
            // Low drive + high asymmetry => even-harmonic dominant ("warm"),
            // because for a biased tanh  H2/H3 ~ tanh(bias)/(drive*level).
            inIron [c].set (0.60, 0.22);   inIron [c].reset();   // input transformer
            fet    [c].set (0.85, 0.40);   fet    [c].reset();   // FET even-harmonic core (set per ratio)
            outIron[c].set (0.70, 0.26);   outIron[c].reset();   // output iron + Class A
            dcb    [c].prepare (fs);
            prevDet[c] = 0.0;
        }
        setRatioMode (Ratio::R4);
    }

    // Recompute rate-dependent coefficients (oversampling change) without
    // clearing running state.
    void setSampleRate (double fs) noexcept
    {
        gr.setSampleRate (fs);
        for (int c = 0; c < kMaxCh; ++c)
            dcb[c].setSampleRate (fs);
    }

    void reset() noexcept
    {
        gr.reset();
        for (int c = 0; c < kMaxCh; ++c)
        {
            inIron[c].reset();
            fet[c].reset();
            outIron[c].reset();
            dcb[c].reset();
            prevDet[c] = 0.0;
        }
    }

    void setTimes (double attSec, double relSec) noexcept { gr.setTimes (attSec, relSec); }

    // ratio -> { loop gain k = R-1, threshold, FET drive/bias }.
    // Higher ratios raise the threshold; "All" drops in a parallel bias network
    // landing ~12-20:1 with shifted bias points => extra grind (README §6.2).
    void setRatioMode (Ratio r) noexcept
    {
        double k = 3.0, thr = -18.0, drive = 0.85, bias = 0.40;
        switch (r)
        {
            case Ratio::R4:  k = 3.0;  thr = -18.0; drive = 0.85; bias = 0.40; break;
            case Ratio::R8:  k = 7.0;  thr = -16.0; drive = 0.95; bias = 0.42; break;
            case Ratio::R12: k = 12.0; thr = -14.0; drive = 1.05; bias = 0.44; break;
            case Ratio::R20: k = 27.0; thr = -12.0; drive = 1.20; bias = 0.46; break;
            // All-buttons: more drive => grittier, more odd content + grind.
            case Ratio::All: k = 17.0; thr = -15.0; drive = 1.80; bias = 0.40; break;
        }
        gr.setRatio (k, thr);
        for (int c = 0; c < kMaxCh; ++c)
            fet[c].set (drive, bias);
    }

    double getGainReductionDb() const noexcept { return gr.lastGrDb; }

    // Process one frame in place. x points to `nCh` samples (interleaved by
    // pointer arg, i.e. x[0], x[1]). Input drive / make-up / mix are handled
    // by the caller. Returns nothing; x is overwritten with the wet signal.
    inline void processFrame (double* x, int nCh) noexcept
    {
        const int c = std::min (nCh, numCh);

        // Stereo-linked feedback detector: max |previous gain-reduced sample|.
        double det = 0.0;
        for (int j = 0; j < c; ++j)
        {
            const double m = std::abs (prevDet[j]);
            if (m > det) det = m;
        }

        const double g = gr.process (det);

        for (int j = 0; j < c; ++j)
        {
            const double in = x[j];                           // drive already applied by caller
            prevDet[j]      = in * g;                          // feedback tap: linear GR'd signal

            const double a      = inIron[j].process (in);     // input transformer (drive colour)
            double       postGR = a * g;                      // FET divider attenuation
            postGR              = fet[j].process (postGR);    // FET nonlinearity (even harmonics)

            double y = outIron[j].process (postGR);           // output iron + Class A
            y        = dcb[j].process (y);                    // remove residual DC
            x[j]     = y;
        }
    }
};

} // namespace teal
