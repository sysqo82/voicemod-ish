juce_add_gui_app(VoicemodIshApp
    PRODUCT_NAME "Voicemod-ish"
    VERSION 0.1.0
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png"
)

target_sources(VoicemodIshApp PRIVATE
    Main.cpp
    MainComponent.h
    MainComponent.cpp
)

target_compile_definitions(VoicemodIshApp PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:VoicemodIshApp,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:VoicemodIshApp,JUCE_VERSION>"
)

target_link_libraries(VoicemodIshApp PRIVATE
    EffectsEngine
    juce::juce_audio_utils
    juce::juce_audio_devices
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
)
