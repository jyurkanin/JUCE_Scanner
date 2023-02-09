#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <memory>


#define CONST_DX 8.0f



Scanner::Scanner(){
    //log_file.open("/home/justin/juce_log.csv");
    //log_file << "idx,sample\n";
  
  bsize = 441;
  scan_buffer.resize(bsize);
  bresize_mutex = 0;
  is_block_ready = 0;
  sample_rate = 44100.0f;
  step_size = .03;
  scan_idx = 0;
  scan_freq = 220;
  
  for(int i = 0; i < num_nodes; i++){
    node_pos[0][i] = CONST_DX*i;
    node_pos[1][i] = 0;
    node_pos[2][i] = CONST_DX*i;
    node_pos[3][i] = 0;
    
    node_eq_pos[0][i] = CONST_DX*i;
    node_eq_pos[1][i] = 0;
  }
  
  for(int i = 0; i < num_nodes; i++){
    node_vel[0][i] = 0;
    node_vel[1][i] = 0;
    node_vel[2][i] = 0;
    node_vel[3][i] = 0;
    
    node_acc[0][i] = 0;
    node_acc[1][i] = 0;
    node_acc[2][i] = 0;
    node_acc[3][i] = 0;
    
    scan_table[0][i] = node_pos[0][i];
    scan_table[1][i] = 0;
  }
  
  damping_gain = 0; //lol this has to positive. Or system will go unstable. Duh.
  connection_gain = 1;
  portamento_tc = 0.0f;
  retrigger = 1;
  
  fillWithWaveform("", hammer_table, num_nodes);
  
  should_swap = 0;
  buf_idx = 0;
}

Scanner::~Scanner(){
    //log_file.close();
}

void Scanner::log_value(float val, float sample){
    //log_file << val << ',' << sample << '\n';
}

void Scanner::setSampleRate(float sr){
  sample_rate = sr;
}

void Scanner::strike(){
    int other_buf = buf_idx ^ 0b10;
    for(int i = 0; i < num_nodes; i++){
        node_pos[other_buf+1][i] = hammer_table[i];
        node_vel[other_buf][i] = 0;
        node_vel[other_buf+1][i] = 0;
    }
}

//this isn't used.
void Scanner::computeScanTable(){
  const juce::ScopedLock sl(mutex_scan_table_);
  //lock this function to 
  //control access to scan_table resource
  
  for(int i = 0; i < num_nodes; i++){
      //float x2 = node_pos[0][i]*node_pos[0][i];
      //float y2 = node_pos[1][i]*node_pos[1][i];
      
      scan_table[0][i] = node_pos[0][i];
      scan_table[1][i] = node_pos[1][i];
  }
}


void Scanner::timerCallback(){
    timerCallbackRK4();
}

void Scanner::timerCallbackEuler(){
    //for(int ii = 0; ii < 10; ii++){
  
    ode(node_pos, node_vel, node_acc);
    
    for(unsigned j = buf_idx; j < (buf_idx + 2); j++){
      for(unsigned i = 0; i < num_nodes; i++){
        node_pos[j][i] = node_pos[j][i] + step_size*node_vel[j][i];
        node_vel[j][i] = node_vel[j][i] + step_size*node_acc[j][i];
      }
    }
    
    //computeScanTable();
    
    //}
}


//This is the symplectic euler integrator (for hamilotonian systems)
void Scanner::timerCallbackSymEuler(){
    ode(node_pos, node_vel, node_acc);
    
    for(unsigned j = buf_idx; j < (buf_idx + 2); j++){
        for(unsigned i = 0; i < num_nodes; i++){
            node_vel[j][i] = node_vel[j][i] + step_size*node_acc[j][i];
            node_pos[j][i] = node_pos[j][i] + step_size*node_vel[j][i];
        }
    }
}



void Scanner::timerCallbackRK4(){
    //I could have had everything be 2 rows and num_nodes cols but I was lazy.
    //And memory is nearly infinite so who cares.
    float k1_pos[4][num_nodes];
    float k1_vel[4][num_nodes];
    float k1_acc[4][num_nodes];
    
    float k2_pos[4][num_nodes];
    float k2_vel[4][num_nodes];
    float k2_acc[4][num_nodes];
    
    float k3_pos[4][num_nodes];
    float k3_vel[4][num_nodes];
    float k3_acc[4][num_nodes];
    
    float k4_pos[4][num_nodes];
    float k4_vel[4][num_nodes];
    float k4_acc[4][num_nodes];
  
  
    for(unsigned j = buf_idx; j < (buf_idx + 2); j++){
        for(unsigned i = 0; i < num_nodes; i++){
            k1_pos[j][i] = node_pos[j][i];
            k1_vel[j][i] = node_vel[j][i];
        }
    }
    ode(k1_pos,k1_vel,k1_acc);

    for(unsigned j = buf_idx; j < (buf_idx + 2); j++){
        for(unsigned i = 0; i < num_nodes; i++){
            k2_pos[j][i] = node_pos[j][i] + step_size*k1_vel[j][i]/2.0f;
            k2_vel[j][i] = node_vel[j][i] + step_size*k1_acc[j][i]/2.0f;
        }
    }
    ode(k2_pos,k2_vel,k2_acc);

    for(unsigned j = buf_idx; j < (buf_idx + 2); j++){
        for(unsigned i = 0; i < num_nodes; i++){
            k3_pos[j][i] = node_pos[j][i] + step_size*k2_vel[j][i]/2.0f;
            k3_vel[j][i] = node_vel[j][i] + step_size*k2_acc[j][i]/2.0f;
        }
    }
    ode(k3_pos,k3_vel,k3_acc);

    for(unsigned j = buf_idx; j < (buf_idx + 2); j++){
        for(unsigned i = 0; i < num_nodes; i++){
            k4_pos[j][i] = node_pos[j][i] + step_size*k3_vel[j][i];
            k4_vel[j][i] = node_vel[j][i] + step_size*k3_acc[j][i];
        }
    }
    ode(k4_pos,k4_vel,k4_acc);

    for(unsigned j = buf_idx; j < (buf_idx + 2); j++){
        for(unsigned i = 0; i < num_nodes; i++){
            node_pos[j][i] = node_pos[j][i] + step_size*(k1_vel[j][i] + 2*k2_vel[j][i] + 2*k3_vel[j][i] + k4_vel[j][i])/6.0f;
            node_vel[j][i] = node_vel[j][i] + step_size*(k1_acc[j][i] + 2*k2_acc[j][i] + 2*k3_acc[j][i] + k4_acc[j][i])/6.0f;
        }
    }
  
    //computeScanTable();
  
}

/*
void Scanner::ode(float (&pos)[2][num_nodes], float (&vel)[2][num_nodes], float (&acc)[2][num_nodes]){
    //endpoints are fixed
    for(int j = 0; j < 2; j++){
        float f_damping;
        float f_spring_prev;
        float f_spring_next;

        float diff;
        
        int idx_prev = 0;
        int idx_next = 2;
        for(int i = 1; i < num_nodes-1; i++){
            f_damping = vel[j][i]*-damping_gain;
            
            
            
            f_spring_prev = (pos[j][idx_prev] - pos[j][i])*connection_gain;
            f_spring_next = (pos[j][idx_next] - pos[j][i])*connection_gain;
            
            acc[j][i] = f_damping + f_spring_prev + f_spring_next;
            
            idx_prev++;
            idx_next++;
        }
    }
    
    acc[0][0] = 0; //make sure boundary conditions are satisfied.
    acc[1][0] = 0;
    acc[0][num_nodes-1] = 0;
    acc[1][num_nodes-1] = 0;
}
*/


void Scanner::ode(float (&pos)[4][num_nodes], float (&vel)[4][num_nodes], float (&acc)[4][num_nodes]){
    //endpoints are fixed
    for(int j = buf_idx; j < (buf_idx + 2); j++){
        float f_damping;
        float f_spring_prev;
        float f_spring_next;
        
        float diff;
        
        int idx_prev = 0;
        int idx_next = 2;
        for(int i = 1; i < num_nodes-1; i++){
            f_damping = vel[j][i]*-damping_gain;
            
            diff = pos[j][idx_prev] - pos[j][i];
            f_spring_prev = diff*connection_gain;
            
            diff = pos[j][idx_next] - pos[j][i];
            f_spring_next = diff*connection_gain;
            
            acc[j][i] = f_damping + f_spring_prev + f_spring_next;
            
            idx_prev++;
            idx_next++;
        }
        
        f_damping = vel[j][0]*-damping_gain;
        diff = pos[j][1] - pos[j][0];
        
        f_spring_next = diff*connection_gain;
        acc[j][0] = f_damping + f_spring_next;
        
        
        
        f_damping = vel[j][num_nodes-1]*-damping_gain;
        diff = pos[j][num_nodes-2] - pos[j][num_nodes-1];
        
        f_spring_prev = diff*connection_gain;
        acc[j][num_nodes-1] = f_damping + f_spring_prev;
    }
    
    // acc[buf_idx][0] = 0; //make sure boundary conditions are satisfied.
    // acc[buf_idx+1][0] = 0;
    // acc[buf_idx][num_nodes-1] = 0;
    // acc[buf_idx+1][num_nodes-1] = 0;

    acc[buf_idx][num_nodes-1] += acc[buf_idx][0];
    acc[buf_idx+1][num_nodes-1] += acc[buf_idx+1][0];
    acc[buf_idx][0] = acc[buf_idx][num_nodes-1];
    acc[buf_idx+1][0] = acc[buf_idx+1][num_nodes-1];
    
    
}


void Scanner::setFreq(float f){
  scan_freq = f;
}



void Scanner::setBlockSize(int bs){
  if(bsize != bs){
    bresize_mutex = 1;
    scan_buffer.resize(bs);
    bsize = bs;
    bresize_mutex = 0;
  }
}

float Scanner::compressAudio(float in, float attack, float threshold, float ratio, int channel){
    static float out_rms[2] = {0,0};
    static float curr_rms[2] = {0,0}; //lpf rms.
    
    float in_rms = fabs(in); //oh this isnt actually the root-mean-square. oh rip.
    float out;
    
    curr_rms[channel] += (in_rms - curr_rms[channel])/attack; //Look at this stupid ass approximation of the signal amplitude.
    
    float norm_sample = 0;
    //if(curr_rms != 0)
    norm_sample = in / std::max(curr_rms[channel], 1e-3f);
    
    
    if(curr_rms[channel] > threshold){
        out_rms[channel] = (threshold + ((curr_rms[channel] - threshold)*ratio));
        out = norm_sample*out_rms[channel];
    }
    else{
        out_rms[channel] = curr_rms[channel];
        out = norm_sample*curr_rms[channel];
    }
    
    return out;
}


void Scanner::getSampleBlock(float **block, int len){ 
    //ignoreme.
}


void Scanner::fillWithWaveform(juce::String fn, float* table, int table_len){
  int scan_len = table_len;
  juce::File file(fn);
  
  if(fn.length() == 0 || !file.existsAsFile()){
    for(int i = 0; i < table_len; i++){
      table[i] = 0;
    }
    return;
  }
  
  juce::AudioFormatManager format_manager;
  format_manager.registerBasicFormats();
  
  juce::AudioFormatReader* reader = format_manager.createReaderFor(file);
  long int num_samples = reader->lengthInSamples;
  juce::AudioBuffer<float> buffer(reader->numChannels, num_samples);
  reader->read(&buffer, 0, num_samples, 0, true, true);
  const float *file_data = buffer.getReadPointer(0);
  
  //now you need to linearly interpolate to make the audio file fit into the hammer table.
  if(scan_len == num_samples){
    for(int i = 0; i < scan_len; i++){
      table[i] = file_data[i];
    }
  }
  else if(scan_len <= num_samples){
    for(int i = 0; i < scan_len; i++){
      table[i] = file_data[(int)(i*num_samples/(float)scan_len)];
    }
  }
  else if(scan_len > num_samples){ //need to linearly interpolate.
    float index;
    int l_index; //lower
    int u_index; //upper
    for(int i = 0; i < scan_len; i++){
      index = (i*num_samples/(float)scan_len);
      l_index = (int) index;
      u_index = l_index+1;
      if(u_index < num_samples)
        table[i] = ((index-l_index)*file_data[u_index] + (u_index-index)*file_data[l_index]); //linear interpolation
      else //edge case
        table[i] = ((index-l_index)*file_data[num_samples] + (u_index-index)*file_data[l_index]); 
    }            
  }
  
  for(int i = 1; i < scan_len; i++){
    table[i] -= table[0];
  }
  table[0] -= table[0];
  table[scan_len-1] = table[0];
}


//Swap completed. 
void Scanner::ack_buffer_swap(){
    should_swap = 0;
}

//request swap. Will be performed when it is safe to do so.
void Scanner::req_buffer_swap(){
    should_swap = 1;
}

void Scanner::swap_buffers(){
    buf_idx ^= 0b10; //nice
}












