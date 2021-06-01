#include <vector>
#include <math.h>

#include <JuceHeader.h>
#include <cstring>
#include <cstdio>

#include "wavfile.h"

#pragma once

class Scanner{
public:
  const static int num_nodes = 151; //random.
  
  float node_eq_pos[3][num_nodes];
  float node_pos[3][num_nodes];
  float node_vel[3][num_nodes];
  float node_acc[3][num_nodes];

  float damping_gain;
  float connection_gain;
  float restoring_gain;
  float hammer_gain;
  float eq_pos_gain;
  float mass_gain;

  float damping_offset;
  float connection_offset;
  float restoring_offset;
  
  

  float node_damping[num_nodes];
  float connection_k[num_nodes]; //k is for stiffness
  float restoring_k[num_nodes]; //restoring k points to equilibrium pos.
  
  float hammer_table[num_nodes];
  float scan_table[num_nodes];
  float node_mass[num_nodes];
  
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

  void strike();
  void shutdown();
  void stepSystem();
  void setFreq(float f);
  void computeScanTable();
  void setBlockSize(int bs);
  void setSampleRate(float sr);
  void getSampleBlock(float **block, int len);
  void fillWithWaveform(int wave_num, float* table, int table_len);
  
};
