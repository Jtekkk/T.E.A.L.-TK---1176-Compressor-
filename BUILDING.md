# Building TEAL 1176

**TEAL 1176** is a FET feedback compressor / limiter (a 1176-style model) built
with [JUCE](https://juce.com) and CMake. The DSP is derived directly from the
research and "one-page build recipe" in [`README.md`](README.md).

## Prerequisites

* A C++17 compiler (GCC 11+, Clang 13+, MSVC 2022, or Apple Clang)
* [CMake](https://cmake.org) 3.22 or newer
* Git (CMake fetches JUCE 8.0.14 automatically the first time you configure)

### Linux build dependencies

```bash
sudo apt-get install -y --no-install-recommends \
  libasound2-dev \
  libx11-dev libxext-dev libxinerama-dev libxrandr-dev libxcursor-dev \
  libxcomposite-dev libxrender-dev \
  libfreetype6-dev libfontconfig1-dev \
  libglu1-mesa-dev mesa-common-dev libcurl4-openssl-dev
```

(The web browser is disabled, so `webkit2gtk` is **not** required.)

macOS and Windows need no extra packages beyond Xcode / the MSVC build tools.

## Configure & build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The first configure clones JUCE and can take a couple of minutes.

### Artifacts

```
build/TEAL1176_artefacts/Release/VST3/TEAL 1176.vst3      # VST3 plugin
build/TEAL1176_artefacts/Release/Standalone/TEAL 1176     # standalone app
```

On macOS an `AU` component can be added by appending `AU` to `FORMATS` in
`CMakeLists.txt`.

#### Installing the VST3 (Linux)

```bash
mkdir -p ~/.vst3
cp -r "build/TEAL1176_artefacts/Release/VST3/TEAL 1176.vst3" ~/.vst3/
```

(macOS: `~/Library/Audio/Plug-Ins/VST3/`, Windows: `C:\Program Files\Common Files\VST3\`.)

## Tests

The DSP core is pure C++ (no JUCE) so it can be validated offline:

```bash
cmake -B build -G Ninja -DTEAL1176_BUILD_TESTS=ON
cmake --build build --target dsp_offline_test plugin_host_test

./build/dsp_offline_test    # static curve, ratios, harmonic profile, LF grit
./build/plugin_host_test     # instantiates the real plugin, runs audio, state round-trip
```

`dsp_offline_test` follows the validation checklist in the README (§8): it prints
the static compression curve, gain-reduction vs. drive, the (even-dominant)
harmonic profile per ratio, and the low-frequency "grit" that rises in
all-buttons mode.

## Source layout

```
CMakeLists.txt              top-level build (fetches JUCE, defines the plugin)
source/
  ParamIDs.h               shared parameter identifiers
  PluginProcessor.{h,cpp}  AudioProcessor: params, oversampling, block routing
  PluginEditor.{h,cpp}     the GUI
  dsp/                     pure-C++ DSP core (unit-testable, no JUCE)
    ADAAShaper.h           antiderivative-antialiased biased-tanh + DC blocker
    FETGainComputer.h      1176 feedback detector / gain computer
    Filters.h              TPT sidechain high-pass
    Transformer.h          Jiles-Atherton transformer (flux-integration) iron
    CompressorEngine.h     full per-sample signal chain + ratio table
  gui/
    LookAndFeel1176.h      knob / combo styling
    GainReductionMeter.h   VU-style GR needle
    LevelMeter.h           input/output peak meters
    RatioSelector.h        the 4 ratio buttons + "All" (British mode)
tests/
  dsp_offline_test.cpp     offline DSP validation (no JUCE)
  plugin_host_test.cpp     headless AudioProcessor smoke test
  editor_snapshot.cpp      renders the editor to a PNG (run under xvfb-run)
.github/workflows/build.yml  CI: macOS/Windows/Linux build + release on v* tags
packaging/windows/installer.iss  Inno Setup script for the Windows installer
```

## Continuous integration & releases

`.github/workflows/build.yml` builds the plugin on macOS, Windows and Linux for
every push/PR, runs the test suite on Linux, and uploads the built VST3 / AU /
Standalone as workflow artifacts. The Windows job also builds an **Inno Setup
installer** (`TEAL1176-Windows-Installer` artifact) that installs the VST3 into
`C:\Program Files\Common Files\VST3` and the standalone into Program Files.
Pushing a tag like `v0.1.0` zips each platform's build and publishes a GitHub
Release with the Windows installer attached. AU is built on macOS only.

To build the installer locally on Windows (after a Release build), install
[Inno Setup](https://jrsoftware.org/isdl.php) and run:

```bat
ISCC.exe /DAppVersion=0.1.0 packaging\windows\installer.iss
```

## How the model maps to the hardware

| 1176 concept | Where it lives |
|---|---|
| FET as voltage-variable resistor / divider gain | `CompressorEngine::processFrame` (`* g`) |
| Feedback detection (senses post-GR), ratio `R = 1+k` | `FETGainComputer` |
| Higher ratio raises threshold; all-buttons grind | `CompressorEngine::setRatioMode` |
| Even-harmonic FET/iron colour (`-V_DS²/2` term) | `BiasedTanh` shapers |
| Retained rectifier ripple → LF grit | un-oversmoothed one-pole in `FETGainComputer` |
| 20 µs attack, zero-latency (feedback, no lookahead) | 1-sample control delay, IIR oversampling |
| ADAA + oversampling antialiasing | `ADAA1` + `juce::dsp::Oversampling` |
