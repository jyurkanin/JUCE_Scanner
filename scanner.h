#include <vector>
#include <math.h>

#include <cstring>
#include <cstdio>
#include <fstream>
#include <eigen3/Eigen/Dense>
#include <atomic>

#include <JuceHeader.h>

#include "filter.h"
#include "wavfile.h"

#pragma once

class Scanner : public juce::Timer{
public:
    const static int num_nodes = 600; //random.

    std::atomic<unsigned> should_swap;
    std::atomic<unsigned> buf_idx; //can be 0 or 2.
    
    float node_eq_pos[4][num_nodes];
    float node_pos[4][num_nodes];
    float node_vel[4][num_nodes];
    float node_acc[4][num_nodes];
    
    
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
    float scan_table[2][num_nodes];
    
    std::ofstream log_file;
    std::vector<float> scan_buffer;
    float scan_idx; //is a float for linear interp.
    int is_block_ready;
    
    float step_size;
    
    float scan_freq;
    float sample_rate;
    int bsize;
    int bresize_mutex;

    float distortion_c3;
    
    juce::CriticalSection mutex_scan_table_;
    LPFilter lp_filter;
    
    
    Scanner();
    ~Scanner();
    
    //Aesthetic
    void strike();

    //please do not swap buffers while the buffer is being used by these functions.
    //It will be utterly fucked
    void timerCallback(); 
    void timerCallbackEuler();
    void timerCallbackSymEuler();
    void timerCallbackRK4();
    
    void log_value(float val, float sample);
    void setFreq(float f);
    void computeScanTable();
    void setBlockSize(int bs);
    void setSampleRate(float sr);
    void getSampleBlock(float **block, int len);
    void fillWithWaveform(int wave_num, float* table, int table_len);
    
    float compressAudio(float in, float attack, float threshold, float ratio, int channel);
    void ode(float (&pos)[4][num_nodes], float (&vel)[4][num_nodes], float (&acc)[4][num_nodes]);
    void ode_fancy(float (&pos)[4][num_nodes], float (&vel)[4][num_nodes], float (&acc)[4][num_nodes]);

    void req_buffer_swap();
    void ack_buffer_swap();
    void swap_buffers();
};
