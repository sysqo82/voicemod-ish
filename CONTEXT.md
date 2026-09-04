# Voicemod-ish

A Windows desktop app that applies real-time voice effects to a microphone signal for monitoring (headphones/speakers), not for injecting into other apps.

## Language

**Effect**:
A single DSP process applied to the signal (Volume, EQ, Pitch, Echo, Distortion, Reverb). Each is independently enabled/disabled and parameterized.
_Avoid_: Filter, module

**Effect Chain**:
The fixed-order sequence of Effects the signal passes through: Volume → EQ → Pitch → Echo → Distortion → Reverb. Order is not user-configurable; only which Effects are enabled and their parameters are.
_Avoid_: Pipeline, effect rack

**Current Settings**:
The single persisted state of all Effect parameters and enabled flags, restored on app launch. Not a named/saveable Preset (out of scope for v1).
_Avoid_: Preset, profile

**Bypass**:
A single global toggle that routes mic input straight to output, skipping the entire Effect Chain, for A/B comparison. Distinct from disabling Effects individually.
_Avoid_: Mute, disable all
