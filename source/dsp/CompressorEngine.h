#pragma once

// =============================================================================
//  CompressorEngine.h  --  full per-sample 1176 signal chain (pure C++, no JUCE)
//
//  Audio path (run at the oversampled rate by the host processor):
//
//     x -> [input iron] -> [ * g ] -> [FET nonlinearity] -> [output iron] -> [DC] -> y
//                             ^
//                             |  g from the feedback loop (or feedforward, ext SC)
//
//     detector tap:  sidechain-HPF( x * g )   (the LINEAR gain-reduced signal)
//
//  Notes:
//  * Input drive / output make-up / mix are applied OUTSIDE the engine (base
//    rate) by the processor; make-up sits after the chain like the Output pot.
//  * Detection feeds back from the LINEAR gain-reduced signal so GR scales with
//    drive (see git history / README behavioural split). With an external
//    sidechain it instead reads that signal feedforward.
//  * Stereo detection can be linked (max across channels, shared gain) or
//    independent per channel.
//  * A sidechain high-pass keeps low frequencies from driving the compressor.
//  * The biased-tanh stages add the even-harmonic "iron + FET" colour.
// =============================================================================

#include <algorithm>
#include <cmath>

#include "ADAAShaper.h"
#include "FETGainComputer.h"
#include "Filters.h"

namespace teal
{

// Ratio button positions. "All" == the 1176 "all-buttons-in" / British mode.
enum class Ratio { R4 = 0, R8, R12, R20, All };

struct CompressorEngine
{
    static constexpr int kMaxCh = 2;

    FETGainComputer gr      [kMaxCh];   // one per channel (used when unlinked)
    BiasedTanh      inIron  [kMaxCh];
    BiasedTanh      fet     [kMaxCh];
    BiasedTanh      outIron [kMaxCh];
    DCBlocker       dcb     [kMaxCh];
    SvfHP           scHpf   [kMaxCh];
    double          prevDet [kMaxCh] { 0.0, 0.0 };
    int             numCh   { 2 };
    bool            linked  { true };
    double          scHpfHz { 20.0 };

    // -------------------------------------------------------------------------
    void prepare (double fs, int channels) noexcept
    {
        numCh = std::max (1, std::min (channels, kMaxCh));
        for (int c = 0; c < kMaxCh; ++c)
        {
            gr[c].prepare (fs);
            inIron [c].set (0.60, 0.22);   inIron [c].reset();
            fet    [c].set (0.85, 0.40);   fet    [c].reset();
            outIron[c].set (0.70, 0.26);   outIron[c].reset();
            dcb    [c].prepare (fs);
            scHpf  [c].setSampleRate (fs); scHpf[c].setCutoff (scHpfHz); scHpf[c].reset();
            prevDet[c] = 0.0;
        }
        setRatioMode (Ratio::R4);
    }

    void setSampleRate (double fs) noexcept
    {
        for (int c = 0; c < kMaxCh; ++c)
        {
            gr[c].setSampleRate (fs);
            dcb[c].setSampleRate (fs);
            scHpf[c].setSampleRate (fs);
        }
    }

    void reset() noexcept
    {
        for (int c = 0; c < kMaxCh; ++c)
        {
            gr[c].reset();
            inIron[c].reset();
            fet[c].reset();
            outIron[c].reset();
            dcb[c].reset();
            scHpf[c].reset();
            prevDet[c] = 0.0;
        }
    }

    void setTimes (double attSec, double relSec) noexcept
    {
        for (int c = 0; c < kMaxCh; ++c) gr[c].setTimes (attSec, relSec);
    }

    void setLinked (bool shouldLink) noexcept { linked = shouldLink; }

    void setSidechainHpf (double hz) noexcept
    {
        scHpfHz = hz;
        for (int c = 0; c < kMaxCh; ++c) scHpf[c].setCutoff (hz);
    }

    // Feedback (internal) vs feedforward (external sidechain) detection law.
    void setExternalSidechain (bool external) noexcept
    {
        for (int c = 0; c < kMaxCh; ++c) gr[c].feedforward = external;
    }

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
        for (int c = 0; c < kMaxCh; ++c)
        {
            gr[c].setRatio (k, thr);
            fet[c].set (drive, bias);
        }
    }

    // Most negative (largest) reduction across channels, for metering.
    double getGainReductionDb() const noexcept
    {
        return std::min (gr[0].lastGrDb, gr[1].lastGrDb);
    }

    // Process one frame in place. `x` holds nCh samples. If `sc` is non-null it
    // is the external sidechain frame (scCh samples) used for feedforward
    // detection; otherwise detection is the internal feedback path.
    inline void processFrame (double* x, int nCh,
                              const double* sc = nullptr, int scCh = 0) noexcept
    {
        const int c = std::min (nCh, numCh);

        double dsig[2] = { 0.0, 0.0 };
        if (sc != nullptr && scCh > 0)
            for (int j = 0; j < c; ++j)
                dsig[j] = scHpf[j].process (sc[j < scCh ? j : scCh - 1]);
        else
            for (int j = 0; j < c; ++j)
                dsig[j] = scHpf[j].process (prevDet[j]);

        double gain[2] = { 1.0, 1.0 };
        if (linked)
        {
            double det = 0.0;
            for (int j = 0; j < c; ++j) det = std::max (det, std::abs (dsig[j]));
            const double g = gr[0].process (det);
            gr[1].lastGrDb = gr[0].lastGrDb;     // keep meter consistent
            gain[0] = gain[1] = g;
        }
        else
        {
            gain[0] = gr[0].process (std::abs (dsig[0]));
            if (c > 1) gain[1] = gr[1].process (std::abs (dsig[1]));
            else       gr[1].lastGrDb = gr[0].lastGrDb;
        }

        for (int j = 0; j < c; ++j)
        {
            const double in = x[j];
            prevDet[j]      = in * gain[j];                   // feedback tap (linear GR'd)

            const double a      = inIron[j].process (in);
            double       postGR = a * gain[j];
            postGR              = fet[j].process (postGR);

            double y = outIron[j].process (postGR);
            y        = dcb[j].process (y);
            x[j]     = y;
        }
    }
};

} // namespace teal
