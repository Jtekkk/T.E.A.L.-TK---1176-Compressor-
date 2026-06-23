// =============================================================================
//  dsp_offline_test.cpp  --  offline validation of the 1178 DSP core.
//
//  No JUCE, no audio device: just drives the engine with sine tones and prints
//  the static compression curve, gain reduction vs. input drive, and the
//  harmonic profile, following the validation checklist in the README (§8).
//
//  Build:  g++ -O2 -std=c++17 -I source tests/dsp_offline_test.cpp -o dsp_test
//  Run:    ./dsp_test
// =============================================================================

#include <cmath>
#include <cstdio>
#include <vector>

#include "dsp/CompressorEngine.h"

namespace
{
constexpr double kPi = 3.14159265358979323846;

double dB (double lin)      { return 20.0 * std::log10 (std::max (1e-12, lin)); }
double fromDb (double dBv)  { return std::pow (10.0, dBv / 20.0); }

// Goertzel magnitude (Hann-windowed) of frequency f in a real buffer.
double goertzel (const std::vector<double>& x, double f, double fs)
{
    const int N = (int) x.size();
    const double w = 2.0 * kPi * f / fs;
    const double coeff = 2.0 * std::cos (w);
    double s0 = 0.0, s1 = 0.0, s2 = 0.0;
    double winSum = 0.0;
    for (int n = 0; n < N; ++n)
    {
        const double win = 0.5 - 0.5 * std::cos (2.0 * kPi * n / (N - 1)); // Hann
        winSum += win;
        s0 = win * x[n] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }
    const double re = s1 - s2 * std::cos (w);
    const double im = s2 * std::sin (w);
    return 2.0 * std::sqrt (re * re + im * im) / winSum; // ~peak amplitude of that bin
}

struct Result { double outRmsDb; double grDb; };

// Feed a steady sine (post input-drive amplitude `amp`) and measure.
Result runTone (teal::CompressorEngine& eng, double fs, double freq, double amp,
                std::vector<double>* capture = nullptr)
{
    eng.reset();
    const int settle = (int) (fs * 0.5);   // 0.5 s to settle ballistics
    const int meas   = (int) (fs * 0.2);
    double sumSq = 0.0, grAccum = 0.0;
    if (capture) capture->clear();

    for (int n = 0; n < settle + meas; ++n)
    {
        const double s = amp * std::sin (2.0 * kPi * freq * n / fs);
        double frame[2] = { s, s };
        eng.processFrame (frame, 2);
        if (n >= settle)
        {
            sumSq   += frame[0] * frame[0];
            grAccum += eng.getGainReductionDb();
            if (capture) capture->push_back (frame[0]);
        }
    }
    return { dB (std::sqrt (sumSq / meas)), grAccum / meas };
}
} // namespace

int main()
{
    const double fs = 192000.0; // emulate a 4x-oversampled rate
    teal::CompressorEngine eng;
    eng.prepare (fs, 2);
    eng.setTimes (0.00025, 0.2);

    std::printf ("TEAL 1178 -- offline DSP validation\n");
    std::printf ("sample rate %.0f Hz, tone 1 kHz\n", fs);

    // --- 1) Static curve at 4:1, drive 0 dB --------------------------------
    eng.setRatioMode (teal::Ratio::R4);
    std::printf ("\n[1] Static curve  (Ratio 4:1, input drive 0 dB)\n");
    std::printf ("   in(dBFS)  out(dBFS)   GR(dB)\n");
    for (double inDb = -42.0; inDb <= 0.001; inDb += 6.0)
    {
        auto r = runTone (eng, fs, 1000.0, fromDb (inDb));
        std::printf ("   %7.1f  %8.2f  %7.2f\n", inDb, r.outRmsDb, r.grDb);
    }

    // --- 2) GR vs. input drive (signal -12 dBFS) ----------------------------
    std::printf ("\n[2] Gain reduction vs. input drive  (Ratio 4:1, signal -12 dBFS)\n");
    std::printf ("   drive(dB)  GR(dB)\n");
    for (double drv = 0.0; drv <= 36.001; drv += 6.0)
    {
        auto r = runTone (eng, fs, 1000.0, fromDb (-12.0 + drv));
        std::printf ("   %8.1f  %7.2f\n", drv, r.grDb);
    }

    // --- 3) Harmonic profile (even should dominate at moderate GR) -----------
    auto harmonics = [&] (teal::Ratio ratio, const char* name, double driveDb)
    {
        eng.setRatioMode (ratio);
        std::vector<double> buf;
        const double f = 1000.0;
        auto r = runTone (eng, fs, f, fromDb (-12.0 + driveDb), &buf);
        const double h1 = goertzel (buf, f,        fs);
        const double h2 = goertzel (buf, 2.0 * f,  fs);
        const double h3 = goertzel (buf, 3.0 * f,  fs);
        const double h4 = goertzel (buf, 4.0 * f,  fs);
        std::printf ("   %-12s GR %5.2f dB | H2 %6.2f dBc  H3 %6.2f dBc  H4 %6.2f dBc\n",
                     name, r.grDb, dB (h2 / h1), dB (h3 / h1), dB (h4 / h1));
    };

    std::printf ("\n[3] Harmonic profile  (1 kHz, +18 dB drive; dBc = relative to fundamental)\n");
    harmonics (teal::Ratio::R4,  "4:1",  18.0);
    harmonics (teal::Ratio::R8,  "8:1",  18.0);
    harmonics (teal::Ratio::R20, "20:1", 18.0);
    harmonics (teal::Ratio::All, "All-buttons", 18.0);

    // --- 4) Low-frequency grit (rectifier ripple, README §C.3) --------------
    std::printf ("\n[4] LF grit: 60 Hz vs 1 kHz, All-buttons, fast attack, +24 dB drive\n");
    eng.setRatioMode (teal::Ratio::All);
    eng.setTimes (0.00002, 0.05); // fastest attack, fast release
    for (double f : { 60.0, 1000.0 })
    {
        std::vector<double> buf;
        auto r = runTone (eng, fs, f, fromDb (-12.0 + 24.0), &buf);
        const double h1 = goertzel (buf, f,       fs);
        const double h3 = goertzel (buf, 3.0 * f, fs);
        std::printf ("   %5.0f Hz | GR %5.2f dB | H3 %6.2f dBc\n", f, r.grDb, dB (h3 / h1));
    }

    std::printf ("\nDone.\n");
    return 0;
}
