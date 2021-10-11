/*
  ==============================================================================
    scanner_window.cpp
    Created: 20 Feb 2021 11:26:40pm
    Author:  justin
  ==============================================================================
*/

#include "scanner_window.h"



ProjectedPoint ScannerWindow::project_point(Vector3f point_v, Vector3f view_v){
  ProjectedPoint pp;
  
  pp.theta = (point_v[1] - view_v[1])/( point_v[0] - view_v[0]);
  pp.phi = (point_v[2] - view_v[2])/( point_v[0] - view_v[0]);

  return pp;
}


MonoPixel ScannerWindow::projection_to_pixel(ProjectedPoint pp){
  MonoPixel mp;
  float theta_max = 2;
  float phi_max = 2;

  float scale_x = (getWidth()/2)/theta_max;
  float scale_y = (getHeight()/2)/phi_max;

  if(abs(pp.theta) > (theta_max + EPS) || abs(pp.phi) > (phi_max + EPS)){
    mp.x = -1;
    mp.y = -1;
  }
  else{
    mp.x = floor(pp.theta*scale_x + getWidth()/2);
    mp.y = floor(-pp.phi*scale_y + getHeight()/2);
  }
  return mp;
}


void ScannerWindow::drawLines(juce::Graphics& g, Vector3f *start_point, Vector3f *end_point, int len){
  ProjectedPoint pp_start;
  ProjectedPoint pp_end;

  MonoPixel mp_start;
  MonoPixel mp_end;

  juce::Line<float> line;
  
  for(int i = 0; i < len; i++){
    pp_start = project_point(start_point[i], origin);
    pp_end = project_point(end_point[i], origin);
    
    mp_start = projection_to_pixel(pp_start);
    mp_end = projection_to_pixel(pp_end);
    
    
    if((mp_start.x != -1) && (mp_end.x != -1) && ((start_point[i][0] - origin[0]) > 0) && ((end_point[i][0] - origin[0]) > 0)){
        line = juce::Line<float>(mp_start.x, mp_start.y, mp_end.x, mp_end.y);
        g.drawLine(line);
    }
  }
}






ScannerWindow::ScannerWindow(Scanner *s){
  scanner = s;
  setFramesPerSecond(30);
  origin = Vector3f(-22,0,0);
}

void ScannerWindow::update() {
  
}

void ScannerWindow::paint(juce::Graphics& g){
  static float spin = 0;
  g.fillAll(juce::Colours::black);  

  int num_nodes = scanner->num_nodes;
  Vector3f start_points[num_nodes];
  Vector3f start_points2[num_nodes];
  Vector3f end_points[num_nodes];

  const float va = -.2;
  Matrix3f rot;
  rot << cosf(va),0,sinf(va),  0,1,0,   -sinf(va),0,cosf(va);
  
  Matrix3f roty;
  roty << cosf(spin),-sinf(spin),0,  sinf(spin),cosf(spin),0,  0,0,1;
  rot = rot*roty;
  
  float x_scale = 3;
  float y_scale = 3;
  float z_scale = 8;

  float z_offset = -8;
  
  for(int i = 0; i < num_nodes; i++){
    unsigned curr_idx = i;
    unsigned prev_idx = (i-1+num_nodes) % num_nodes;
    
    start_points[i][0] = scanner->node_pos[0][curr_idx]*x_scale;
    start_points[i][1] = scanner->node_pos[1][curr_idx]*y_scale;
    start_points[i][2] = scanner->node_pos[2][curr_idx]*z_scale + z_offset;

    end_points[i][0] = scanner->node_pos[0][prev_idx]*x_scale;
    end_points[i][1] = scanner->node_pos[1][prev_idx]*y_scale;
    end_points[i][2] = scanner->node_pos[2][prev_idx]*z_scale  + z_offset;

    start_points[i] = rot*start_points[i];
    end_points[i] = rot*end_points[i];
  }
  
  g.setColour(juce::Colours::blue);
  drawLines(g, start_points, end_points, scanner->num_nodes); //draw connections between nodes.

  for(int i = 0; i < num_nodes; i++){
    unsigned prev_idx = (i-1+num_nodes) % num_nodes;
    
    start_points2[i][0] = scanner->node_eq_pos[0][i]*x_scale;
    start_points2[i][1] = scanner->node_eq_pos[1][i]*y_scale;
    start_points2[i][2] = scanner->node_eq_pos[2][i]*z_scale + z_offset;

    end_points[i][0] = scanner->node_eq_pos[0][prev_idx]*x_scale;
    end_points[i][1] = scanner->node_eq_pos[1][prev_idx]*y_scale;
    end_points[i][2] = scanner->node_eq_pos[2][prev_idx]*z_scale  + z_offset;

    start_points2[i] = rot*start_points2[i];
    end_points[i] = rot*end_points[i];
  }
  
  g.setColour(juce::Colours::red);
  drawLines(g, start_points2, end_points, scanner->num_nodes); //draw connections between nodes.
  
  
  for(int i = 0; i < num_nodes; i++){
    //start_points[i][0] = scanner->node_pos[0][i];
    //start_points[i][1] = scanner->node_pos[1][i];
    //start_points[i][2] = scanner->node_pos[2][i];

    //end_points[i][0] = scanner->node_eq_pos[0][i]*x_scale;
    //end_points[i][1] = scanner->node_eq_pos[1][i]*y_scale;
    //end_points[i][2] = scanner->node_eq_pos[2][i]*z_scale  + z_offset;

    //start_points[i] = rot*start_points[i];
    //end_points[i] = rot*end_points[i];
  }

  g.setColour(juce::Colours::green);
  drawLines(g, start_points, end_points, scanner->num_nodes); //draw connections from nodes to eq_pos
  

  float y_offset = 100;
  float x_spacing = (getWidth()/num_nodes) - 1;
  float x_offset = 10;
  float y_spacing = 20;
  juce::Line<float> line;
  for(int i = 0; i < num_nodes-1; i++){
    line = juce::Line<float>(i*x_spacing + x_offset, scanner->scan_table[i]*y_spacing + y_offset, (i+1)*x_spacing + x_offset, scanner->scan_table[i+1]*y_spacing + y_offset);
    g.drawLine(line);
  }
  line = juce::Line<float>(x_offset, y_offset, (num_nodes-1)*x_spacing + x_offset, y_offset);
  g.drawLine(line);
  
  spin += .02f; //spin it.
}


void ScannerWindow::resized() {
  //Not sure I actually need to implement this.
}

