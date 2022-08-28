#include "MainComponent.h"
#include "ScannerPlugin.h"

#include <stdio.h>




MainComponent::MainComponent(ScannerPlugin& scanner_plugin) : AudioProcessorEditor(scanner_plugin), plugin(scanner_plugin) {
    // for(int i = 0; i < io_devices.size(); i++){
    //   juce::String device_name = io_devices[i]->getTypeName();
    //   printf("Device %s\n", device_name.toRawUTF8());
    // }
    //printf("Connected to audio driver: %s\n", deviceManager.getCurrentAudioDeviceType().toRawUTF8());
    
    //the midi gui bullshit (mostly copy pasted from the tutorial)
    
    //Custom scanner related stuff
    LOGFILE::log_value(5.0f);
    
    scanner_window = new ScannerWindow(&(plugin.scanner));
    
    addAndMakeVisible(scanner_window);
    
    //various sliders
    fileChooser = std::make_unique<juce::FileChooser>("Select Waveform",
                                                juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                                                "*.wav");
    
    addAndMakeVisible(&openWaveformButton);
    openWaveformButton.setButtonText("Open Waveform");
    openWaveformButton.onClick = [this] { openWaveformButtonClicked(); };
    
    addAndMakeVisible(&retriggerButton);
    retriggerButton.setButtonText("Retrigger");
    retriggerButton.onClick = [this] { retriggerButtonClicked(); };
    
    
    addAndMakeVisible(dampingSlider);
    dampingSlider.setRange(0.0f, 1.0f, .001f);
    dampingSlider.setSkewFactorFromMidPoint(.1f);
    dampingSlider.setTextValueSuffix(" ");
    dampingSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    dampingSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxAbove,false,60,25);
    dampingSlider.setValue(.001);
    dampingSlider.addListener(this);
    addAndMakeVisible(dampingLabel);
    dampingLabel.setText("Damping", juce::dontSendNotification);
    dampingLabel.attachToComponent(&dampingSlider, false);
    
    
    addAndMakeVisible(connectionSlider);
    connectionSlider.setRange(1.0f, 100.0f, .1f);
    connectionSlider.setTextValueSuffix("");
    connectionSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    connectionSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxAbove,false,60,25);
    connectionSlider.setValue(1.0f);
    connectionSlider.addListener(this);
    addAndMakeVisible(connectionLabel);
    connectionLabel.setText("Edge", juce::dontSendNotification);
    connectionLabel.attachToComponent(&connectionSlider, false);
    
    
    addAndMakeVisible(portamentoSlider);
    portamentoSlider.setRange(0.0f, 1.0f, .001f);
    portamentoSlider.setTextValueSuffix("");
    portamentoSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    portamentoSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxAbove,false,60,25);
    portamentoSlider.setValue(0.001f);
    portamentoSlider.addListener(this);
    addAndMakeVisible(portamentoLabel);
    portamentoLabel.setText("Glide", juce::dontSendNotification);
    portamentoLabel.attachToComponent(&portamentoSlider, false);
    
    terrain_window.reset(new WaveTerrainWindow(&(plugin.scanner)));
    addAndMakeVisible(terrain_window.get());
    
    //Make sure you set the size of the component after
    //you add any child components
    setSize(800, 600);

    LOGFILE::log_value(5.9f);
}

MainComponent::~MainComponent(){
    delete scanner_window;
}

void MainComponent::openWaveformButtonClicked(){
  unsigned folderChooserFlags = juce::FileBrowserComponent::openMode;
  fileChooser->launchAsync(folderChooserFlags,
                           [this] (const juce::FileChooser& chooser){
                               juce::File wavFile(chooser.getResult());
                               if(wavFile.getFullPathName().length() == 0){
                                   return;
                               }
                               plugin.scanner.fillWithWaveform(wavFile.getFullPathName(), plugin.scanner.hammer_table, plugin.scanner.num_nodes);
                           });
}

void MainComponent::retriggerButtonClicked(){
    plugin.scanner.retrigger ^= 1;
}


void MainComponent::paint(juce::Graphics& g) {
    
}

void MainComponent::resized(){
  int scanner_window_height = 200;
  int button_width = 80;
  int slider_width = 50;
  int slider_height = 100;
  int slider_y_pos = 50 + scanner_window_height;
  
  scanner_window->setBounds(0,0,getWidth(), scanner_window_height);
  
  //hammerTableSlider.setBounds(0,             slider_y_pos, slider_width, slider_height);
  openWaveformButton.setBounds(0,            slider_y_pos, button_width, slider_height/2);
  retriggerButton.setBounds(0,               slider_y_pos + (slider_height/2), button_width, slider_height/2);
    
  dampingSlider.setBounds(button_width,      slider_y_pos, slider_width, slider_height);
  connectionSlider.setBounds(button_width + slider_width, slider_y_pos, slider_width, slider_height);
  portamentoSlider.setBounds(button_width + slider_width*2, slider_y_pos, slider_width, slider_height);

  // int keyboard_y_pos = slider_y_pos + slider_height + 50;
  // int keyboard_height = 200;
  // int keyboard_width = 200;
  
  terrain_window->setBounds(300,200, 500,400);
}

void MainComponent::sliderValueChanged(juce::Slider* slider) {
    if(slider == &dampingSlider){
        plugin.scanner.damping_gain = dampingSlider.getValue();
    }
    else if(slider == &connectionSlider){
        plugin.scanner.connection_gain = connectionSlider.getValue();
    }
    else if(slider == &portamentoSlider){
        plugin.scanner.portamento_tc = portamentoSlider.getValue();
    }
    
}
