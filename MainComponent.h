#pragma once

#include <JuceHeader.h>
#include "ScannerVoice.h"
#include "scanner_window.h"
#include "scanner.h"

#pragma once

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
//==============================================================================

class MainComponent : public juce::AudioAppComponent, juce::Slider::Listener, juce::Timer {
public:
  
  MainComponent();
  ~MainComponent() override;
  
  void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
  void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
  void releaseResources() override;
  void sliderValueChanged (juce::Slider* slider) override;
  void openWaveformButtonClicked();
  void retriggerButtonClicked();
  
  void setMidiInput (int index);
  void timerCallback() override;
    
  void paint (juce::Graphics& g) override;
  void resized() override;
  
private:
  juce::Synthesiser synth;
  Scanner scanner;
  ScannerWindow *scanner_window;
  
  juce::TextButton openWaveformButton;
  juce::TextButton retriggerButton;
  
  juce::Slider dampingSlider;
  juce::Label  dampingLabel;
  
  juce::Slider connectionSlider;
  juce::Label  connectionLabel;

  juce::Slider portamentoSlider;
  juce::Label  portamentoLabel;
    
  std::unique_ptr<juce::FileChooser> fileChooser;
  juce::MidiKeyboardState keyboardState;
  juce::MidiKeyboardComponent keyboardComponent;
  juce::MidiMessageCollector midiCollector;

  juce::ComboBox midiInputList;
  juce::Label midiInputListLabel;
  int lastInputIndex = 0;

    
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
