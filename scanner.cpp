#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>


Scanner::Scanner(){
  bsize = 441;
  scan_buffer.resize(bsize);
  bresize_mutex = 0;
  is_block_ready = 0;
  sample_rate = 44100.0f;
  step_size = .001; //.1 caused massive instability.
  scan_idx = 0;
  scan_freq = 110;
  
  
  //Circular configuration
  int offset_x = 0;
  int offset_y = 0;
  float radius = 4;
  for(int i = 0; i < num_nodes; i++){
    float theta = 2.0f*M_PI*i/(float)num_nodes;
    node_pos[0][i] = radius*cosf(theta);
    node_pos[1][i] = radius*sinf(theta);
    node_pos[2][i] = 0;

    node_eq_pos[0][i] = (radius+1)*cosf(theta);
    node_eq_pos[1][i] = (radius+1)*sinf(theta);
    node_eq_pos[2][i] = 0;
  }
  
  for(int i = 0; i < num_nodes; i++){
    node_vel[0][i] = 0;
    node_vel[1][i] = 0;
    node_vel[2][i] = 0;

    node_acc[0][i] = 0;
    node_acc[1][i] = 0;
    node_acc[2][i] = 0;

    node_damping[i] = 0;
    restoring_k[i] = 0;
  }

  damping_offset = -.1;
  connection_offset = 1; //without this, poorly connected waveforms can result.
  restoring_offset = 1;
  
  damping_gain = -.1; //lol this has to be negative. Or system will go unstable. Duh.
  connection_gain = 1;
  restoring_gain = 1;
  mass_gain = 1;
  
  
  fillWithWaveform(0, hammer_table, num_nodes);
  
  //startTimerHz(hz);

  //fillWithWaveform(0, node_damping, num_nodes);
  fillWithWaveform(1, connection_k, num_nodes);
  //fillWithWaveform(0, restoring_k, num_nodes);
  //fillWithWaveform(0, node_eq_pos[2], num_nodes);
  fillWithWaveform(1, node_pos[2], num_nodes);
  //fillWithWaveform(0, scan_table, num_nodes);
  fillWithWaveform(1, node_mass, num_nodes);
}

Scanner::~Scanner(){
  
}

/*
void Scanner::timerCallback(){
    //compute new positions. Newton-Euler
    //Vectorize Later
        
    for(int j = 0; j < 3; j++){ //compute acclerations
        for(int i = 0; i < num_nodes; i++){
            float sum_f = (restoring_gain + restoring_k[i])*(node_eq_pos[j][i] - node_pos[j][i]);
            
            //adding num_nodes solves problem of -1, because apparently negative number modulus is implementation specific. rip.
            int next = (i+1+num_nodes) % num_nodes;
            int prev = (i-1+num_nodes) % num_nodes;
            
            sum_f += (connection_gain + connection_k[next])*(node_pos[j][next] - node_pos[j][i]); //this is simple. too simple...
            sum_f += (connection_gain + connection_k[prev])*(node_pos[j][prev] - node_pos[j][i]);
            sum_f += -(damping_gain + node_damping[i])*node_vel[j][i];
            
            node_acc[j][i] = sum_f / (mass_gain + node_mass[i]);
        }
    }

    for(int j = 0; j < 3; j++){
        for(int i = 0; i < num_nodes; i++){
            node_pos[j][i] = time_step*node_vel[j][i] + node_pos[j][i];
        }
    }

    for(int j = 0; j < 3; j++){
        for(int i = 0; i < num_nodes; i++){ //this is split to eliminate dependency and promote vectorization
            node_vel[j][i] = time_step*node_acc[j][i] + node_vel[j][i];
        }
    }
}
*/


void Scanner::setSampleRate(float sr){
  sample_rate = sr;
}

void Scanner::strike(){
  for(int i = 0; i < num_nodes; i++){
    node_pos[2][i] = hammer_table[i];
  }
}

void Scanner::computeScanTable(){
  for(int i = 0; i < num_nodes; i++){
    scan_table[i] = node_pos[2][i];
  }
}

void Scanner::stepSystem(){
  float force[3];
  for(int i = 0; i < num_nodes; i++){
    int idx_prev = (i-1+num_nodes)%num_nodes;
    int idx_next = (i+1)%num_nodes;

    
    force[0] = (node_vel[0][i]*(node_damping[i]*damping_gain + damping_offset)) +
                     (node_pos[0][idx_prev] - node_pos[0][i])*(connection_k[idx_prev]*connection_gain + connection_offset) +
                     (node_pos[0][idx_next] - node_pos[0][i])*(connection_k[idx_next]*connection_gain + connection_offset) +
                     ((node_eq_pos[0][i]*1) - node_pos[0][i])*(restoring_k[i]*restoring_gain + restoring_offset);
    force[1] = (node_vel[1][i]*(node_damping[i]*damping_gain + damping_offset)) +
                     (node_pos[1][idx_prev] - node_pos[1][i])*(connection_k[idx_prev]*connection_gain + connection_offset) +
                     (node_pos[1][idx_next] - node_pos[1][i])*(connection_k[idx_next]*connection_gain + connection_offset) +
                     ((node_eq_pos[1][i]*1) - node_pos[1][i])*(restoring_k[i]*restoring_gain + restoring_offset);
    force[2] = (node_vel[2][i]*(node_damping[i]*damping_gain + damping_offset)) +
                     (node_pos[2][idx_prev] - node_pos[2][i])*(connection_k[idx_prev]*connection_gain + connection_offset) +
                     (node_pos[2][idx_next] - node_pos[2][i])*(connection_k[idx_next]*connection_gain + connection_offset) +
                     ((node_eq_pos[2][i]*eq_pos_gain) - node_pos[2][i])*(restoring_k[i]*restoring_gain + restoring_offset);

    node_acc[0][i] = force[0] / (node_mass[i]*mass_gain + .01f); //prevent divide by zeros
    node_acc[1][i] = force[1] / (node_mass[i]*mass_gain + .01f); //prevent divide by zeros
    node_acc[2][i] = force[2] / (node_mass[i]*mass_gain + .01f); //prevent divide by zeros
    
  }

  for(int i = 0; i < num_nodes; i++){
    node_pos[0][i] += (step_size*node_vel[0][i]);
    node_pos[1][i] += (step_size*node_vel[1][i]);
    node_pos[2][i] += (step_size*node_vel[2][i]);
    
    node_vel[0][i] += (step_size*node_acc[0][i]);
    node_vel[1][i] += (step_size*node_acc[1][i]);
    node_vel[2][i] += (step_size*node_acc[2][i]);
  }
  
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

void Scanner::getSampleBlock(float **block, int len){
  //don't access the scan_buffer. it is getting resized and will segfault.
  if(bresize_mutex){
    for(int i = 0; i < len; i++){
      block[0][i] = 0;
      block[1][i] = 0;
    }
  }
  
  int lower;
  int upper;
  float diff;
  int scan_len = num_nodes;
  float p_freq = scan_freq;
  float temp = p_freq*((float)scan_len)/(sample_rate);

  //printf("Num Nodes %d\n", num_nodes);
  for(int i = 0; i < len; i++){
    lower = floorf(scan_idx);
    upper = (lower+1) % scan_len;
    diff = scan_idx - lower;
    
    block[0][i] = scan_table[lower]*(1-diff) + scan_table[upper]*(diff); //interpolate along scan table axis
    block[1][i] = block[0][i]; //left and right audio are the same.
    
    scan_idx = fmod(scan_idx+temp, scan_len);
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
      table[i] = 2*sinf(M_PI*i/(scan_len-1));
    }
  }
  else if(hammer_num == 101){ //another special case I felt was worth including.
    for(int i = 1; i < scan_len-1; i++){
      table[i] = 5*fabs(((scan_len-1) / 2.0) - i) / ((scan_len-1)/2);
    }
  }
  else{ //load the file from AKWF.
    WavFile wavfile(fn); //opens the wav file associated with the waveform given in the string.
    //now you need to linearly interpolate to make the wavfile fit into the hammer table.
    if(scan_len == wavfile.data_len){
      for(int i = 0; i < scan_len; i++){
        table[i] = wavfile.data[i]*5;
      }
    }
    else if(scan_len < wavfile.data_len){
      for(int i = 0; i < scan_len; i++){
        table[i] = 5*wavfile.data[(int)(i*wavfile.data_len/(float)scan_len)];
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
          table[i] = 5*((index-l_index)*wavfile.data[u_index] + (u_index-index)*wavfile.data[l_index]); //linear interpolation
        else //edge case
          table[i] = 5*((index-l_index)*wavfile.data[wavfile.data_len] + (u_index-index)*wavfile.data[l_index]); 
      }            
    }
  }
}
