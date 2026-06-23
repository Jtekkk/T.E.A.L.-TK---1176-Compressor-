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
#include "Transformer.h"

namespace teal
{

// Ratio button positions. "All" == the 1176 "all-buttons-in" / British mode.
enum class Ratio { R4 = 0, R8, R12, R20, All };

struct CompressorEngine
{
    static constexpr int kMaxCh = 2;

    FETGainComputer gr      [kMaxCh];   // one per channel (used when unlinked)
    Transformer     inIron  [kMaxCh];   // input transformer (Jiles-Atherton)
    BiasedTanh      fet     [kMaxCh];   // FET even-harmonic core
    Transformer     outIron [kMaxCh];   // output transformer (the main "iron")
    DCBlocker       dcb     [kMaxCh];
    SvfHP           scHpf   [kMaxCh];
    double          prevDet [kMaxCh] { 0.0, 0.0 };
    int             numCh   { 2 };
    bool            linked  { true };
    double          scHpfHz { 20.0 };
    double          lastFs  { 0.0 };    // guards rate-dependent recompute
    double          baseAtt { 0.00025 };
    double          baseRel { 0.4 };
    bool            allMode { false };

    // -------------------------------------------------------------------------
    void prepare (double fs, int channels) noexcept
    {
        numCh = std::max (1, std::min (channels, kMaxCh));
        for (int c = 0; c < kMaxCh; ++c)
        {
            gr[c].prepare (fs);
            inIron [c].drive = 0.5;  inIron [c].setSampleRate (fs); inIron [c].reset();
            fet    [c].set (0.85, 0.40);                            fet    [c].reset();
            outIron[c].drive = 1.3;  outIron[c].setSampleRate (fs); outIron[c].reset();
            dcb    [c].prepare (fs);
            scHpf  [c].setSampleRate (fs); scHpf[c].setCutoff (scHpfHz); scHpf[c].reset();
            prevDet[c] = 0.0;
        }
        lastFs = fs;
        setRatioMode (Ratio::R4);
    }

    void setSampleRate (double fs) noexcept
    {
        if (fs == lastFs) return;       // rate unchanged -> skip (avoids re-calibration)
        lastFs = fs;
        for (int c = 0; c < kMaxCh; ++c)
        {
            gr[c].setSampleRate (fs);
            dcb[c].setSampleRate (fs);
            scHpf[c].setSampleRate (fs);
            inIron[c].setSampleRate (fs);
            outIron[c].setSampleRate (fs);
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
        baseAtt = attSec;
        baseRel = relSec;
        applyTimes();
    }

    // All-buttons mode shifts the circuit's bias points, which changes the
    // effective ballistics: a slower attack "lag" and a faster, pumping release
    // (README §6.2). Applied on top of the user's Attack/Release.
    void applyTimes() noexcept
    {
        const double am = allMode ? 1.35 : 1.0;
        const double rm = allMode ? 0.55 : 1.0;
        for (int c = 0; c < kMaxCh; ++c) gr[c].setTimes (baseAtt * am, baseRel * rm);
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
        // FET drive/bias: the transformer iron is symmetric (J-A), so the FET
        // is the source of the 1176's even-harmonic "presence" -- bias it fairly
        // hard. Higher ratios / all-buttons push it for more grit.
        double k = 3.0, thr = -18.0, drive = 1.05, bias = 0.62;
        switch (r)
        {
            case Ratio::R4:  k = 3.0;  thr = -18.0; drive = 1.05; bias = 0.62; break;
            case Ratio::R8:  k = 7.0;  thr = -16.0; drive = 1.15; bias = 0.64; break;
            case Ratio::R12: k = 12.0; thr = -14.0; drive = 1.30; bias = 0.66; break;
            case Ratio::R20: k = 27.0; thr = -12.0; drive = 1.45; bias = 0.68; break;
            case Ratio::All: k = 17.0; thr = -15.0; drive = 2.10; bias = 0.60; break;
        }
        for (int c = 0; c < kMaxCh; ++c)
        {
            gr[c].setRatio (k, thr);
            fet[c].set (drive, bias);
        }
        allMode = (r == Ratio::All);
        applyTimes();
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
