cmake_minimum_required(VERSION 3.22)
project(SC55Editor VERSION 1.16)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# IS_SYNTH FALSE & IS_MIDI_EFFECT TRUE : le plugin se comporte comme un effet MIDI.
juce_add_plugin(SC55Editor
    COMPANY_NAME "Ricky_Fr"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    PLUGIN_MANUFACTURER_CODE Rick
    PLUGIN_CODE Sc55
    FORMATS VST3 Standalone
    ICON_BIG   "${CMAKE_CURRENT_SOURCE_DIR}/icon_big.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/icon_small.png"
    PRODUCT_NAME "SC55 Editor")

target_sources(SC55Editor PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
    SysExQueue.cpp
    SysExQueue.h
    MixState.h
    SysExParamSlider.h
    PartParams.h
    InstrumentData.h
    RoundLedButton.h
    FloppyButton.h
    HeaderBar.h
    PartListView.cpp
    PartListView.h
    AdvancedView.cpp
    AdvancedView.h
    SystemView.cpp
    SystemView.h
    ParamPanel.cpp
    ParamPanel.h
    PresetManager.cpp
    PresetManager.h
    SC55LookAndFeel.h
    SC55Config.h)

target_compile_definitions(SC55Editor PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1)

target_link_libraries(SC55Editor
    PRIVATE juce::juce_audio_utils
            juce::juce_audio_devices
    PUBLIC  juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

# Application autonome personnalisee (menu Options avec "About") : cible Standalone uniquement
target_sources(SC55Editor_Standalone PRIVATE StandaloneApp.cpp)