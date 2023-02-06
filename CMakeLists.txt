cmake_minimum_required(VERSION 3.15)
project(SUPER_SCANNER_PLUGIN VERSION 0.0.1)


set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall -O3 -DNDEBUG -m64")
set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
FetchContent_Declare(
  Eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG master
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE)
set(EIGEN_BUILD_DOC OFF)
set(BUILD_TESTING OFF)
set(EIGEN_BUILD_PKGCONFIG OFF)
set( OFF)
FetchContent_MakeAvailable(Eigen)

FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 7.0.5
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(SuperScanner
    COMPANY_NAME Justin Yurkanin
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Juce
    PLUGIN_CODE Cnt0
    FORMATS VST3 Standalone
    PRODUCT_NAME "Super Scanner")        # The name of the final executable, which can differ from the target name

juce_generate_juce_header(SuperScanner)

target_sources(SuperScanner PRIVATE
               scanner.cpp
	       log_value.cpp
	       MainComponent.cpp
	       Main.cpp
	       ScannerVoice.cpp
	       scanner_window.cpp
	       WaveTerrainWindow.cpp
	       wavfile.cpp
)



target_compile_definitions(SuperScanner
    PUBLIC
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(SuperScanner PRIVATE
		      juce::juce_audio_basics
		      juce::juce_audio_devices
		      juce::juce_audio_formats
		      juce::juce_audio_plugin_client
		      juce::juce_audio_processors
    		      juce::juce_audio_utils
		      juce::juce_core
		      juce::juce_data_structures
		      juce::juce_dsp
		      juce::juce_events
		      juce::juce_graphics
		      juce::juce_gui_basics
		      juce::juce_gui_extra
		      juce::juce_opengl
		      juce::juce_osc
		      juce::juce_video
		      Eigen3::Eigen
		     )
