#!/usr/bin/env bash
set -e
export PATH="$PATH:/c/Program Files/CMake/bin"
cd "$(dirname "$0")"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --target VoicemodIshApp --config Release
echo "Built: build/app/VoicemodIshApp_artefacts/Release/Voicemod-ish.exe"
