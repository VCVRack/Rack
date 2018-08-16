# Southpole VCV Rack Modules 0.6.0

![All](./doc/sp-main-0_6_0.png)


A personal collection of modules I've always wanted for my workflow.

- Some of the modules are simply reskins of the existing Audible Instruments VCV Rack versions of Mutable Instruments eurorack modules.
  - The skins are inspired by hardware micro versions of various MI modules.
  - Some skins are additonal modes of firmware with appropriate labels for knobs and ports to make usage easier.
  - Some skins do not exist yet in hardware.
- Some modules did not yet exist in VCV Rack versions.
- Some modules are new.

- [Mutable Instruments](https://mutable-instruments.net/)
- [Audible Instruments](https://github.com/VCVRack/AudibleInstruments/)

- Extra "Southpole Parasites" for parasites firmware based modules are on a separate branch


## Building

Compile Rack from source, following the instructions at https://github.com/VCVRack/Rack.

After checking out in the `plugins/` directory, get external dependencies with

	git submodule update --init --recursive

There is a build script

	./scripts/make_all.sh

WARNING: Overwrites existing Southpole directories from eg. a regular download, inspect script before use		

## Modules copied from Audible Instruments

### Cornrows X - Macro Oscillator

- Based on [Braids](https://mutable-instruments.net/modules/braids), [Manual](https://mutable-instruments.net/modules/braids/manual/)

- Extended as follows
  - As complete implementation of Braids / macro oscillator as possible
  - so far not available features in the Audible Instruments version have been activated and broken out as controls: AD-envelope, bit crusher, scale quantizer, tuning
  - Implemented as menu options: AUTO, VCA, FLAT, Paques
  
### Splash - Tidal Modulator / Lambs - Wavetable Oscillator

- Based on [Tides](https://mutable-instruments.net/modules/tides), [Manual](https://mutable-instruments.net/modules/tides/manual/)

- Based on [Sheep](https://mutable-instruments.net/modules/tides/firmware/) (Tides alternative firmware)

### Smoke - Texture Synthesizer
Based on [Clouds](https://mutable-instruments.net/modules/clouds), [Manual](https://mutable-instruments.net/modules/clouds/manual/)

- Smoke, extra skins for additional modes: Espectro, Ritardo, Camilla

### Annuli - Resonator
- Based on [Rings](https://mutable-instruments.net/modules/rings), [Manual](https://mutable-instruments.net/modules/rings/manual/)
- extra skin for Disastrous Peace mode

### Bandana - Quad VC-polarizer
- Based on [Blinds](https://mutable-instruments.net/modules/blinds), [Manual](https://mutable-instruments.net/modules/blinds/manual/)

### Balaclava - Quad VCA
- Based on [Veils](https://mutable-instruments.net/modules/veils), [Manual](https://mutable-instruments.net/modules/veils/manual/)

## New modules

### Aux - 2 stereo effect send/return loops 

  - with cross feedback (same or swapped stereo channels)
  - mute button for input (to listen to those sweet reverb tails)
  - bypass button for fx (in case you went wild with the feedback pots)  

### Abr - Manual A/B Switch 

- Inverse of But, either input A or B goes to output
- Sums of inputs set to A or B provided

### Blanks

- Nothing special right now, but watch this space
- 42HP useful if you want to recreate a 84HP row common in hardware

### But - Manual A/B Buss 

- Inspired by [DJ Steevio's modular method](https://www.youtube.com/watch?v=x6hJa2lRRgM)
- Either input A or B go to the output and the A/B busses
- Two summed identical outputs for A and B buss are provided.

### Deux Etagères

- convenience stereo version of Etagère

### Etagère - EQ Filter

- Inspired by [Shelves](https://mutable-instruments.net/modules/shelves), [Manual](https://mutable-instruments.net/modules/shelves/manual/)

- Does not fully reproduce all characteristics of Shelves!
- Biquad filters copied from RJModules Filters

### Falls - Attenumixer

- Attenumixverter, inspired by Fonitronik Cascade
- empty inputs provide offset voltages
- empty outputs mix into output below
- Range switch +/-1 or +/-10

### Ftagn: No Filter

  - transgressive module for your nihilistic no-input patches at the noise show
  - stereo - neither left nor right channel do anything at all

### Fuse - Synchronized one-shot triggers

- Intended to build a "queue next pattern" system like on groove boxes / drum machines
- Helps fire synchronized events in 4/4 based music if you can't count like me
- Can be used to launch triggers (or beat-long gates) ONCE at a given beat on a 16 step counter
- Arm triggers/gates manually or via inputs
- For example, clock with bars, trigger to mute the kick for exactly 4 bars, trigger 4 bar later to drop

### Piste - Drum Processor

- One-stop module to turn oscillators into drums / percussion, or process existing drums to taste
- Inspired by [Bastl Skis](http://www.bastl-instruments.com/modular/skis/)
- Input gain + eq + overdrive because sometimes basic processing is all that's needed
- 2 independent decay envelopes (eg. body and accent) the sum of which is applied to input signal via internal VCA
- 2nd envelope is scaled to 1st
- Triggers are scalable
- Mute input / trigger veto intended for choke groups (eg. open hihat mutes closed hihat)

### Pulse: Pulse Generator

  - main purpose is to convert triggers to gates
  - clock through sends 1ms triggers - clean your clock
  - range selectable (1s / 10s)
  - when clocked, duration/delay are quantized to "musically relevant" intervals

### Rakes: Stereo Resonator

  - inspired by the Ableton Resonator effect
  - 6 stereo comb filters
  - V/OCT inputs quantized to SEMI tones or FREE
  - FINE detuning works with opposite sign on L/R channels
  - small values produce Haas effect (stereo widening)
  - individual GAINs
  - best used together with a chord generator patched into the V/OCT inputs

### Riemann

- chord generator based on Neo-Riemannian Tonnetz analysis
- inspired by NE Tonnetz Sequent, Navichord app, o_C Automatonnetz (and my incapability to compose / improvise diatonic music)
- Traversal via perfect fifth (P5/V) per volt and major triad per volt (M3/V) inputs  
- Major/minor, Augmented/diminished chord groups
- 3 - 7 parts per chord (from standard triads to thirteenth chords)
- Parts mostly display for now, outputs after last part output root note
- Switch for suspensions (sus4) close to fifth-axis
- Transpose rotates within octave
- Outputs in V/oct
- Tonic output (T) for tonic drone
- Voicing inc/decreases octaves to the right/left sequentially by part (small settings to the right give chord inversions)

### SNS - Euclidean Sequencer

- I know. but this one looks cool and has a few additional features:
- Up to 32 steps
- Accents: nested euclidean sequence on main sequence
- Rotate sequence and accents
- Up to 32 pads (silent steps)
- All inputs scaled to each other in what i think is a sensible way - great for LFO modulation of inputs
- CLK/RST thru for easy chaining
- Turing mode to translate pattern into CV a la Turing Machine

### Snake

 - 16 internal busses 0-F with 10 lines per buss to bridge large distances or generally tidy up the patch
- First connection to input locks input (green led - red on all other instances)
- Works best with slow signal. In case of parallel processin check difference between original and bussed signal by means of inverting summer and scope

### Sssh - Noise / S+H

- 4x noise / S+H - inspired by the lowest section from [Kinks](https://mutable-instruments.net/modules/kinks/)
- Triggers are normalled from top to bottom
- Inputs are normalled to noise
- Noises are four calls to generator (not a copy)

### Wriggle - Spring Model

- Follows input with a given stiffness and damping of the spring
- Mass normalized to one
- Output can be scaled/offset
- Designed to implement "Woggle CV" (works best at LFO rates)

  
### Gnome - Synth Voice

- An all-in-one module inspired by [MFB Nanozwerg](http://mfberlin.de/en/modules/nanozwerg_pro_e/)
- Because sometimes you just need the basics and want to get started fast
- The submodules are mostly copied from the Fundamental modules with some omissions and additions:
- VCO: analog part of Fundamental VCO, morphing
- Sub-OSC 1/2 Oct / Noise
- LFO from Fundamental LFO, morphing + S+H
- ADSR from Fundamental ADSR
- Biquad VCF with resonance gain-compensation

