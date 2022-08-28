/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             Scanner
 version:          1.0.0
 vendor:           Justin
 website:          http://juce.com
 description:      Dope Scanner Plugin

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2017, linux_make, xcode_iphone

 type:             AudioProcessor
 mainClass:        MainComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/



#pragma once

#include <JuceHeader.h>
#include "ScannerVoice.h"
#include "scanner_window.h"
#include "WaveTerrainWindow.h"
#include "scanner.h"
#include "log_value.h"
//#include "ScannerPlugin.h"


class ScannerPlugin;

class MainComponent : public juce::AudioProcessorEditor, juce::Slider::Listener {
public:
    
    MainComponent(ScannerPlugin& scanner_plugin);
    ~MainComponent() override;
    

    void sliderValueChanged (juce::Slider* slider) override;
    void openWaveformButtonClicked();
    void retriggerButtonClicked();
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
private:
    ScannerWindow *scanner_window;
    std::unique_ptr<WaveTerrainWindow> terrain_window;
    
    juce::TextButton openWaveformButton;
    juce::TextButton retriggerButton;
    
    juce::Slider dampingSlider;
    juce::Label  dampingLabel;
    
    juce::Slider connectionSlider;
    juce::Label  connectionLabel;
    
    juce::Slider portamentoSlider;
    juce::Label  portamentoLabel;
    
    std::unique_ptr<juce::FileChooser> fileChooser;
    
    juce::ComboBox midiInputList;
    juce::Label midiInputListLabel;
    int lastInputIndex = 0;

    ScannerPlugin &plugin;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
