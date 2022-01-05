#include <JuceHeader.h>
#include "ScannerVoice.h"

//==============================================================================
// This is an audio source that streams the output of our demo synth.
struct SynthAudioSource  : public AudioSource {
    SynthAudioSource (MidiKeyboardState& keyState);
    
    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    
    MidiMessageCollector midiCollector;
    MidiKeyboardState& keyboardState;
    Synthesiser synth;
};

