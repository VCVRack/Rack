# PvC Modules

##  Changelog

#### [0.5.8](https://github.com/phdsg/PvC/tree/0.5.8)
##### All Modules
  - layout changes. still trying a few things...  

##### [NEW] Bang!, da Button
  - momentary button that fires gates/triggers on press and release events  
  - also triggers: flip flops and some switches (A or B into Out / In into A or B)  

##### [NEW] AorBtoOut / InToAorB
  - chance switches

##### ComPair
  - half the panel size  

##### [NEW] CoSuOf
  - comparator, substractor, offsetter based on the functionality of the D-A167  

##### [NEW] FlipOLogic
  - logic gates and then some  

##### [NEW] Geigths
  - another sort of comparator (inspired by the bartos flur)
  - fires a pulse if the input is in range of one the 8 outputs. [10V/8]
  - pulse length is adjustable, also the input signal can be trimmed and offset  
  - with trig input plugged the unit switches into sample and hold mode  

##### ShutIt
  - 2HP wider  
  - new section at the bottom to unmute/mute/flip all channels

##### [NEW] SlimSeq
  - 16 step sequencer / sequential switch (inspired by tm 8s, code based on m.lueders seq. switch)  
  
##### [NEW] TaHaSaHaN
  - Track and Hold / Sample and Hold / Noise  

##### Vamps
  - no more dual but still stereo.  

##### [REMOVED] Multy, Oomph
  - Multy gone for good. r.i.p.  
  - Oomph (or some kind of distortion) might a return at some point.  

***

#### [0.5.7](https://github.com/phdsg/PvC/tree/0.5.7)
##### ComPair
  - initialization  
  
##### ShutIt
  - cv inputs now normalized to last connected above  
  - panel layout  
  - two-color lights
  
##### VUBar
  - fixed lights not shutting off when input is unplugged  
  
***

#### [0.5.6](https://github.com/phdsg/PvC/tree/0.5.6) ("Happy New Year")
##### All Modules
 - slight visual changes (panels,knobs,ports)
    
##### ComPair
 - multi color compare LED and slightly changed panel layout
 - testing bi-polar outputs option
 - inverter buttons are now on the compare LEDs
 
##### vAMPs
 - port layout

##### VUBar
 - Brightness Knob
 - Clip LED is now a toggle to select the dB interval of the lights
 - nicer green - red transition
 - 3 Lights less, remaining 12 are bigger tho.

##### Shape (now Oomph)
 - changed working title to Oomph

##### [NEW] ShutIt
 - 8 x triggerable mutes
 - inputs are normalized to the last connected above
 - panel fields around the ports are invisible manual mute triggers
  
##### [SOON DEPRECATED] Mul\[L\]ty
 - ShutIt makes Mu[L]ty pretty much obsolete (at least for me, so with 0.6 multy won't be part of the pack anymore)

##### [NEW] SumIt (working title)
 - 12 into 1 mixer
 - sums up to 12 inputs and divides the signal by the number of connected inputs
 - final output has a gain knob and its also clamped to [-10..10]V

***

#### [0.5.5](https://github.com/phdsg/PvC/tree/0.5.5) ("ComPair beta3")
##### ComPair
 - each channel now has a toggle to invert it's output to the logic section.

***

#### [0.5.4](https://github.com/phdsg/PvC/tree/0.5.4) ("ComPair beta2")
##### ComPair
 - [FIX] typo in cv input normalization code
 - above/below-the-window lights
 - layout and labeled panel

##### VUBar
 - through output

***

#### [0.5.3](https://github.com/phdsg/PvC/tree/0.5.3) ("ComPair beta")
##### [NEW] ComPair
 - dual window comparator inspired by the joranalogue compare2

##### [NEW] Shape (working title)
 - primitive waveshaping distortion

***

#### [0.5.2](https://github.com/phdsg/PvC/tree/0.5.2) ("can't have too many vcas")
##### [NEW] vAMPs
 - slim stereo mod of the fundamental vca

***

#### [0.5.1](https://github.com/phdsg/PvC/tree/0.5.1) ("cutting the fat")
##### Mu\[L\]ty
 - [NEW] slim layout
 - [NEW] less outputs (breaks patches with old version)

##### VUBar
 - [NEW] more lights

***

#### [0.5.0](https://github.com/phdsg/PvC/tree/0.5.0) ("hello world")
##### [NEW] Mu\[L\]ty
 - 1 X 10 Multiple with mute toggles

##### [NEW] VUBar 
 - simple vumeter
