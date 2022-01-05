#include "ScannerVoice.h"
#include <stdio.h>


ScannerVoice::ScannerVoice(Scanner &s) : scanner_osc(s){
    adsr.setSampleRate(getSampleRate());

    //arbitrary
    juce::ADSR::Parameters params;
    params.attack = .01;
    params.decay = .1;
    params.sustain = .7;
    params.release = .2;
    
    adsr.setParameters(params);
}

bool ScannerVoice::canPlaySound (juce::SynthesiserSound* sound){
  return dynamic_cast<ScannerSound*> (sound) != nullptr;
}

void ScannerVoice::startNote (int midiNoteNumber, float velocity,
                               juce::SynthesiserSound* soundType, int currentPitchWheelPosition){
  level = velocity * 0.05;
  tailOff = 0.0;
  
  scanner_osc.strike();
  
  cycles_per_second = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
  float cyclesPerSample = cycles_per_second / getSampleRate();
  
  printf("cycles_per_second %f\n", cycles_per_second);
  
  scan_idx = 0.0f;
  prev_scan_idx = 0;
  
  delta_scan_idx = scanner_osc.node_pos[0][scan_table_len - 1]*cyclesPerSample;
  max_scan_len = scanner_osc.node_pos[0][scan_table_len - 1];

  //delta_scan_idx = (scan_table_len - 1.0f)*cyclesPerSample;
  //max_scan_len = scan_table_len - 1;
  
  //initialize scan table
  {
      const juce::ScopedLock sl(scanner_osc.mutex_scan_table_);
      for(int i = 0; i < total_scan_len; i++){
          //Left hand side.
          //the scan_table_len-1 prevents the LHS from going negative so % returns positive
          // -SINC_FILTER_LEN is an offset to ensure current_table[0][SINC_FILTER_LEN] corresponds to
          //start of the wavetable
          int mod_i = (i - SINC_FILTER_LEN + scan_table_len-1) % (scan_table_len-1);
          current_table[0][i] = scanner_osc.scan_table[0][mod_i];
          current_table[1][i] = scanner_osc.scan_table[1][mod_i];
      }
      //this is going to fix the part of the array before the actual wavetable.
      for(int i = 0; i < SINC_FILTER_LEN; i++){
          current_table[0][i] -= max_scan_len;
      }
      //fixes the part of the array after the wavetable
      for(int i = scan_table_len + SINC_FILTER_LEN; i < total_scan_len; i++){
          current_table[0][i] += max_scan_len;
      }
  }
  
  
  adsr.noteOn();
}

void ScannerVoice::stopNote (float /*velocity*/, bool allowTailOff)  {
    adsr.noteOff();
    
    if(!allowTailOff){
        // we're being told to stop playing immediately, so reset everything..
        clearCurrentNote();
        scan_idx = 0;
    }
}

void ScannerVoice::pitchWheelMoved (int /*newValue*/)                               {}
void ScannerVoice::controllerMoved (int /*controllerNumber*/, int /*newValue*/)     {}
void ScannerVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)  {
    float updated_table[2][total_scan_len]; //original scan table
    
    if(!adsr.isActive()){
        return;
    }
    
    {
        const juce::ScopedLock sl(scanner_osc.mutex_scan_table_);
        for(int i = 0; i < total_scan_len; i++){
            //Left hand side.
            //the scan_table_len-1 prevents the LHS from going negative so % returns positive
            // -SINC_FILTER_LEN is an offset to ensure updated_table[0][SINC_FILTER_LEN] corresponds to
            //start of the wavetable
            int mod_i = (i - SINC_FILTER_LEN + scan_table_len-1) % (scan_table_len-1);
            updated_table[0][i] = scanner_osc.scan_table[0][mod_i];
            updated_table[1][i] = scanner_osc.scan_table[1][mod_i];
        }
        //this is going to fix the part of the array before the actual wavetable.
        for(int i = 0; i < SINC_FILTER_LEN; i++){
            updated_table[0][i] -= max_scan_len;
        }
        //fixes the part of the array after the wavetable
        for(int i = scan_table_len + SINC_FILTER_LEN; i < total_scan_len; i++){
            updated_table[0][i] += max_scan_len;
        }
    }
    
    int lower;
    int upper;
    float diff;
    int temp_scan_idx = 0;
    //int prev_scan_idx = 0;
    
    for(int ii = startSample; ii < numSamples; ii++){
        //figure out x position for interpolation
        for(int j = 0; j < (scan_table_len-1); j++){
            if(current_table[0][j+SINC_FILTER_LEN] <= scan_idx){
                temp_scan_idx = j;
            }
            else{
                break;
            }
        }
        
        //need to detect zero crossing, and update wvetable only fter zero crossing.
        //detects when we loop around the scan table and go back to the beginning.
        if(temp_scan_idx < prev_scan_idx){
            for(int j = 0; j < total_scan_len; j++){
                current_table[0][j] = updated_table[0][j];
                current_table[1][j] = updated_table[1][j];
            }
        }
        prev_scan_idx = temp_scan_idx;
        
        
        lower = temp_scan_idx;
        upper = lower+1;
        diff = (scan_idx - current_table[0][lower+SINC_FILTER_LEN]) / (current_table[0][upper+SINC_FILTER_LEN] - current_table[0][lower+SINC_FILTER_LEN]);
        float sample = current_table[1][lower+SINC_FILTER_LEN]*(1-diff) + current_table[1][upper+SINC_FILTER_LEN]*(diff); //interpolate along scan table axis
        
        
        /*
        lower = temp_scan_idx;
        upper = lower+1;
        diff = (scan_idx - scanner_osc.node_pos[0][lower]) / (scanner_osc.node_pos[0][upper] - scanner_osc.node_pos[0][lower]);
        float sample = scanner_osc.node_pos[1][lower]*(1-diff) + scanner_osc.node_pos[1][upper]*(diff); //interpolate along scan table axis
        */
        
        /*
        lower = temp_scan_idx;
        //lower = floorf(scan_idx);
        float sample = 0;
        for(int j = -SINC_FILTER_LEN; j < SINC_FILTER_LEN; j++){
            int m = j + lower;
            float x_m = current_table[1][m+SINC_FILTER_LEN];
            
            float window = sinf(M_PI*(j+SINC_FILTER_LEN)/((2*SINC_FILTER_LEN)-1)); //hanning window. Sorry about the eye cancer.
            window = window*window;
            float temp = scan_idx - m;    //current_table[0][m+SINC_FILTER_LEN];
            float h_n_m = temp == 0 ? 1: sinf(M_PI*temp)/(M_PI*temp);
            //h_n_m = h_n_m * window;
            sample += (x_m * h_n_m); //this is one of them converlutions.
        }
        */
        
        
        float gain = adsr.getNextSample();
        sample *= gain;
        
        for(auto channel = 0; channel < outputBuffer.getNumChannels(); channel++){
            outputBuffer.addSample(channel, ii, sample);
        }
        
        scan_idx = fmod(scan_idx + delta_scan_idx, max_scan_len);
    }
    
    //adsr.applyEnvelopeToBuffer(outputBuffer, startSample, numSamples);
    if(!adsr.isActive()){ //adsr has deactivated. (finished release phase)
        clearCurrentNote();
    }
}

