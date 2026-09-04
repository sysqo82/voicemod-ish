# Monitoring-only scope for v1; no virtual microphone

**Status: superseded by [ADR 0004](0004-virtual-microphone-output.md).**

The app processes mic input and outputs to the user's speakers/headphones for monitoring only. It does not install a virtual audio driver to expose the processed signal as a microphone to other apps (Discord, games, Zoom), unlike Voicemod's core use case. A virtual-mic driver requires a signed kernel-mode/WDM audio driver and is a much larger scope; it's deferred to a possible v2 rather than blocking v1 on it.
