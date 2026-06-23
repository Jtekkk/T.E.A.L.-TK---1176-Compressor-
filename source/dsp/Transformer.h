#pragma once

// =============================================================================
//  Transformer.h  --  Jiles-Atherton transformer/core saturation (pure C++).
//
//  Structure (the physically-motivated audio approach):
//
//      in --> leaky integrator (flux = INT V dt) --> Jiles-Atherton M(H)
//          --> leaky differentiator (back to voltage) --> out
//
//  Integrating first makes the saturation FLUX-dependent, so low frequencies
//  (large flux for the same level) saturate more than highs -- the core "iron"
//  behaviour the README describes (LF 3rd-harmonic that grows at low frequency
//  and high level). The Jiles-Atherton magnetisation supplies the hysteresis
//  loop; small signals pass at ~unity (transparent until you push it).
//
//  Reference: Jiles & Atherton, "Theory of ferromagnetic hysteresis" (1986);
//  audio adaptation per the README Part 2/3 transformer notes.
// =============================================================================

#include <cmath>
#include <algorithm>

namespace teal
{

struct Transformer
{
    // --- Jiles-Atherton parameters (normalised, tunable) --------------------
    double Ms    { 1.0 };       // saturation magnetisation
    double a     { 0.16 };      // anhysteretic shape
    double alpha { 1.6e-3 };    // inter-domain coupling
    double kPin  { 0.45 };      // pinning / coercivity (loop width)
    double c     { 0.55 };      // reversibility (0..1)

    double drive { 1.0 };       // how hard the core is pushed (per stage)

    // --- running state ------------------------------------------------------
    double fs    { 48000.0 };
    double M     { 0.0 };
    double Hprev { 0.0 };
    double lastDelta { 1.0 };
    double intState  { 0.0 };   // leaky integrator
    double diffPrev  { 0.0 };   // differentiator memory
    double rInt  { 0.0 };       // integrator leak pole
    double normGain { 1.0 };    // small-signal normalisation (=> ~unity)
    double fluxScale { 0.0 };   // maps integrated flux to field H
    double baseK { 0.0011 };    // flux->H scaling at drive = 1

    static inline double langevin (double z) noexcept
    {
        if (std::abs (z) < 1.0e-4) return z / 3.0;            // series near 0
        return 1.0 / std::tanh (z) - 1.0 / z;
    }
    static inline double langevinPrime (double z) noexcept
    {
        if (std::abs (z) < 1.0e-4) return 1.0 / 3.0;
        const double cth = 1.0 / std::tanh (z);
        return 1.0 - cth * cth + 1.0 / (z * z);
    }

    void setSampleRate (double sampleRate) noexcept
    {
        fs = sampleRate;
        // ~15 Hz leaky-integrator corner: a true integrator (pole near 1) paired
        // with the (1 - z^-1) differentiator is a flat high-pass above the
        // corner, so the linear path is transparent while the integrated flux
        // (large at LF) drives the core -- low frequencies saturate first.
        rInt = std::exp (-2.0 * 3.14159265358979323846 * 15.0 / fs);
        fluxScale = drive * baseK;
        updateNorm();
        calibrate();
    }

    void set (double driveAmount) noexcept
    {
        drive = driveAmount;
        setSampleRate (fs);
    }

    void updateNorm() noexcept
    {
        // Small-signal susceptibility chi = dM/dH at H->0.
        const double chi0 = (Ms / (3.0 * a));
        const double chi  = chi0 / (1.0 - alpha * chi0);
        // Above the corner the integrator's 1/(2 sin) and the differentiator's
        // 2 sin cancel, leaving linear gain = fluxScale * (1 + chi). Normalise.
        const double lin = fluxScale * (1.0 + chi);
        normGain = (std::abs (lin) > 1.0e-12) ? 1.0 / lin : 1.0;
    }

    void reset() noexcept
    {
        M = 0.0; Hprev = 0.0; lastDelta = 1.0;
        intState = 0.0; diffPrev = 0.0;
    }

    // The effective small-signal gain through the J-A loop differs from the
    // closed-form estimate (the irreversible term contributes), so measure it
    // once with a low-level tone and fold the inverse into normGain => unity.
    // Cheap and only run when the sample rate / drive actually changes.
    void calibrate() noexcept
    {
        const double saveM = M, saveH = Hprev, saveD = lastDelta,
                     saveI = intState, saveDp = diffPrev;
        normGain = 1.0;
        reset();
        const double f = 1000.0, amp = 1.0e-3;
        const int N = (int) (fs * 0.06), settle = (int) (fs * 0.03);
        double si = 0.0, so = 0.0;
        for (int n = 0; n < N; ++n)
        {
            const double v = amp * std::sin (2.0 * 3.14159265358979323846 * f * n / fs);
            const double y = process (v);
            if (n >= settle) { si += v * v; so += y * y; }
        }
        const double graw = (si > 1.0e-20) ? std::sqrt (so / si) : 1.0;
        M = saveM; Hprev = saveH; lastDelta = saveD; intState = saveI; diffPrev = saveDp;
        normGain = (graw > 1.0e-9) ? 1.0 / graw : 1.0;
    }

    inline double process (double x) noexcept
    {
        // True leaky integrator -> flux (large at LF, small at HF).
        intState = rInt * intState + x;
        const double flux = intState;

        // Field driving the core.
        const double H  = flux * fluxScale;
        double dH = H - Hprev;
        double delta = (dH > 0.0) ? 1.0 : (dH < 0.0 ? -1.0 : lastDelta);
        lastDelta = delta;

        const double He  = H + alpha * M;
        const double z   = He / a;
        const double Man = Ms * langevin (z);
        const double dManHe = (Ms / a) * langevinPrime (z);

        // Jiles-Atherton dM/dH.
        double denom1 = kPin * delta - alpha * (Man - M);
        if (std::abs (denom1) < 1.0e-9) denom1 = (denom1 < 0.0 ? -1.0e-9 : 1.0e-9);
        const double dMirr = (Man - M) / denom1;
        double dMdH = ((1.0 - c) * dMirr + c * dManHe) / (1.0 - alpha * c * dManHe);
        if (dMdH < 0.0) dMdH = 0.0;                          // enforce passivity

        M += dMdH * dH;
        M = std::clamp (M, -1.5 * Ms, 1.5 * Ms);
        Hprev = H;

        // Differentiate (B ~ H + M; the H part is the linear through-path).
        const double B = H + M;
        const double d = B - diffPrev;
        diffPrev = B;

        return d * normGain;
    }
};

} // namespace teal
