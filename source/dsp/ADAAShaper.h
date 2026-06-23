#pragma once

// =============================================================================
//  ADAAShaper.h  --  Antiderivative-antialiased static nonlinearities
//
//  Pure C++ (no JUCE) so the whole DSP core can be unit-tested offline.
//
//  Theory (see README, "Modeling the 1176 in Software", sections 4 & 1.3):
//    * A memoryless nonlinearity broadens the spectrum; harmonics above
//      Nyquist fold back as aliasing.
//    * First-order ADAA replaces  y = f(x)  with the average of f over the
//      segment [x[n-1], x[n]]:
//          y[n] = (F1(x[n]) - F1(x[n-1])) / (x[n] - x[n-1]),   F1' = f
//      which is a 1-sample-box convolution => a sinc-shaped low-pass on the
//      generated harmonics (~1 extra order of alias suppression, ~free).
//    * When x[n] ~= x[n-1] the divisor collapses (catastrophic cancellation)
//      so we fall back to the midpoint f((x+x1)/2).
//
//  The 1176's "warm/present" colour is even-harmonic dominant: it comes from
//  the -V_DS^2/2 term in the FET triode law. We get even harmonics from an
//  *asymmetric* shaper, here a biased tanh whose antiderivative stays closed
//  form (so ADAA needs no special functions beyond log-cosh).
// =============================================================================

#include <cmath>

namespace teal
{

// Numerically stable log(cosh(x)) = |x| + log(1 + e^{-2|x|}) - log(2).
inline double logCosh (double x) noexcept
{
    const double a = std::abs (x);
    return a + std::log1p (std::exp (-2.0 * a)) - 0.6931471805599453;
}

// CRTP base implementing first-order ADAA around a Derived nonlinearity that
// provides instance methods  double f(double)  and  double F1(double).
template <typename Derived>
struct ADAA1
{
    double x1 { 0.0 };

    void reset() noexcept { x1 = 0.0; }

    inline double process (double x) noexcept
    {
        auto& d = static_cast<Derived&> (*this);
        constexpr double eps = 1.0e-6;
        const double dx = x - x1;
        double y;
        if (std::abs (dx) < eps)
            y = d.f (0.5 * (x + x1));                 // midpoint fallback
        else
            y = (d.F1 (x) - d.F1 (x1)) / dx;          // averaged over the segment
        x1 = x;
        return y;
    }
};

// -----------------------------------------------------------------------------
//  Biased tanh saturator.
//
//      f(x)  = ( tanh(a*x + b) - tanh(b) ) / a
//      F1(x) = ( logCosh(a*x + b)/a - tanh(b)*x ) / a      (exact antiderivative)
//
//  a (drive)  : how hard it saturates (larger => earlier/softer ceiling).
//  b (bias)   : asymmetry => generates the even (2nd) harmonic. b = 0 is the
//               symmetric, odd-only case. The tanh(b) subtraction keeps the
//               static operating point at 0 so there is no constant DC offset.
//  Small-signal slope f'(0) = sech^2(b) ~= 1, i.e. transparent at low level.
// -----------------------------------------------------------------------------
struct BiasedTanh : ADAA1<BiasedTanh>
{
    double a  { 1.0 };
    double b  { 0.0 };
    double tb { 0.0 };   // cached tanh(b)

    void set (double drive, double bias) noexcept
    {
        a  = (drive < 1.0e-4 ? 1.0e-4 : drive);
        b  = bias;
        tb = std::tanh (b);
    }

    inline double f (double x) const noexcept
    {
        return (std::tanh (a * x + b) - tb) / a;
    }

    inline double F1 (double x) const noexcept
    {
        return (logCosh (a * x + b) / a - tb * x) / a;
    }
};

// -----------------------------------------------------------------------------
//  One-pole DC blocker (first-order high-pass), ~20 Hz corner.
//  Removes the small signal-dependent DC the asymmetric stages can introduce.
// -----------------------------------------------------------------------------
struct DCBlocker
{
    double x1 { 0.0 }, y1 { 0.0 }, R { 0.9995 };

    void setSampleRate (double fs) noexcept
    {
        // corner ~= (1-R)*fs/(2*pi). For ~20 Hz: 1-R = 2*pi*20/fs.
        R = 1.0 - 125.6637061 / fs;
        if (R < 0.0) R = 0.0;
    }

    void prepare (double fs) noexcept { setSampleRate (fs); reset(); }
    void reset() noexcept             { x1 = 0.0; y1 = 0.0; }

    inline double process (double x) noexcept
    {
        const double y = x - x1 + R * y1;
        x1 = x;
        y1 = y;
        return y;
    }
};

} // namespace teal
