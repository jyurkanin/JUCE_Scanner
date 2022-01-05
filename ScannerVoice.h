#include <JuceHeader.h>
#include "scanner.h"



struct ScannerSound : public juce::SynthesiserSound {
  ScannerSound(){}
  bool appliesToNote(int midiNoteNumber) override { return true; }
  bool appliesToChannel(int midiChannel) override { return true; }
};



struct ScannerVoice  : public juce::SynthesiserVoice {
  ScannerVoice(Scanner &s);
  bool canPlaySound(juce::SynthesiserSound* sound) override;
  void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
  void stopNote(float /*velocity*/, bool allowTailOff) override;
  bool isVoiceActive() const override;
  void pitchWheelMoved(int /*newValue*/) override;
  void controllerMoved(int /*controllerNumber*/, int /*newValue*/) override;
  void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
  using juce::SynthesiserVoice::renderNextBlock;
  
private:
    const static int SINC_FILTER_LEN = 32; //goes +-32. SO more like 64 but not really.
    const static int scan_table_len = Scanner::num_nodes;
    const static int total_scan_len = scan_table_len + 2*SINC_FILTER_LEN;
    
    //making this wavetable oversized makes the math easier.
    //otherwise I have to do a lot of math with modulus
    float current_table[2][total_scan_len]; 
    
    Scanner &scanner_osc;
    float scan_idx = 0.0;
    int prev_scan_idx = 0;
    float delta_scan_idx = 0.0;
    float max_scan_len = 0.0;
    float cycles_per_second = 0.0f;
    
    unsigned t_;
    
    float level = 0.0;
    float tailOff = 0.0;
    
    juce::ADSR adsr;
};
