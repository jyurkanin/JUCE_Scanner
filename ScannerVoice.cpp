#include "ScannerVoice.h"
#include <stdio.h>
#include <math.h>
#include "log_value.h"


ScannerVoice::ScannerVoice(Scanner &s) : scanner_osc(s){
    adsr.setSampleRate(getSampleRate());

    //arbitrary
    juce::ADSR::Parameters params;
    params.attack = .1;
    params.decay = .1;
    params.sustain = .7;
    params.release = .2;
    
    adsr.setParameters(params);
    
    t_ = 0;
    scan_idx = 0.0f;
    delta_scan_idx_current = 0;
}

ScannerVoice::~ScannerVoice(){
    
}

bool ScannerVoice::canPlaySound (juce::SynthesiserSound* sound){
  return dynamic_cast<ScannerSound*> (sound) != nullptr;
}

void ScannerVoice::startNote(int midiNoteNumber, float velocity,
                             juce::SynthesiserSound* soundType, int currentPitchWheelPosition){
    level = velocity * 0.05;
    tailOff = 0.0;
    
    //Then we will need to interpolate between prev and current scan table (or else it will click)
    if(adsr.isActive()){
        if(scanner_osc.retrigger){
            table_xfade_counter = NUM_TABLE_XFADE_SAMPLES;
            scanner_osc.strike();
            scanner_osc.req_buffer_swap();
        }
    }
    else{
        scanner_osc.strike();
        scanner_osc.req_buffer_swap();
    }
    
    cycles_per_second = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    float cyclesPerSample = cycles_per_second / getSampleRate();
    
    //scan_idx = 0.0f;

    delta_scan_idx_target = scanner_osc.node_pos[0][scan_table_len - 1]*cyclesPerSample;
    if(delta_scan_idx_current == 0){
        printf("Problem delta scan idx current is 0\n");
        delta_scan_idx_current = delta_scan_idx_target;
    }
    
    max_scan_len = scanner_osc.node_pos[0][scan_table_len - 1];
    
    //t_ = 0;
    
    //delta_scan_idx = (scan_table_len - 1.0f)*cyclesPerSample;
    //max_scan_len = scan_table_len - 1;
    
    //if(!adsr.isActive()){
    adsr.noteOn();
    //}
}

void ScannerVoice::stopNote (float /*velocity*/, bool allowTailOff)  {
    //printf("stopNote\n");
    if(!allowTailOff){
        //we're being told to stop playing immediately, so reset everything..
        //voice is stolen. I think.
        //clearCurrentNote();
        //scan_idx = 0;
        clearCurrentNote();
        adsr.noteOff();
        //delta_scan_idx = 0.0f;
        printf("Dont Allow tail off\n");
    }
    else{
      printf("Allow tail off\n");
      adsr.noteOff();
    }
}

void ScannerVoice::pitchWheelMoved (int /*newValue*/) {}
void ScannerVoice::controllerMoved (int /*controllerNumber*/, int /*newValue*/) {}

/*
void ScannerVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) {
    if(!adsr.isActive()){
        return; //No need to zero buffer. Its already cleared.
    }

    float mix = expf(-8 + 6*scanner_osc.portamento_tc);
    float sample = 0;
    printf("mix %f\n", mix);
    for(int ii = startSample; ii < (startSample+numSamples); ii++){
        delta_scan_idx_current = ((1-mix)*delta_scan_idx_current) + (mix*delta_scan_idx_target);
        
        float gain = adsr.getNextSample();
        sample = sinf(M_PI*2*scan_idx);
        sample *= gain;
        
        for(auto channel = 0; channel < outputBuffer.getNumChannels(); channel++){
            outputBuffer.addSample(channel, ii, sample);
        }
        
        scan_idx = fmod(scan_idx + (delta_scan_idx_current/max_scan_len), 1);
        //scan_idx += delta_scan_idx/max_scan_len;
        //LOGFILE::log_value(delta_scan_idx_current);
    }
    
    
    if(!adsr.isActive()){ //adsr has deactivated. (finished release phase)
        clearCurrentNote();
        //t_ = 0; //this ensures that the wavetable will be updated.
        scan_idx = 0.0f;
        delta_scan_idx_current = 0;
    }
    
}
*/


void ScannerVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) {
  if(!adsr.isActive()){
    for(int ii = startSample; ii < numSamples; ii++){
      for(int channel = 0; channel < outputBuffer.getNumChannels(); channel++){
        outputBuffer.addSample(channel, ii, 0);
      }
    }
    return;
  }
  
  int lower;
  int upper;
  float diff;
  float sample;
  int temp_scan_idx = 0;
  
  float fdbk = expf(-4 + 4*scanner_osc.portamento_tc);
  delta_scan_idx_current = ((1-fdbk)*delta_scan_idx_current) + (fdbk*delta_scan_idx_target);
  for(int ii = startSample; ii < (numSamples+startSample); ii++){
      //scan table critical section
      if((t_ % 10) == 0){
          scanner_osc.timerCallbackSymEuler(); //
      }
      
      
      if(scanner_osc.should_swap){
          scanner_osc.swap_buffers();
          scanner_osc.ack_buffer_swap();
      }
      
      //figure out x position for interpolation
      for(int j = 0; j < (scan_table_len-1); j++){
          if(scanner_osc.node_pos[0][j] <= scan_idx){
              temp_scan_idx = j;
          }
          else{
              break;
          }
      }
            
            
      lower = temp_scan_idx;
      upper = lower+1;
      //lower = floorf(scan_idx);
      //upper = (lower+1) % scanner_osc.num_nodes;
            
      float lower_val = scanner_osc.node_pos[scanner_osc.buf_idx+1][lower];
      float upper_val = scanner_osc.node_pos[scanner_osc.buf_idx+1][upper];
      float mix;
      unsigned other_buf = scanner_osc.buf_idx ^ 0b10;
      if(table_xfade_counter > 0){
          mix = (float)table_xfade_counter / NUM_TABLE_XFADE_SAMPLES;
          lower_val = (mix*scanner_osc.node_pos[other_buf+1][lower]) + ((1-mix)*lower_val);
          upper_val = (mix*scanner_osc.node_pos[other_buf+1][upper]) + ((1-mix)*upper_val);
          table_xfade_counter--;
          //LOGFILE::log_value(mix);
      }
            
      diff = (scan_idx - scanner_osc.node_pos[scanner_osc.buf_idx+0][lower]) / (scanner_osc.node_pos[scanner_osc.buf_idx+0][upper] - scanner_osc.node_pos[scanner_osc.buf_idx+0][lower]);
      sample = lower_val*(1-diff) + upper_val*(diff); //interpolate along scan table axis
        
      float gain = adsr.getNextSample();
      sample *= gain;

      for(auto channel = 0; channel < outputBuffer.getNumChannels(); channel++){
          outputBuffer.addSample(channel, ii, sample);
      }
        
      scan_idx = fmod(scan_idx + delta_scan_idx_current, max_scan_len);
      t_++;
  }
    
  if(!adsr.isActive()){ //adsr has deactivated. (finished release phase)
      clearCurrentNote();
      //t_ = 0; //this ensures that the wavetable will be updated.
      scan_idx = 0.0f;
  }
}



//ensure bounded difference quotient.
void ScannerVoice::enforceLipshitz(float &sample){
  static float prev_sample = 0;
  const float L = .1f;
  sample = std::min(L, std::max(-L, sample - prev_sample)) + prev_sample;
  prev_sample = sample;
}

bool ScannerVoice::isVoiceActive() const{
    return adsr.isActive();
}
