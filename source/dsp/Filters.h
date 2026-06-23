#pragma once

// =============================================================================
//  Filters.h  --  small TPT/zero-delay filters (pure C++, no JUCE).
// =============================================================================

#include <cmath>

namespace teal
{

// Zavalishin TPT state-variable filter, high-pass output (12 dB/oct, Butterworth
// Q by default). Used for the detector / sidechain high-pass.
struct SvfHP
{
    double fs { 48000.0 };
    double fc { 20.0 };
    double k  { 1.4142135623730951 };  // 1/Q, Q = 1/sqrt(2)
    double g { 0.0 }, a1 { 0.0 }, a2 { 0.0 }, a3 { 0.0 };
    double ic1 { 0.0 }, ic2 { 0.0 };
    bool   active { false };            // false => pass-through (HPF "off")

    void update() noexcept
    {
        g  = std::tan (3.14159265358979323846 * fc / fs);
        a1 = 1.0 / (1.0 + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }

    void setSampleRate (double sampleRate) noexcept { fs = sampleRate; update(); }

    // Cutoff in Hz; <= 21 Hz disables the filter (pass-through).
    void setCutoff (double hz) noexcept
    {
        active = hz > 21.0;
        fc = hz;
        update();
    }

    void reset() noexcept { ic1 = 0.0; ic2 = 0.0; }

    inline double process (double v0) noexcept
    {
        if (! active) return v0;
        const double v3 = v0 - ic2;
        const double v1 = a1 * ic1 + a2 * v3;
        const double v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0 * v1 - ic1;
        ic2 = 2.0 * v2 - ic2;
        return v0 - k * v1 - v2;       // high-pass
    }
};

} // namespace teal
