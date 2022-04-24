#include <JuceHeader.h>
#include "scanner.h"



struct ScannerSound : public juce::SynthesiserSound {
  ScannerSound(){}
  bool appliesToNote(int midiNoteNumber) override { return true; }
  bool appliesToChannel(int midiChannel) override { return true; }
};


//, public juce::Thread 
struct ScannerVoice  : public juce::SynthesiserVoice {
  ScannerVoice(Scanner &s);
  ~ScannerVoice();
  
  bool canPlaySound(juce::SynthesiserSound* sound) override;
  void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
  void stopNote(float /*velocity*/, bool allowTailOff) override;
  bool isVoiceActive() const override;
  void pitchWheelMoved(int /*newValue*/) override;
  void controllerMoved(int /*controllerNumber*/, int /*newValue*/) override;
  void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
  using juce::SynthesiserVoice::renderNextBlock;
  
  void generateBlock();
  //void run() override;
  
  int samples_consumed;
  int samples_available;
  int max_block_size; //this is essentially a guess. It is determined by checking the number of
  const static int audio_q_size = 8192;
  float audio_queue[audio_q_size]; //surely no one is going to raise the audio block size higher than this.
  //semaphores
  volatile int gen_next_block;
  volatile int has_block_ready;
  volatile int debug_counter;
private:
    const static int scan_table_len = Scanner::num_nodes;
    float prev_table[scan_table_len]; //used during note transitions when portamento is active.
    int table_xfade_counter = 0;
    const static int NUM_TABLE_XFADE_SAMPLES = 64;
    
    juce::CriticalSection mutex_scan_table_;
    
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
