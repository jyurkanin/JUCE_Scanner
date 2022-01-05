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
  
  scan_idx = 0.0f;
  prev_scan_idx = 0;
  
  delta_scan_idx = scanner_osc.node_pos[0][scan_table_len - 1]*cyclesPerSample;
  max_scan_len = scanner_osc.node_pos[0][scan_table_len - 1];
  
  t_ = 0;
  
  //delta_scan_idx = (scan_table_len - 1.0f)*cyclesPerSample;
  //max_scan_len = scan_table_len - 1;

  //if(!adsr.isActive()){
  adsr.noteOn();
  //}
}

void ScannerVoice::stopNote (float /*velocity*/, bool allowTailOff)  {
    if(!allowTailOff){
        // we're being told to stop playing immediately, so reset everything..
        //voice is stolen. I think.
        //clearCurrentNote();
        //scan_idx = 0;
        clearCurrentNote();
        adsr.noteOff();
    }
    else{
        adsr.noteOff();
    }
}

void ScannerVoice::pitchWheelMoved (int /*newValue*/)                               {}
void ScannerVoice::controllerMoved (int /*controllerNumber*/, int /*newValue*/)     {}
void ScannerVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)  {
    float updated_table[2][total_scan_len]; //original scan table
    
    if(!adsr.isActive()){
        return;
    }
    
    int lower;
    int upper;
    float diff;
    int temp_scan_idx = 0;
    //int prev_scan_idx = 0;
    
    for(int ii = startSample; ii < numSamples; ii++){
        if((t_ % 10) == 0){
            scanner_osc.timerCallbackRK4();
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
        diff = (scan_idx - scanner_osc.node_pos[0][lower]) / (scanner_osc.node_pos[0][upper] - scanner_osc.node_pos[0][lower]);
        float sample = scanner_osc.node_pos[1][lower]*(1-diff) + scanner_osc.node_pos[1][upper]*(diff); //interpolate along scan table axis
        
        
        float gain = adsr.getNextSample();
        sample *= gain;
        
        
        for(auto channel = 0; channel < outputBuffer.getNumChannels(); channel++){
            outputBuffer.addSample(channel, ii, sample);
        }
        
        scan_idx = fmod(scan_idx + delta_scan_idx, max_scan_len);
        t_++;
    }
        
    //adsr.applyEnvelopeToBuffer(outputBuffer, startSample, numSamples);
    if(!adsr.isActive()){ //adsr has deactivated. (finished release phase)
        clearCurrentNote();
    }
}

bool ScannerVoice::isVoiceActive() const{
    return adsr.isActive();
}
