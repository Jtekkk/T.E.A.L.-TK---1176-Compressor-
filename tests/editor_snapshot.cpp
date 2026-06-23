// =============================================================================
//  editor_snapshot.cpp  --  render the plugin editor to a PNG (offscreen).
//  Run under a virtual display:  xvfb-run -a ./editor_snapshot out.png
// =============================================================================

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <cstdio>
#include <memory>

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;

    TEAL1176AudioProcessor proc;
    proc.prepareToPlay (48000.0, 512);

    std::unique_ptr<juce::AudioProcessorEditor> editor (proc.createEditor());
    if (editor == nullptr) { std::printf ("FAIL: no editor\n"); return 1; }

    editor->setBounds (0, 0, editor->getWidth(), editor->getHeight());

    const auto img = editor->createComponentSnapshot (editor->getLocalBounds(), false, 1.0f);
    if (! img.isValid()) { std::printf ("FAIL: invalid snapshot\n"); return 1; }

    const juce::File out (juce::File::getCurrentWorkingDirectory()
                              .getChildFile (argc > 1 ? argv[1] : "editor.png"));
    out.deleteFile();
    if (auto os = out.createOutputStream())
    {
        juce::PNGImageFormat png;
        png.writeImageToStream (img, *os);
        std::printf ("wrote %dx%d -> %s\n", img.getWidth(), img.getHeight(),
                     out.getFullPathName().toRawUTF8());
        return 0;
    }
    std::printf ("FAIL: could not open output\n");
    return 1;
}
