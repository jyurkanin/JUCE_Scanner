#include <JuceHeader.h>
#include <eigen3/Eigen/Dense>
#include "scanner.h"

#pragma once


#define EPS 1e-7


typedef struct {
  float theta, phi;
} ProjectedPoint;


typedef struct{
  int x, y;
} MonoPixel;

//typedef float[3] Vector3f;
using namespace Eigen;



class ScannerWindow : public juce::AnimatedAppComponent{
public:
  ScannerWindow(Scanner *s);
  void update() override;
  void paint(juce::Graphics& g) override;
  void resized() override;


  ProjectedPoint project_point(Vector3f point_v, Vector3f view_v);
  MonoPixel projection_to_pixel(ProjectedPoint pp);
  void drawLines(juce::Graphics& g, Vector3f *start_point, Vector3f *end_point, int len);
  
private:
    Scanner *scanner;
    Vector3f origin;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScannerWindow)
};
