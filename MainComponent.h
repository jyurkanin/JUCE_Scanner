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
class MainComponent : public juce::AudioAppComponent {
public:
  
  MainComponent();
  ~MainComponent() override;

  
  void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
  void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
  void releaseResources() override;

  
  void paint (juce::Graphics& g) override;
  void resized() override;
  
private:
  Scanner *scanner;
  ScannerWindow *scanner_window;
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
