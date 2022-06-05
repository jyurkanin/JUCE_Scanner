#include "MainComponent.h"
#include <stdio.h>




MainComponent::MainComponent()
    :  keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    
    const juce::OwnedArray<juce::AudioIODeviceType>& io_devices = deviceManager.getAvailableDeviceTypes();
    juce::String dn("JACK");
    deviceManager.setCurrentAudioDeviceType(dn, true);
    
    
    // for(int i = 0; i < io_devices.size(); i++){
    //   juce::String device_name = io_devices[i]->getTypeName();
    //   printf("Device %s\n", device_name.toRawUTF8());
    // }
    printf("Connected to audio driver: %s\n", deviceManager.getCurrentAudioDeviceType().toRawUTF8());
    
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

    
    // printf("ODE speed test\n");
    // long start_time = juce::Time::currentTimeMillis();
    // for(int i = 0; i < 1000000; i++){
    //     scanner.timerCallbackSymEuler();
    // }
    // long stop_time = juce::Time::currentTimeMillis();
    // printf("1,000,000 EulerSym Duration %ld\n", stop_time - start_time);
    
    
    
    //Custom scanner related stuff
    scanner_window = new ScannerWindow(&scanner);
    for (int i = 0; i < 1; ++i) {
        synth.addVoice(new ScannerVoice(scanner));
    }
    synth.clearSounds();
    synth.addSound(new ScannerSound());
    synth.setNoteStealingEnabled(true);
    
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


    
    // Make sure you set the size of the component after
    // you add any child components
    setSize(800, 600);
    setAudioChannels(0, 2);

    startTimer(400); //this callback fires fter 400ms and only runs once.
}

MainComponent::~MainComponent(){
    shutdownAudio();
    scanner.stopTimer();
    
    delete scanner_window;
    //delete scanner;
}

void MainComponent::openWaveformButtonClicked(){
  unsigned folderChooserFlags = juce::FileBrowserComponent::openMode;
  fileChooser->launchAsync(folderChooserFlags,
                           [this] (const juce::FileChooser& chooser){
                               juce::File wavFile(chooser.getResult());
                               if(wavFile.getFullPathName().length() == 0){
                                   return;
                               }
                               scanner.fillWithWaveform(wavFile.getFullPathName(), scanner.hammer_table, scanner.num_nodes);
                           });
}

void MainComponent::retriggerButtonClicked(){
    scanner.retrigger ^= 1;
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
  midiCollector.removeNextBlockOfMessages(incomingMidi, bufferToFill.numSamples);
  keyboardState.processNextMidiBuffer(incomingMidi, 0, bufferToFill.numSamples, true);
  
  synth.renderNextBlock(*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
  bufferToFill.buffer->applyGain(1.0f);
}

void MainComponent::releaseResources() {
    
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
  
  int keyboard_y_pos = slider_y_pos + slider_height + 50;
  int keyboard_height = 200;
  int keyboard_width = 200;
  
  midiInputList.setBounds(10, keyboard_y_pos - 30, keyboard_width, 20);
  keyboardComponent.setBounds(10,  keyboard_y_pos, keyboard_width, keyboard_height);
  
  
}

void MainComponent::sliderValueChanged(juce::Slider* slider) {
    if(slider == &dampingSlider){
        scanner.damping_gain = dampingSlider.getValue();
    }
    else if(slider == &connectionSlider){
        scanner.connection_gain = connectionSlider.getValue();
    }
    else if(slider == &portamentoSlider){
        scanner.portamento_tc = portamentoSlider.getValue();
    }
    
}
