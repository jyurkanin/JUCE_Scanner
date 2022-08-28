/*
  ==============================================================================

    This file was auto-generated and contains the startup code for a PIP.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ScannerPlugin.h"
#include <iostream>
#include "log_value.h"

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    LOGFILE::open_log();
    LOGFILE::log_value(0.0f);
    return new ScannerPlugin();
}
