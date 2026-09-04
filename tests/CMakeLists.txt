add_executable(EffectsEngineTests
    EffectChainTests.cpp
    VolumeEffectTests.cpp
    EQEffectTests.cpp
    TestRunner.cpp
)

target_link_libraries(EffectsEngineTests PRIVATE
    EffectsEngine
    juce::juce_audio_basics
)

target_compile_definitions(EffectsEngineTests PRIVATE
    JUCE_STANDALONE_APPLICATION=1
)

add_test(NAME EffectsEngineTests COMMAND EffectsEngineTests)
