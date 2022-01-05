#include "MainComponent.h"
#include <stdio.h>




MainComponent::MainComponent()
    :  keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    //the midi gui bullshit (mostly copy pasted from the tutorial)
    addAndMakeVisible(keyboardComponent);    
    addAndMakeVisible(midiInputListLabel);
    midiInputListLabel.setText ("MIDI Input:", juce::dontSendNotification);
    midiInputListLabel.attachToComponent (&midiInputList, true);
 
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    addAndMakeVisible (midiInputList);
    midiInputList.setTextWhenNoChoicesAvailable ("No MIDI Inputs Enabled");
    
    juce::StringArray midiInputNames;
    for(auto input : midiInputs)
        midiInputNames.add(input.name);
 
    midiInputList.addItemList(midiInputNames, 1);
    midiInputList.onChange = [this] { setMidiInput (midiInputList.getSelectedItemIndex()); };
    
    for(auto input : midiInputs) {
        if(deviceManager.isMidiInputDeviceEnabled(input.identifier)) {
            setMidiInput(midiInputs.indexOf(input));
            break;
        }
    }
    
    if(midiInputList.getSelectedId() == 0){
        setMidiInput (0);
    }

    //Custom scanner related stuff
    scanner_window = new ScannerWindow(&scanner);
    for (int i = 0; i < 1; ++i) {
        synth.addVoice(new ScannerVoice(scanner));
    }
    synth.clearSounds();
    synth.addSound(new ScannerSound());
    synth.setNoteStealingEnabled(true);
    
    //scanner.startTimerHz(1000);
    
    addAndMakeVisible(scanner_window);
    
    //various sliders
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
    
    
    addAndMakeVisible(dampingSlider);
    dampingSlider.setRange(.001, 10.0f, .1f);
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
    
    addAndMakeVisible(restoringSlider);
    restoringSlider.setRange(0, 10.0f, .1f);
    restoringSlider.setTextValueSuffix("");
    restoringSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    restoringSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxAbove,false,60,25);
    restoringSlider.setValue(0.0);
    restoringSlider.addListener(this);
    addAndMakeVisible(restoringLabel);
    restoringLabel.setText("Center", juce::dontSendNotification);
    restoringLabel.attachToComponent(&restoringSlider, false);
    
    
    
    // Make sure you set the size of the component after
    // you add any child components
    setSize (800, 600);
    setAudioChannels (0, 2);

    startTimer(400); //this callback fires fter 400ms and only runs once.
}

MainComponent::~MainComponent(){
    shutdownAudio();
    scanner.stopTimer();
    
    delete scanner_window;
    //delete scanner;
}

void MainComponent::setMidiInput(int index) {
    auto list = juce::MidiInput::getAvailableDevices();
    
    deviceManager.removeMidiInputDeviceCallback(list[lastInputIndex].identifier, &midiCollector); // [12]
    
    auto newInput = list[index];
    
    if(!deviceManager.isMidiInputDeviceEnabled(newInput.identifier))
        deviceManager.setMidiInputDeviceEnabled(newInput.identifier, true);
    
    deviceManager.addMidiInputDeviceCallback(newInput.identifier, &midiCollector); // [13]
    midiInputList.setSelectedId(index + 1, juce::dontSendNotification);
    
    lastInputIndex = index;
}


void MainComponent::timerCallback(){
    keyboardComponent.grabKeyboardFocus();
    stopTimer();
}

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate) {
    midiCollector.reset(sampleRate);
    synth.setCurrentPlaybackSampleRate(sampleRate);
    scanner.setBlockSize(samplesPerBlockExpected);
    scanner.setSampleRate((float)sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) {
  bufferToFill.clearActiveBufferRegion();

  juce::MidiBuffer incomingMidi;
  midiCollector.removeNextBlockOfMessages (incomingMidi, bufferToFill.numSamples);
  keyboardState.processNextMidiBuffer (incomingMidi, 0, bufferToFill.numSamples, true);
  
  synth.renderNextBlock(*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
  bufferToFill.buffer->applyGain(1.0f);
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
  dampingSlider.setBounds(slider_width,     slider_y_pos, slider_width,slider_height);
  connectionSlider.setBounds(slider_width*2,slider_y_pos, slider_width,slider_height);
  restoringSlider.setBounds(slider_width*3, slider_y_pos, slider_width,slider_height);
  
  int keyboard_y_pos = slider_y_pos + slider_height + 100;
  int keyboard_height = 200;
  int keyboard_width = 200;
  //midiInputList.setBounds(200, 10, keyboard_width - 10, 20);
  keyboardComponent.setBounds(10,  keyboard_y_pos, keyboard_width, keyboard_height);  
}

void MainComponent::sliderValueChanged(juce::Slider* slider) {
  if(slider == &hammerTableSlider){
    scanner.fillWithWaveform((int)hammerTableSlider.getValue(), scanner.hammer_table, scanner.num_nodes);
  }
  else if(slider == &dampingSlider){
    scanner.damping_gain = dampingSlider.getValue();
  }
  else if(slider == &connectionSlider){
    scanner.connection_gain = connectionSlider.getValue();
  }
  else if(slider == &restoringSlider){
    scanner.restoring_gain = restoringSlider.getValue();
  }
}
