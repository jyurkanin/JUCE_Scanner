/*
  ==============================================================================
    scanner_window.cpp
    Created: 20 Feb 2021 11:26:40pm
    Author:  justin
  ==============================================================================
*/

#include "scanner_window.h"


ScannerWindow::ScannerWindow(Scanner *s) : scanner(s){
  setFramesPerSecond(60);
  setSize(800, 200);
}

void ScannerWindow::update() {
  //Most of the updates actually happen in the audio thread.
}

void ScannerWindow::paint(juce::Graphics& g){
  static float spin = 0;
  g.fillAll(juce::Colours::black);  

  int num_nodes = scanner->num_nodes;
  Vector2f start_points[num_nodes];
  Vector2f start_points2[num_nodes];
  Vector2f end_points[num_nodes];
  
  float x_scale = getWidth() / (scanner->node_pos[0][num_nodes-1] - scanner->node_pos[0][0]);
  float y_scale = 100;
  
  g.setColour(juce::Colours::red);
  for(int i = 1; i < num_nodes; i++){
    unsigned prev_idx = i-1;
    
    start_points2[i][0] = scanner->node_eq_pos[0][i]*x_scale;
    start_points2[i][1] = scanner->node_eq_pos[1][i]*y_scale + 100;
    
    end_points[i][0] = scanner->node_eq_pos[0][prev_idx]*x_scale;
    end_points[i][1] = scanner->node_eq_pos[1][prev_idx]*y_scale + 100;
    
    juce::Line<float> line = juce::Line<float>(start_points2[i][0], start_points2[i][1], end_points[i][0], end_points[i][1]);
    g.drawLine(line, 2.0f);
  }
  
  
  for(int i = 1; i < num_nodes; i++){
    unsigned curr_idx = i;
    unsigned prev_idx = i-1;
    
    start_points[i][0] = scanner->node_pos[0][curr_idx]*x_scale;
    start_points[i][1] = scanner->node_pos[1][curr_idx]*y_scale + 100;
    
    end_points[i][0] = scanner->node_pos[0][prev_idx]*x_scale;
    end_points[i][1] = scanner->node_pos[1][prev_idx]*y_scale + 100;
    
    g.setColour(juce::Colours::blue);
    juce::Line<float> line = juce::Line<float>(start_points[i][0], start_points[i][1], end_points[i][0], end_points[i][1]);
    g.drawLine(line, 2.0f);
    
    //g.setColour(juce::Colours::green);
    //line = juce::Line<float>(start_points[i][0], start_points[i][1]-5, start_points[i][0], start_points[i][1]+5);
    //g.drawLine(line, 2.0f);
  }  
  
}


void ScannerWindow::resized() {
  //Don't do anything here. Its all taken care of in the parent's resize()
}

