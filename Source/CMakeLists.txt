cmake_minimum_required(VERSION 3.22)
project(SC55Editor VERSION 0.1.4)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# IS_SYNTH TRUE : le plugin apparait sur une piste instrument dans la DAW,
# ce qui facilite le routage de sa sortie MIDI vers d'autres pistes.
juce_add_plugin(SC55Editor
    COMPANY_NAME "Ricky_Fr"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    PLUGIN_MANUFACTURER_CODE Rick
    PLUGIN_CODE Sc55
    FORMATS VST3 Standalone
    PRODUCT_NAME "SC55 Editor")

target_sources(SC55Editor PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/SysExQueue.cpp
    Source/SysExQueue.h
    Source/SysExParamSlider.h
    Source/PartParams.h
    Source/InstrumentData.h
    Source/RoundLedButton.h
    Source/SC55Config.h)

target_compile_definitions(SC55Editor PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(SC55Editor
    PRIVATE juce::juce_audio_utils
    PUBLIC  juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)
