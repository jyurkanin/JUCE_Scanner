#include "MainComponent.h"


MainComponent::MainComponent(){
    scanner = new Scanner();
    scanner_window = new ScannerWindow(scanner);
    addAndMakeVisible(scanner_window);

    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);
    setAudioChannels (0, 2);
}

MainComponent::~MainComponent(){
    shutdownAudio();

    scanner->shutdown();
    
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
}

void MainComponent::releaseResources() {
  
}


void MainComponent::paint (juce::Graphics& g) {
  
}

void MainComponent::resized(){
  printf("width height   %d %d\n", getWidth(), getHeight());
  scanner_window->setBounds(0,0,getWidth(),getHeight());
}
