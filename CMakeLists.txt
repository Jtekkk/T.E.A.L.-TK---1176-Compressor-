cmake_minimum_required(VERSION 3.22)

project(TEAL1176 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(TEAL1176_BUILD_TESTS "Build the offline DSP validation harness (no JUCE needed)" OFF)

# ---------------------------------------------------------------------------
# JUCE (pulled automatically the first time you configure)
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.14
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

# ---------------------------------------------------------------------------
# The plugin target
# ---------------------------------------------------------------------------
juce_add_plugin(TEAL1176
    COMPANY_NAME              "T.E.A.L."
    PLUGIN_MANUFACTURER_CODE  Teal
    PLUGIN_CODE               T176
    FORMATS                   VST3 Standalone
    PRODUCT_NAME              "TEAL 1176"
    DESCRIPTION               "FET feedback compressor / limiter modelled on the 1176"
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD   FALSE
    VST3_CATEGORIES           Fx Dynamics)

target_sources(TEAL1176 PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp)

target_include_directories(TEAL1176 PRIVATE source)

target_compile_definitions(TEAL1176 PUBLIC
    JUCE_WEB_BROWSER=0          # avoids the webkit2gtk dependency on Linux
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0)

target_link_libraries(TEAL1176 PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

# ---------------------------------------------------------------------------
# Offline DSP test (pure C++, builds without JUCE)
# ---------------------------------------------------------------------------
if(TEAL1176_BUILD_TESTS)
    add_executable(dsp_offline_test tests/dsp_offline_test.cpp)
    target_include_directories(dsp_offline_test PRIVATE source)
    set_target_properties(dsp_offline_test PROPERTIES CXX_STANDARD 17)

    # Headless harness that instantiates the real AudioProcessor and runs audio
    # through it (params, oversampling, state round-trip) -- no GUI/audio device.
    add_executable(plugin_host_test tests/plugin_host_test.cpp)
    target_link_libraries(plugin_host_test PRIVATE
        TEAL1176
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags)

    # Renders the editor to a PNG (run under a virtual display, e.g. xvfb-run).
    add_executable(editor_snapshot tests/editor_snapshot.cpp)
    target_include_directories(editor_snapshot PRIVATE source)
    target_link_libraries(editor_snapshot PRIVATE
        TEAL1176
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags)
endif()
