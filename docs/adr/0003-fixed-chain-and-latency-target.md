# Fixed effect chain order, <20ms round-trip latency target

The Effect Chain order (Volume → EQ → Pitch → Echo → Distortion → Reverb) is fixed and not user-reorderable; each Effect is independently enabled/disabled and combinable with the others. Round-trip latency (mic in to monitored output) targets under ~20ms, which requires WASAPI exclusive-mode (or similarly small buffer sizes) rather than shared-mode audio, since shared mode's larger buffers would push latency high enough to be audibly distracting to the user hearing their own delayed voice.
