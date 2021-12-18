#pragma once

#include <JuceHeader.h>
#include "scanner_window.h"
#include "scanner.h"

#pragma once

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent, juce::Slider::Listener {
public:
  
  MainComponent();
  ~MainComponent() override;

  
  void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
  void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
  void releaseResources() override;
  void sliderValueChanged (juce::Slider* slider) override;
  
  void paint (juce::Graphics& g) override;
  void resized() override;
  
private:
  Scanner *scanner;
  ScannerWindow *scanner_window;
  
  juce::Slider hammerTableSlider;
  juce::Label  hammerTableLabel;
  
  juce::Slider dampingTableSlider;
  juce::Label  dampingTableLabel;
  
  juce::Slider connectionTableSlider;
  juce::Label  connectionTableLabel;
  
  juce::Slider restoringTableSlider;
  juce::Label  restoringTableLabel;
  
  
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
