#include "ScannerVoice.h"
#include <stdio.h>
#include "log_value.h"


ScannerVoice::ScannerVoice(Scanner &s) : scanner_osc(s), juce::Thread("Voice Worker Thread"){
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
    
    startThread(10);

    samples_consumed = 0;
    samples_available = 0;
    max_block_size = 1024;
    has_block_ready = 0;
    gen_next_block = 1;

    debug_counter = 0;
    
    for(int i = 0; i < audio_q_size; i++){
        audio_queue[i] = 0;
    }
}

ScannerVoice::~ScannerVoice(){
    stopThread(2000);
}

bool ScannerVoice::canPlaySound (juce::SynthesiserSound* sound){
  return dynamic_cast<ScannerSound*> (sound) != nullptr;
}

void ScannerVoice::startNote (int midiNoteNumber, float velocity,
                               juce::SynthesiserSound* soundType, int currentPitchWheelPosition){
    level = velocity * 0.05;
    tailOff = 0.0;

    { //critical section.
        const juce::ScopedLock sl(mutex_scan_table_);
        //Then we will need to interpolate between prev and current scan table (or else it will click)
        if(adsr.isActive()){
            table_xfade_counter = NUM_TABLE_XFADE_SAMPLES;
            for(int i = 0; i < scan_table_len; i++){
                prev_table[i] = scanner_osc.node_pos[1][i];
            }
        
            scanner_osc.strike();    
        }
        else{
            scanner_osc.strike();    
        }
    }
    
    cycles_per_second = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    float cyclesPerSample = cycles_per_second / getSampleRate();
    
    //scan_idx = 0.0f;
    
    delta_scan_idx = scanner_osc.node_pos[0][scan_table_len - 1]*cyclesPerSample;
    max_scan_len = scanner_osc.node_pos[0][scan_table_len - 1];
    
    //t_ = 0;
    
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
        //clearCurrentNote();
        //adsr.noteOff();
    }
    else{
        adsr.noteOff();
    }
}

void ScannerVoice::pitchWheelMoved (int /*newValue*/) {}
void ScannerVoice::controllerMoved (int /*controllerNumber*/, int /*newValue*/) {}
void ScannerVoice::run() {
    int interval = 1; //milliseconds
    while(!threadShouldExit()){
        //printf("gen block has block %d %d\n", gen_next_block, has_block_ready);
        //printf("counter %d\n", debug_counter);
        if(!gen_next_block){
            wait(interval);
        }
        else{
            //printf("generateBlock\n");
            generateBlock();
        }
    }
}

void ScannerVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) {
    debug_counter++;
    //if(numSamples > max_block_size){
    //    max_block_size = numSamples;
    //}
    if(!has_block_ready){
        printf("Buffer was not ready %d.\n", debug_counter); 
    }

    has_block_ready = 0;
    
    for(int ii = 0; ii < numSamples; ii++){
        float sample = audio_queue[ii];
        for(auto channel = 0; channel < outputBuffer.getNumChannels(); channel++){
            outputBuffer.addSample(channel, ii, sample);
        }
    }
        
    
    samples_consumed = numSamples;
    gen_next_block = 1;
    //ensures buffer wil be ready the next time this function is called.
    //hopefully removes all possibility of a buffer underrun
}

void ScannerVoice::generateBlock(){
    //debug_counter--;
    gen_next_block = 0;
    
    //fix queue.
    //Remove consumed samples
    //samples available should equal max_block_size except for the very first time this runs.
    for(int ii = samples_consumed; ii < samples_available; ii++){
        audio_queue[ii - samples_consumed] = audio_queue[ii];
    }
    
    int write_idx = samples_available - samples_consumed;
    
    if(!adsr.isActive()){
        has_block_ready = 1;
        samples_available = max_block_size;
        for(int ii = 0; ii < max_block_size; ii++){
            audio_queue[ii] = 0;
        }
        return;
    }
    
    int lower;
    int upper;
    float diff;
    float sample;
    int temp_scan_idx = 0;
    
    for(int ii = write_idx; ii < max_block_size; ii++){
        {
           //scan table critical section
            const juce::ScopedLock sl(mutex_scan_table_);
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

            float lower_val = scanner_osc.node_pos[1][lower];
            float upper_val = scanner_osc.node_pos[1][upper];
            float mix;
            if(table_xfade_counter > 0){
                mix = (float)table_xfade_counter / NUM_TABLE_XFADE_SAMPLES;
                lower_val = (mix*prev_table[lower]) + ((1-mix)*lower_val);
                upper_val = (mix*prev_table[upper]) + ((1-mix)*upper_val);
                table_xfade_counter--; 
            }
            //LOGFILE::log_value(mix);
        
            diff = (scan_idx - scanner_osc.node_pos[0][lower]) / (scanner_osc.node_pos[0][upper] - scanner_osc.node_pos[0][lower]);
            sample = lower_val*(1-diff) + upper_val*(diff); //interpolate along scan table axis

        } //end scan table critical section
        
        float gain = adsr.getNextSample();
        sample *= gain;

        audio_queue[ii] = sample;
        
        scan_idx = fmod(scan_idx + delta_scan_idx, max_scan_len);
        t_++;
    }
    
    if(!adsr.isActive()){ //adsr has deactivated. (finished release phase)
        clearCurrentNote();
        t_ = 0;
        scan_idx = 0.0f;
    }

    samples_available = max_block_size;
    has_block_ready = 1;
}

bool ScannerVoice::isVoiceActive() const{
    return adsr.isActive();
}
