#include <vector>
#include <math.h>
#include <cstring>
#include <cstdio>
#include <eigen3/Eigen/Dense>

#include <JuceHeader.h>

#include "wavfile.h"

#pragma once

class Scanner : public juce::Timer{
public:
  const static int num_nodes = 351; //random.
  
  float node_eq_pos[3][num_nodes];
  float node_pos[3][num_nodes];
  float node_vel[3][num_nodes];
  float node_acc[3][num_nodes];

  float damping_gain;
  float connection_gain;
  float restoring_gain;
  float hammer_gain;
  float eq_pos_gain;

  float damping_offset;
  float connection_offset;
  float restoring_offset;
  
  
  float node_damping[num_nodes];
  float connection_k[num_nodes]; //k is for stiffness
  float restoring_k[num_nodes]; //restoring k points to equilibrium pos.
  
  float hammer_table[num_nodes];
  float scan_table[num_nodes];
  
  std::vector<float> scan_buffer;
  float scan_idx; //is a float for linear interp.
  int is_block_ready;
  
  float step_size;
  float scan_freq;
  float sample_rate;
  int bsize;
  int bresize_mutex;
  

  Scanner();
  ~Scanner();

  //Aesthetic
  void strike();
  void timerCallback();
  void setFreq(float f);
  void computeScanTable();
  void setBlockSize(int bs);
  void setSampleRate(float sr);
  void getSampleBlock(float **block, int len);
  void fillWithWaveform(int wave_num, float* table, int table_len);
  float compressAudio(float in, float attack, float threshold, float ratio, int channel);
  void ode(float (&pos)[3][num_nodes], float (&vel)[3][num_nodes], float (&acc)[3][num_nodes]);
};
