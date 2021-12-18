#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>


Scanner::Scanner(){
  bsize = 441;
  scan_buffer.resize(bsize);
  bresize_mutex = 0;
  is_block_ready = 0;
  sample_rate = 44100.0f;
  step_size = .01;
  scan_idx = 0;
  scan_freq = 220;
  
  for(int i = 0; i < num_nodes; i++){
    node_pos[0][i] = 2*i;
    node_pos[1][i] = 0;
    
    node_eq_pos[0][i] = 2*i;
    node_eq_pos[1][i] = 0;
  }
  
  for(int i = 0; i < num_nodes; i++){
    node_vel[0][i] = 0;
    node_vel[1][i] = 0;

    node_acc[0][i] = 0;
    node_acc[1][i] = 0;
    
    node_damping[i] = 0;
    restoring_k[i] = 0;
    scan_table[i] = 0;
  }

  damping_offset = -.1;
  connection_offset = 1; //without this, poorly connected waveforms can result.
  restoring_offset = 1;
  
  damping_gain = 0; //lol this has to be negative. Or system will go unstable. Duh.
  connection_gain = 2;
  restoring_gain = .1;
  hammer_gain = 1;
  eq_pos_gain = 1;
  
  
  fillWithWaveform(8, hammer_table, num_nodes);
  fillWithWaveform(101, node_damping, num_nodes);
  fillWithWaveform(1, connection_k, num_nodes);
  fillWithWaveform(3, restoring_k, num_nodes);

  //fillWithWaveform(21, node_eq_pos[2], num_nodes);
  fillWithWaveform(0, node_pos[1], num_nodes);
  //fillWithWaveform(0, scan_table, num_nodes);
  
  for(int i = 0; i < num_nodes; i++){
    node_damping[i] = std::max(node_damping[i], 0.0f);
    connection_k[i] = std::max(connection_k[i], 0.0f);
    restoring_k[i] = std::max(restoring_k[i], 0.0f);
  }

  lp_filter.setCutoff(4000.0f);
  lp_filter.setQFactor(.1f);
}

Scanner::~Scanner(){
  
}

void Scanner::setSampleRate(float sr){
  sample_rate = sr;
}

void Scanner::strike(){
  for(int i = 0; i < num_nodes; i++){
    node_pos[1][i] = hammer_table[i];
  }
}

void Scanner::computeScanTable(){
  const juce::ScopedLock sl(mutex_scan_table_);
  //lock this function to 
  //control access to scan_table resource
  
  for(int i = 0; i < num_nodes; i++){
    //float x2 = node_pos[0][i]*node_pos[0][i];
    //float y2 = node_pos[1][i]*node_pos[1][i];
    
    scan_table[i] = node_pos[1][i]; //sqrtf(x2 + y2) - 4.0f;
  }
}


void Scanner::timerCallback(){
  timerCallbackRK4();
}

void Scanner::timerCallbackEuler(){
  for(int ii = 0; ii < 10; ii++){
  
    ode(node_pos, node_vel, node_acc);
  
    for(unsigned j = 0; j < 2; j++){
      for(unsigned i = 0; i < num_nodes; i++){
        node_pos[j][i] = node_pos[j][i] + step_size*node_vel[j][i];
        node_vel[j][i] = node_vel[j][i] + step_size*node_acc[j][i];
      }
    }


  
    computeScanTable();

  }
}


void Scanner::timerCallbackRK4(){
  float k1_pos[2][num_nodes];
  float k1_vel[2][num_nodes];
  float k1_acc[2][num_nodes];

  float k2_pos[2][num_nodes];
  float k2_vel[2][num_nodes];
  float k2_acc[2][num_nodes];

  float k3_pos[2][num_nodes];
  float k3_vel[2][num_nodes];
  float k3_acc[2][num_nodes];

  float k4_pos[2][num_nodes];
  float k4_vel[2][num_nodes];
  float k4_acc[2][num_nodes];
  
  
  for(unsigned j = 0; j < 2; j++){
    for(unsigned i = 0; i < num_nodes; i++){
      k1_pos[j][i] = node_pos[j][i];
      k1_vel[j][i] = node_vel[j][i];
    }
  }
  ode(k1_pos,k1_vel,k1_acc);

  for(unsigned j = 0; j < 2; j++){
    for(unsigned i = 0; i < num_nodes; i++){
      k2_pos[j][i] = node_pos[j][i] + step_size*k1_vel[j][i]/2.0f;
      k2_vel[j][i] = node_vel[j][i] + step_size*k1_acc[j][i]/2.0f;
    }
  }
  ode(k2_pos,k2_vel,k2_acc);

  for(unsigned j = 0; j < 2; j++){
    for(unsigned i = 0; i < num_nodes; i++){
      k3_pos[j][i] = node_pos[j][i] + step_size*k2_vel[j][i]/2.0f;
      k3_vel[j][i] = node_vel[j][i] + step_size*k2_acc[j][i]/2.0f;
    }
  }
  ode(k3_pos,k3_vel,k3_acc);

  for(unsigned j = 0; j < 2; j++){
    for(unsigned i = 0; i < num_nodes; i++){
      k4_pos[j][i] = node_pos[j][i] + step_size*k3_vel[j][i];
      k4_vel[j][i] = node_vel[j][i] + step_size*k3_acc[j][i];
    }
  }
  ode(k4_pos,k4_vel,k4_acc);

  for(unsigned j = 0; j < 2; j++){
    for(unsigned i = 0; i < num_nodes; i++){
      node_pos[j][i] = node_pos[j][i] + step_size*(k1_vel[j][i] + 2*k2_vel[j][i] + 2*k3_vel[j][i] + k4_vel[j][i])/6.0f;
      node_vel[j][i] = node_vel[j][i] + step_size*(k1_acc[j][i] + 2*k2_acc[j][i] + 2*k3_acc[j][i] + k4_acc[j][i])/6.0f;
    }
  }
  
  computeScanTable();
  
  
}
void Scanner::ode(float (&pos)[2][num_nodes], float (&vel)[2][num_nodes], float (&acc)[2][num_nodes]){
  //endpoints are fixed
  for(int i = 1; i < num_nodes-1; i++){
    int idx_prev = i-1;
    int idx_next = i+1;

    float f_damping;
    float f_spring_prev;
    float f_spring_next;
    float f_restore;
    
    for(int j = 0; j < 2; j++){
      f_damping = vel[j][i]*(node_damping[i]*damping_gain + damping_offset);
      f_spring_prev = (pos[j][idx_prev] - pos[j][i])*(connection_k[idx_prev]*connection_gain + connection_offset);
      f_spring_next = (pos[j][idx_next] - pos[j][i])*(connection_k[idx_next]*connection_gain + connection_offset);
      f_restore = ((node_eq_pos[j][i]*1) - pos[j][i])*(restoring_k[i]*restoring_gain + restoring_offset);
      
      //f_damping = 0;
      //f_spring_next = 0;
      //f_spring_prev = 0;
      //f_restore = 0;
      
      acc[j][i] =
        f_damping +
        f_spring_prev + 
        f_spring_next + 
        f_restore;
    }
  }
  
  acc[0][0] = 0;
  acc[1][0] = 0;
  acc[0][num_nodes-1] = 0;
  acc[1][num_nodes-1] = 0;
  
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
  //don't access the scan_buffer. it is getting resized and will segfault.
  if(bresize_mutex){
    for(int i = 0; i < len; i++){
      block[0][i] = 0;
      block[1][i] = 0;
    }
    return;
  }
  
  float copied_scan_table[num_nodes];
  
  {
    const juce::ScopedLock sl(mutex_scan_table_);
    for(int i = 0; i < num_nodes; i++){
      copied_scan_table[i] = scan_table[i];
    }
  }

  
  
  int lower;
  int upper;
  float diff;
  int scan_len = num_nodes;
  float p_freq = scan_freq;
  float temp = p_freq*((float)scan_len-1)/(sample_rate);
  
  float avg = 0;
  for(int i = 0; i < num_nodes; i++){
    avg += copied_scan_table[i];
  }
  avg /= (float) num_nodes;
  
  Eigen::Vector3f t_val;
  Eigen::Vector3f x_val;
  Eigen::Vector3f y_val;
  Eigen::Vector3f coeffs;
  Eigen::Matrix3f vand_matrix;
  float temp_scan_idx;
  for(int i = 0; i < len; i++){
    //linear interpolation right here. Good. Not great.
    temp_scan_idx = fmod(scan_idx, scan_len-1);
    
    lower = floorf(temp_scan_idx);
    upper = (lower+1) % (scan_len-1);
    diff = temp_scan_idx - lower;
    float sample = copied_scan_table[lower]*(1-diff) + copied_scan_table[upper]*(diff); //interpolate along scan table axis
    sample = scan_idx > (scan_len-1) ? -sample : sample;
       
    /*
    for(unsigned j = 0; j < 3; j++){
      unsigned idx = (j-1+num_nodes-1)%(num_nodes-1);
      x_val[j] = idx;
      y_val[j] = copied_scan_table[idx];
    }

    //construct vandermonde matrix
    
    for(unsigned r = 0; r < 3; r++){
      vand_matrix(r,0) = 1;
      for(unsigned c = 1; c < 3; c++){
        vand_matrix(r,c) = vand_matrix(r,c-1) * x_val[r];
      }
    }

    t_val[0] = 1;
    for(unsigned j = 1; j < 3; j++){
      t_val[j] = t_val[j-1]*temp_scan_idx; 
    }
    
    coeffs = vand_matrix.inverse()*y_val;
    
    float sample = coeffs.dot(t_val);
    */

    /*
    int lower = floorf(temp_scan_idx);
    float sample = 0;
    for(int i = -31; i < 32; i++){
      int m = i + lower;
      float x_m = copied_scan_table[(scan_len-1 + m) % (scan_len-1)];
      float temp = temp_scan_idx - m;
      float h_n_m = temp == 0 ? 1: sinf(M_PI_2*temp)/(M_PI_2*temp);
      sample += (x_m * h_n_m); //this is one of them converlutions.
    }
    */ 
    
    
    //float sample = copied_scan_table[(int)floorf(temp_scan_idx)];
    
    
    //compressAudio(sample, 100, .05, .01, 0)*.5;
    block[0][i] = sample; //lp_filter.tick(sample);
    block[1][i] = block[0][i];
    
    scan_idx = fmod(scan_idx+temp, 2*(scan_len-1));
  }  
  
}


void Scanner::fillWithWaveform(int num, float* table, int table_len){
  char fn[100];
  memset(fn, 0, sizeof(fn));
  sprintf(fn, "../../Source/AKWF/AKWF_%04d.wav", num);
  
  int scan_len = table_len;
  int hammer_num = num;
  if(hammer_num == 0){ //special case.
    for(int i = 0; i < scan_len; i++){
      table[i] = sinf(M_PI*i/(scan_len-1));
    }
  }
  else if(hammer_num == 101){ //another special case I felt was worth including.
    for(int i = 0; i < scan_len; i++){
      table[i] = fabs(((scan_len-1) / 2.0) - i) / ((scan_len-1)/2);
    }
  }
  else{ //load the file from AKWF.
    WavFile wavfile(fn); //opens the wav file associated with the waveform given in the string.
    //now you need to linearly interpolate to make the wavfile fit into the hammer table.
    if(scan_len == wavfile.data_len){
      for(int i = 0; i < scan_len; i++){
        table[i] = wavfile.data[i];
      }
    }
    else if(scan_len < wavfile.data_len){
      for(int i = 0; i < scan_len; i++){
        table[i] = wavfile.data[(int)(i*wavfile.data_len/(float)scan_len)];
      }
    }
    else if(scan_len > wavfile.data_len){ //need to linearly interpolate.
      float index;
      int l_index; //lower
      int u_index; //upper
      for(int i = 0; i < scan_len; i++){
        index = (i*wavfile.data_len/(float)scan_len);
        l_index = (int) index;
        u_index = l_index+1;
        if(u_index < wavfile.data_len)
          table[i] = ((index-l_index)*wavfile.data[u_index] + (u_index-index)*wavfile.data[l_index]); //linear interpolation
        else //edge case
          table[i] = ((index-l_index)*wavfile.data[wavfile.data_len] + (u_index-index)*wavfile.data[l_index]); 
      }            
    }
  }
}
