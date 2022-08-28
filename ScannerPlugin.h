
#pragma once

#include <JuceHeader.h>
#include "MainComponent.h"
#include "scanner.h"
#include "log_value.h"
#include <iostream>

//forward declare this class name to prevent circular includes

class ScannerPlugin : public juce::AudioProcessor
{
public:
    ScannerPlugin() : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
    {
        LOGFILE::log_value(1.0f);
        addParameter(damping = new juce::AudioParameterFloat("damping", "scanner_damping",0,1.0,.5));
        addParameter(stiffness = new juce::AudioParameterFloat("stiffness", "scanner_stiffness",0,1.0,.5));
        addParameter(glide = new juce::AudioParameterFloat("glide", "scanner_glide",0,1.0,.5));
        
        for (int i = 0; i < 1; ++i) {
            synth.addVoice(new ScannerVoice(scanner));
        }
        synth.clearSounds();
        synth.addSound(new ScannerSound());
        synth.setNoteStealingEnabled(true);
        LOGFILE::log_value(1.9f);
    }
    
    ~ScannerPlugin()
    {
        LOGFILE::log_value(19.0f);
        //delete damping; //these get autmatically deleted. This causes a segfault if you 
        //delete stiffness;
        //delete glide;
        LOGFILE::log_value(19.9f);
        LOGFILE::close_log();
    }

    void getStateInformation (juce::MemoryBlock& destData) override
    {
        LOGFILE::log_value(2.0f);
        juce::MemoryOutputStream stream(destData, true);
        
        stream.writeFloat(*damping);
        stream.writeFloat(*stiffness);
        stream.writeFloat(*glide);
        stream.writeString(wave_filename);
        
        LOGFILE::log_value(2.9f);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        LOGFILE::log_value(3.0f);
        juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
        
        damping->setValueNotifyingHost(stream.readFloat());
        stiffness->setValueNotifyingHost(stream.readFloat());
        glide->setValueNotifyingHost(stream.readFloat());

        wave_filename = stream.readString();
        scanner.fillWithWaveform(wave_filename, scanner.hammer_table, scanner.num_nodes);
        LOGFILE::log_value(3.9f);
    }
    
    
    void prepareToPlay(double sampleRate, int samplesPerBlockExpected) override
    {
        LOGFILE::log_value(4.0f);
        synth.setCurrentPlaybackSampleRate(sampleRate);
        scanner.setBlockSize(samplesPerBlockExpected);
        scanner.setSampleRate((float)sampleRate);
        LOGFILE::log_value(4.9f);
    }
    
    void processBlock (juce::AudioBuffer<float>& audio_buffer, juce::MidiBuffer& midi_buffer) override
    {
        LOGFILE::log_value(6.0f);
        //audio_buffer.clear();
        juce::AudioBuffer<float> stereo_buffer = getBusBuffer(audio_buffer, false, 0);
        synth.renderNextBlock(stereo_buffer, midi_buffer, 0, stereo_buffer.getNumSamples());
        stereo_buffer.applyGain(1.0f);
        LOGFILE::log_value(6.9f);
    }
    
    juce::AudioProcessorEditor* createEditor() override
    {
        LOGFILE::log_value(7.0f);
        //return new juce::GenericAudioProcessorEditor (*this);
        return new MainComponent(*this);
    }
    bool hasEditor() const override
    {
        LOGFILE::log_value(13.0f);
        return true;
    }
    const juce::String getName() const override                    {
        LOGFILE::log_value(8.0f);
        return "Scanner Synth";
    }
    bool acceptsMidi() const override                              {LOGFILE::log_value(18.0f);return true; }
    bool producesMidi() const override                             {LOGFILE::log_value(17.0f);return false; }
    
    double getTailLengthSeconds() const override
    {
        LOGFILE::log_value(12.0f);
        return 0.0;
    }
    
    int getNumPrograms() override                                  {LOGFILE::log_value(16.0f);return 1; }
    int getCurrentProgram() override                               {LOGFILE::log_value(15.0f);return 0; }
    void setCurrentProgram (int) override                          {LOGFILE::log_value(14.0f);}
    
    const juce::String getProgramName (int) override               {
        LOGFILE::log_value(9.0f);
        return {"Scanner"};
    }
    void changeProgramName (int, const juce::String&) override
    {
        LOGFILE::log_value(11.0f);
    }
    
    void releaseResources() override
    {
        LOGFILE::log_value(10.0f);
    }
    
    bool isMidiEffect() const override
    {
        LOGFILE::log_value(19.0f);
        return false;
    }

    void reset() override
    {
        LOGFILE::log_value(20.0f);
    }
    
    Scanner scanner;
private:
    juce::Synthesiser synth;
    
    juce::MidiMessageCollector midiCollector;    
    
    juce::AudioParameterFloat* damping;
    juce::AudioParameterFloat* stiffness;
    juce::AudioParameterFloat* glide;
    
    juce::String wave_filename;
    
    //JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScannerPlugin)
};
