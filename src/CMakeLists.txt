add_library(EffectsEngine STATIC
    Effects/Effect.h
    Effects/VolumeEffect.h
    Effects/VolumeEffect.cpp
    Effects/EffectChain.h
    Effects/EffectChain.cpp
)

target_include_directories(EffectsEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(EffectsEngine PUBLIC
    juce::juce_audio_basics
    juce::juce_dsp
)

target_compile_definitions(EffectsEngine PUBLIC
    JUCE_STANDALONE_APPLICATION=1
)
