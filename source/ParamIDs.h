#pragma once

// Centralised parameter identifiers so the processor and editor never disagree.
namespace pid
{
    inline constexpr const char* input   = "input";    // input drive (dB)
    inline constexpr const char* output  = "output";   // output / make-up (dB)
    inline constexpr const char* attack  = "attack";   // attack time (us)
    inline constexpr const char* release = "release";  // release time (ms)
    inline constexpr const char* ratio   = "ratio";    // 4:1 / 8:1 / 12:1 / 20:1 / All
    inline constexpr const char* mix     = "mix";      // dry/wet (%)
    inline constexpr const char* os      = "os";       // oversampling: 1x/2x/4x/8x
    inline constexpr const char* bypass  = "bypass";   // true bypass
}
