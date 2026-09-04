# Virtual microphone output (v2): route processed audio to a third-party virtual audio cable, not a custom driver

Supersedes [ADR 0001](0001-monitoring-only-scope.md). The processed signal must now be selectable as a microphone in other apps (Discord, Zoom, games), matching Voicemod's core "Virtual Audio Device" use case.

Writing and signing our own WDM/kernel-mode audio driver is out of scope (Windows driver signing, WDK toolchain, ongoing driver maintenance is a different discipline from an audio app). Instead, the app outputs its processed signal to an already-installed third-party virtual audio cable (e.g. VB-CABLE), by simply treating that cable's playback endpoint as a selectable output device in the existing device selector. Other apps then select the cable's matching recording endpoint as their microphone. This trades "one-click install" for "user installs a small free driver once," in exchange for avoiding building/maintaining our own kernel driver.
