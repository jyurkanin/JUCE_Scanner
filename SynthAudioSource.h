#include <JuceHeader.h>
#include "ScannerVoice.h"

//==============================================================================
// This is an audio source that streams the output of our demo synth.
struct SynthAudioSource  : public juce::AudioSource {
    SynthAudioSource (juce::MidiKeyboardState& keyState);
    
    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    
    juce::MidiMessageCollector midiCollector;
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
};

