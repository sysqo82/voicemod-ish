# Virtual microphone output (v2): route processed audio to a third-party virtual audio cable, not a custom driver

Supersedes [ADR 0001](0001-monitoring-only-scope.md). The processed signal must now be selectable as a microphone in other apps (Discord, Zoom, games), matching Voicemod's core "Virtual Audio Device" use case.

Writing and signing our own WDM/kernel-mode audio driver is out of scope (Windows driver signing, WDK toolchain, ongoing driver maintenance is a different discipline from an audio app). Instead, the app outputs its processed signal to an already-installed third-party virtual audio cable (e.g. VB-CABLE), by simply treating that cable's playback endpoint as a selectable output device in the existing device selector. Other apps then select the cable's matching recording endpoint as their microphone. This trades "one-click install" for "user installs a small free driver once," in exchange for avoiding building/maintaining our own kernel driver.

## Details

- **Installing the virtual cable is a documented manual prerequisite**, not bundled/auto-installed by this app. Avoids third-party installer licensing/distribution complexity.
- **Dual output**: the processed signal is sent to the real monitoring device (speakers/headphones) and the virtual cable simultaneously, so the user can still hear themselves while the virtual mic feeds another app.
- **UI**: a second output-device dropdown, "Virtual Mic Output", defaulting to "None", alongside the existing input/output device dropdowns. Selecting a device opens it as a second, independent, output-only `AudioIODevice`, fed a copy of the same processed mono buffer via a lock-free ring buffer (the two devices run independent audio callbacks/threads).
