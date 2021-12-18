#include "MainComponent.h"


MainComponent::MainComponent(){
    scanner = new Scanner();
    scanner_window = new ScannerWindow(scanner);
    
    scanner->startTimerHz(1000);
    
    addAndMakeVisible(scanner_window);

    addAndMakeVisible(hammerTableSlider);
    hammerTableSlider.setRange(0, 101.0f, 1.0f);
    hammerTableSlider.setTextValueSuffix(" ");
    hammerTableSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    hammerTableSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxAbove,false,60,25);
    hammerTableSlider.setValue(0.0);
    hammerTableSlider.addListener(this);    
    addAndMakeVisible(hammerTableLabel);
    hammerTableLabel.setText("Hammer", juce::dontSendNotification);
    hammerTableLabel.attachToComponent(&hammerTableSlider, false);
    
    
    addAndMakeVisible(dampingTableSlider);
    dampingTableSlider.setRange(0, 101.0f, 1.0f);
    dampingTableSlider.setTextValueSuffix(" ");
    dampingTableSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    dampingTableSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxAbove,false,60,25);
    dampingTableSlider.setValue(0.0);
    dampingTableSlider.addListener(this);
    addAndMakeVisible(dampingTableLabel);
    dampingTableLabel.setText("Damping", juce::dontSendNotification);
    dampingTableLabel.attachToComponent(&dampingTableSlider, false);


    addAndMakeVisible(connectionTableSlider);
    connectionTableSlider.setRange(0, 101.0f, 1.0f);
    connectionTableSlider.setTextValueSuffix("");
    connectionTableSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    connectionTableSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxAbove,false,60,25);
    connectionTableSlider.setValue(0.0);
    connectionTableSlider.addListener(this);
    addAndMakeVisible(connectionTableLabel);
    connectionTableLabel.setText("Edge", juce::dontSendNotification);
    connectionTableLabel.attachToComponent(&connectionTableSlider, false);
        
    addAndMakeVisible(restoringTableSlider);
    restoringTableSlider.setRange(0, 101.0f, 1.0f);
    restoringTableSlider.setTextValueSuffix("");
    restoringTableSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    restoringTableSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxAbove,false,60,25);
    restoringTableSlider.setValue(0.0);
    restoringTableSlider.addListener(this);
    addAndMakeVisible(restoringTableLabel);
    restoringTableLabel.setText("Center", juce::dontSendNotification);
    restoringTableLabel.attachToComponent(&restoringTableSlider, false);
    
    
    
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);
    setAudioChannels (0, 2);
}

MainComponent::~MainComponent(){
    shutdownAudio();
    scanner->stopTimer();
    
    delete scanner_window;
    delete scanner;
}


void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate) {
  // This function will be called when the audio device is started, or when
  // its settings (i.e. sample rate, block size, etc) are changed.
  // You can use this function to initialise any resources you might need,
  // but be careful - it will be called on the audio thread, not the GUI thread.
  // For more details, see the help for AudioProcessor::prepareToPlay()
  scanner->setBlockSize(samplesPerBlockExpected);
  scanner->setSampleRate((float)sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) {
  bufferToFill.clearActiveBufferRegion();
  scanner->getSampleBlock(bufferToFill.buffer->getArrayOfWritePointers(), bufferToFill.numSamples);
  //bufferToFill.buffer->applyGain(1.0f);
}

void MainComponent::releaseResources() {
  
}


void MainComponent::paint (juce::Graphics& g) {
  
}

void MainComponent::resized(){
  int scanner_window_height = 200;
  int slider_width = 60;
  int slider_height = 100;
  int slider_y_pos = 50 + scanner_window_height;
  
  scanner_window->setBounds(0,0,getWidth(), scanner_window_height);
  
  hammerTableSlider.setBounds(0,                 slider_y_pos, slider_width,slider_height);
  dampingTableSlider.setBounds(slider_width,     slider_y_pos, slider_width,slider_height);
  connectionTableSlider.setBounds(slider_width*2,slider_y_pos, slider_width,slider_height);
  restoringTableSlider.setBounds(slider_width*3, slider_y_pos, slider_width,slider_height);

  
}

void MainComponent::sliderValueChanged(juce::Slider* slider) {
  if(slider == &hammerTableSlider){
    scanner->fillWithWaveform((int)hammerTableSlider.getValue(), scanner->hammer_table, scanner->num_nodes);
  }
  else if(slider == &dampingTableSlider){
    scanner->fillWithWaveform((int)dampingTableSlider.getValue(), scanner->node_damping, scanner->num_nodes);
    for(int i = 0; i < scanner->num_nodes; i++){
      scanner->node_damping[i] = std::max(scanner->node_damping[i], 0.0f);
    }
  }
  else if(slider == &connectionTableSlider){
    scanner->fillWithWaveform((int)connectionTableSlider.getValue(), scanner->connection_k, scanner->num_nodes);
    for(int i = 0; i < scanner->num_nodes; i++){
      scanner->connection_k[i] = std::max(scanner->connection_k[i], 0.0f);
    }
  }
  else if(slider == &restoringTableSlider){
    scanner->fillWithWaveform((int)restoringTableSlider.getValue(), scanner->restoring_k, scanner->num_nodes);
    for(int i = 0; i < scanner->num_nodes; i++){
      scanner->restoring_k[i] = std::max(scanner->restoring_k[i], 0.0f);
    }
  }  
}
