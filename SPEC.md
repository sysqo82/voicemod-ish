# Voicemod-ish v1

## Problem Statement

The user wants a Windows desktop app that takes microphone input, applies a chain of voice effects, and lets them monitor the result through headphones/speakers — the "voicemod-like" experience — without needing the processed audio to be injected into other apps as a virtual microphone.

## Solution

A JUCE-based (C++) Windows app that captures mic input via WASAPI, runs it through a fixed-order Effect Chain (Volume → EQ → Pitch → Echo → Distortion → Reverb), and outputs the processed signal to a user-selected playback device at low latency (<20ms round-trip). Settings persist between launches; effects are individually enabled/disabled and parameterized, with a global Bypass toggle for A/B comparison.

## User Stories

1. As a user, I want to select my microphone from a dropdown of available input devices, so that I use the right mic when I have more than one.
2. As a user, I want to select my playback device from a dropdown of available output devices, so that I hear the processed audio on the right speakers/headphones.
3. As a user, I want to see an input level meter, so that I can confirm my mic is being picked up.
4. As a user, I want a global Bypass toggle, so that I can instantly compare my raw voice to the processed voice.
5. As a user, I want a Volume effect with a gain slider (-∞ to +12dB) and mute toggle, so that I can control the loudness of my monitored voice.
6. As a user, I want a 3-band EQ (low shelf, mid peak, high shelf), so that I can shape the tone of my voice.
7. As a user, I want a Pitch effect that shifts my voice up or down, so that I can sound higher or lower (including unnatural "chipmunk/demon" extremes).
8. As a user, I want an Echo effect with delay-time, feedback, and mix controls, so that I can add a repeating echo to my voice.
9. As a user, I want a Distortion effect with a single Drive knob, so that I can add an overdriven/gritty character to my voice.
10. As a user, I want a Reverb effect with room-size/damping/wet-dry controls, so that I can add a sense of space to my voice.
11. As a user, I want to enable/disable each effect independently and combine any subset of them, so that I can mix and match effects freely.
12. As a user, I want each effect to have its own reset-to-default control, so that I can undo my tweaks to just that effect without losing settings on the others.
13. As a user, I want my last-used settings (device selection, effect params, enabled flags) restored automatically the next time I open the app, so that I don't have to reconfigure it every session.
14. As a user, I want the app to keep processing audio when I minimize the window, so that I can tuck it away while using it in a call/game.
15. As a user, I want closing the app window to fully stop it, so that I have a simple, predictable way to turn it off.
16. As a user, I want to run the app as a single portable .exe, so that I don't need to go through an installer to try it.
17. As a user, I want to enable/disable each effect independently with its own on/off control (in addition to per-effect parameters), so that I can quickly A/B an effect without losing its settings.
18. As a user, I want to select a virtual audio cable as a second "Virtual Mic Output", so that other apps (Discord, Zoom, games) can use my processed voice as their microphone, while I still hear myself on my real speakers/headphones.

## Implementation Decisions

- **Stack**: C++, JUCE framework, for audio I/O (WASAPI via `juce::AudioDeviceManager`), DSP, and GUI (`juce::Component`-based). See [ADR 0002](docs/adr/0002-cpp-juce-stack.md).
- **Audio engine**: A single `EffectChain` module owns an ordered, fixed sequence of `Effect` stages (Volume, EQ, Pitch, Echo, Distortion, Reverb). Each stage exposes an enabled flag and its own parameters, and processes a `juce::AudioBuffer<float>` in place. `EffectChain::processBlock(buffer)` is the core seam: it takes an input buffer and produces the processed output, independent of real audio hardware. A top-level `Bypass` flag on `EffectChain` skips all stages and passes the buffer through unmodified.
- **Signal path**: Mono in, mono through the chain, duplicated to L=R only at the final output stage (see [ADR 0003](docs/adr/0003-fixed-chain-and-latency-target.md)).
- **Sample rate**: No internal resampling; process at the audio device's reported native sample rate.
- **Latency**: Target <20ms round-trip; use WASAPI exclusive mode or otherwise minimal buffer sizes.
- **Effect DSP choices**:
  - Volume: gain (-∞ to +12dB, linear-to-dB mapped) + mute flag.
  - EQ: 3-band (low shelf, mid peak, high shelf), e.g. via `juce::dsp::IIR::Filter` chain.
  - Pitch: simple resampling-style pitch shift, no formant correction.
  - Echo: single-tap delay line with delay-time, feedback, mix parameters.
  - Distortion: single soft-clip/overdrive waveshaper with one Drive parameter.
  - Reverb: `juce::dsp::Reverb` (algorithmic), exposing its standard room-size/damping/wet-dry parameters.
- **Persistence**: `juce::PropertiesFile` stores the `Current Settings` (all effect params, enabled flags, selected input/output/virtual-mic device names) and restores them on launch.
- **Device selection**: In-app dropdowns for input and output device, backed by `juce::AudioDeviceManager`'s device list.
- **Virtual Microphone output (v2, see [ADR 0004](docs/adr/0004-virtual-microphone-output.md))**: a second "Virtual Mic Output" dropdown (default "None") lists output devices from the current device type; selecting one opens it as an independent, output-only `AudioIODevice` fed a copy of the processed mono buffer via a lock-free ring buffer, so the app monitors on the real device and feeds the virtual cable simultaneously. Requires the user to have a third-party virtual audio cable (e.g. VB-CABLE) already installed; not bundled.
- **GUI**: Single window, all effect sections visible simultaneously (no tabs), plus device dropdowns (input/output/virtual mic output), input level meter, global Bypass toggle, and a per-effect enable toggle and reset-to-default control.
- **Lifecycle**: Minimizing the window leaves audio processing running; closing the window terminates the app (no system tray/background mode).
- **Packaging**: Build output is a portable `.exe`; no installer for v1.

## Testing Decisions

- Focus tests on `EffectChain::processBlock` and each individual `Effect` stage, driven with synthetic input buffers (e.g. a sine wave or impulse), asserting on the output buffer's measurable properties (RMS level for gain/mute, frequency content for EQ, pitch/period for the pitch shifter, decay/tail presence for echo/reverb, waveshaping for distortion) — not on internal implementation details of each DSP algorithm.
- No prior art in this repo yet (greenfield); JUCE's `juce::UnitTestRunner` is the natural harness since it's already part of the framework being used.
- Real audio device I/O (`juce::AudioDeviceManager`, WASAPI) is not unit tested; it's a thin adapter over JUCE/OS APIs and is validated manually.
- GUI is not unit tested; validated manually.

## Out of Scope

- Named/saveable presets (only a single persisted "current settings" state).
- User-reorderable effect chain.
- Formant-preserving pitch shifting.
- Multiple distortion algorithms or convolution reverb.
- Multi-tap/ping-pong echo.
- Installer/Start Menu integration, code signing.
- System tray / background operation after window close.
- Global "reset all effects" button (only per-effect reset).

## Further Notes

Full interview history and rationale for the architectural decisions above are in [CONTEXT.md](CONTEXT.md) and `docs/adr/0001`–`0003`. No issue tracker is configured for this repo, so this spec lives as a local file rather than being published as a tracked issue.
