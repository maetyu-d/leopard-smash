# Leopard Smash!

Dedicated to my cat, Leonard, and his preferred approach to showing disdain, Leopard Smash! is a "chaotic" compressor that treats compression as nonlinear, unstable dynamics sculpting/rhythmic destruction. There's an optional unsubtle noise gate too, because you'll probably need it.

## Implemented concept blocks

- Multi-domain detector (RMS, peak, transient derivative, spectral flux, crest factor, stereo phase correlation)
- Control mutation engine (quantization, noise injection, logistic-map chaos, hysteresis memory)
- Nonlinear dynamics core (envelope-driven wavefolding, asymmetry, bias shift, sample-rate collapse style hold/aliasing)
- Temporal distortion (micro-delay warp + micro-repeat taps)
- Spectral collapse approximation (high-band quantization + harmonic coercion)
- True per-band FFT spectral collapse (1024-point frame processing with per-band quantization, phase rotation, and harmonic coercion)
- Spectral Collapse Mix (latency-aligned dry/wet blend for FFT path)
- Nonlinear feedback corruption loop with containment
- Five modes: `Fracture`, `Implode`, `Shrapnel`, `Riot Bus`, `Sentient`
- Envelope switches: `Freeze Envelope`, `Reverse Release`
- Oversampling quality modes: `Off`, `2x`, `4x`
- Selectable collapse behavior: `Hold`, `Rate Drop`, `Hybrid`
- Host latency reporting/compensation for FFT frame path (latency scales with oversampling mode)
- 12 factory presets with extreme voicing spread (clean control through near-self-destruct)
- Optional extreme noise gate (`Noise Gate`, `Gate Threshold`, `Gate Release`)

## Parameters

Core:
- Threshold
- Ratio (supports negative ratios)
- Mix
- Attack (`-5 ms` pseudo pre-trigger to `200 ms`)
- Release (`1 ms` to `5 s`)

Radical/Mutation:
- Chaos
- Fold Depth
- Detector Resolution
- Aliasing Amount
- Spectral Corruption
- Spectral Collapse Mix
- Temporal Warp
- FFT Bands
- Feedback Instability
- Stereo Divergence
- Memory
- Hysteresis
- Random Seed Stability
- Containment
- Oscillation Bias
- Freeze Envelope
- Reverse Release
- Oversampling Quality
- Collapse Behavior
- Mode selector

## Build

Prerequisites:
- CMake 3.22+
- Xcode Command Line Tools
- JUCE checkout path

Configure + build:

```bash
cmake -S . -B build -DJUCE_DIR=/Users/md/JUCE
cmake --build build -j 8
```

Artifacts:
- `/Users/md/Downloads/Smash compressor/build/LeopardSmash_artefacts/AU/Leopard Smash!.component`
- `/Users/md/Downloads/Smash compressor/build/LeopardSmash_artefacts/VST3/Leopard Smash!.vst3`

## Notes

This is intentionally aggressive and unstable at extreme settings, especially in `Implode` and `Sentient`.

Factory preset names:
- Concrete Control
- Subquake Imploder
- Cymbal Shredder
- Snare Riot Bus
- Fracture Steps
- Sentient Drift
- Reverse Vacuum
- Glitch Mono Crusher
- Phase Predator
- Dry Punch / Wet Wreck
- Pump Tunnel
- Zero Containment
