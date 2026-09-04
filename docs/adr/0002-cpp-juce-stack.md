# C++ with JUCE for the audio engine and app

C++ with the JUCE framework is used for the whole app: real-time audio I/O (WASAPI), the DSP effect chain, and the GUI. JUCE was chosen over a C#/.NET or Rust stack because it's the de facto standard for real-time audio effects work, ships with low-latency audio device handling and DSP building blocks (EQ, delay, reverb), and avoids the GC-pause / managed-runtime risk in the real-time audio callback that a C#/.NET stack would carry. This is a significant lock-in: swapping frameworks later means rewriting the audio engine and GUI.
