# VeeSeeVSTRack

*VeeSeeVSTRack* is an adaption of VCV Rack for the VST2.4 format.

+ supports multiple instances
+ supports VST MIDI input 
+ supports up to 8 audio outputs
+ supports up to 8 audio inputs
+ support VST program chunks (=> patches are saved with the DAW's project file or as .fxp files)
- does not support plugin DLLs due to VCV Rack's architecture which prevents this when it is run as a plugin itself
   - future releases may contain additional (open source) add-ons modules

Here's a demo video of it: https://vimeo.com/277703414

Tested in 
  - Eureka (my own work-in-progress VST host)
  - Cockos Reaper
  - Propellerhead Reason 10

# Add-on modules

The following add-on modules are statically linked with the VST plugin:
 - AS.ADSR
 - AS.AtNuVrTr
 - AS.BPMCalc
 - AS.BPMClock
 - AS.BlankPanel4
 - AS.BlankPanel6
 - AS.BlankPanel8
 - AS.BlankPanelSpecial
 - AS.Cv2T
 - AS.DelayPlusFx
 - AS.DelayPlusStereoFx
 - AS.Flow
 - AS.KillGate
 - AS.LaunchGate
 - AS.Merge2.5
 - AS.Mixer8ch
 - AS.MonoVUmeter
 - AS.Multiple2.5
 - AS.PhaserFx
 - AS.QuadVCA
 - AS.ReverbFx
 - AS.ReverbStereoFx
 - AS.SEQ16
 - AS.SawOsc
 - AS.SignalDelay
 - AS.SineOsc
 - AS.Steps
 - AS.SuperDriveFx
 - AS.TremoloFx
 - AS.TremoloStereoFx
 - AS.TriLFO
 - AS.TriggersMKI
 - AS.TriggersMKII
 - AS.VCA
 - AS.WaveShaper
 - AS.StereoVUmeter
 - AudibleInstruments.Braids
 - AudibleInstruments.Elements
 - AudibleInstruments.Tides
 - AudibleInstruments.Clouds
 - AudibleInstruments.Warps
 - AudibleInstruments.Rings
 - AudibleInstruments.Links
 - AudibleInstruments.Kinks
 - AudibleInstruments.Shades
 - AudibleInstruments.Branches
 - AudibleInstruments.Blinds
 - AudibleInstruments.Veils
 - AudibleInstruments.Frames
 - Befaco.ABC
 - Befaco.DualAtenuverter
 - Befaco.EvenVCO
 - Befaco.Mixer
 - Befaco.Rampage
 - Befaco.SlewLimiter
 - Befaco.SpringReverb
 - Bogaudio.VCO
 - Bogaudio.XCO
 - Bogaudio.Additator
 - Bogaudio.FMOp
 - Bogaudio.LFO
 - Bogaudio.EightFO
 - Bogaudio.DADSRH
 - Bogaudio.DADSRHPlus
 - Bogaudio.DGate
 - Bogaudio.Shaper
 - Bogaudio.ShaperPlus
 - Bogaudio.ADSR
 - Bogaudio.Follow
 - Bogaudio.Mix4
 - Bogaudio.Mix8
 - Bogaudio.VCM
 - Bogaudio.Pan
 - Bogaudio.XFade
 - Bogaudio.VCA
 - Bogaudio.VCAmp
 - Bogaudio.Analyzer
 - Bogaudio.VU
 - Bogaudio.Detune
 - Bogaudio.Stack
 - Bogaudio.Reftone
 - Bogaudio.Bool
 - Bogaudio.CVD
 - Bogaudio.FlipFlop
 - Bogaudio.Manual
 - Bogaudio.Mult
 - Bogaudio.Noise
 - Bogaudio.Offset
 - Bogaudio.SampleHold
 - Bogaudio.Sums
 - Bogaudio.Switch
 - Bogaudio.Lag
 - Bogaudio.RM
 - Bogaudio.Test
 - Bogaudio.Test2
 - Bogaudio.ThreeHP
 - Bogaudio.SixHP
 - Bogaudio.EightHP
 - Bogaudio.TenHP
 - Bogaudio.TwelveHP
 - Bogaudio.ThirteenHP
 - Bogaudio.FifteenHP
 - Bogaudio.EighteenHP
 - Bogaudio.TwentyHP
 - Bogaudio.TwentyTwoHP
 - Bogaudio.TwentyFiveHP
 - Bogaudio.ThirtyHP
 - cf.trSEQ
 - cf.LEDSEQ
 - cf.L3DS3Q
 - cf.SLIDERSEQ
 - cf.PLAYER
 - cf.STEPS
 - cf.METRO
 - cf.EACH
 - cf.FOUR
 - cf.PEAK
 - cf.MONO
 - cf.STEREO
 - cf.MASTER
 - cf.SUB
 - cf.CUBE
 - cf.PATCH
 - cf.LEDS
 - cf.DAVE
 - ESeries.E340
 - ErraticInstruments.MPEToCV
 - ErraticInstruments.QuadMPEToCV
 - Fundamentals.8vert
 - Fundamentals.ADSR
 - Fundamentals.Delay
 - Fundamentals.LFO
 - Fundamentals.LFO2
 - Fundamentals.Mutes
 - Fundamentals.SEQ3
 - Fundamentals.SequentialSwitch1
 - Fundamentals.SequentialSwitch2
 - Fundamentals.Scope
 - Fundamentals.Unity
 - Fundamentals.VCA
 - Fundamentals.VCF
 - Fundamentals.VCMixer
 - Fundamentals.VCO
 - Fundamentals.VCO2
 - HetrickCV.TwoToFour
 - HetrickCV.AnalogToDigital
 - HetrickCV.ASR
 - HetrickCV.Bitshift
 - HetrickCV.BlankPanel
 - HetrickCV.Boolean3
 - HetrickCV.Comparator
 - HetrickCV.Contrast
 - HetrickCV.Crackle
 - HetrickCV.Delta
 - HetrickCV.DigitalToAnalog
 - HetrickCV.Dust
 - HetrickCV.Exponent
 - HetrickCV.FlipFlop
 - HetrickCV.FlipPan
 - HetrickCV.GateJunction
 - HetrickCV.LogicCombine
 - HetrickCV.RandomGates
 - HetrickCV.Rotator
 - HetrickCV.Scanner
 - HetrickCV.Waveshape
 - Koralfx.Beatovnik
 - Koralfx.Mixovnik
 - Koralfx.Nullovnik4
 - Koralfx.Nullovnik6
 - Koralfx.Presetovnik
 - Koralfx.Quantovnik
 - Koralfx.Scorovnik
 - LindenbergResearch.SimpleFilter
 - LindenbergResearch.MS20Filter
 - LindenbergResearch.AlmaFilter
 - LindenbergResearch.ReShaper
 - LindenbergResearch.BlankPanel
 - LindenbergResearch.BlankPanelM1
 - Qwelk.Automaton
 - Qwelk.Byte
 - Qwelk.Chaos
 - Qwelk.Column
 - Qwelk.Gate
 - Qwelk.Or
 - Qwelk.Not
 - Qwelk.Xor
 - Qwelk.Mix
 - Qwelk.News
 - Qwelk.Scaler
 - Qwelk.Wrap
 - Qwelk.XFade
 - SonusModular.Addiction
 - SonusModular.Bitter
 - SonusModular.Bymidside
 - SonusModular.Campione
 - SonusModular.Chainsaw
 - SonusModular.Ctrl
 - SonusModular.Deathcrush
 - SonusModular.Harmony
 - SonusModular.Ladrone
 - SonusModular.Luppolo
 - SonusModular.Luppolo3
 - SonusModular.Micromacro
 - SonusModular.Multimulti
 - SonusModular.Oktagon
 - SonusModular.Osculum
 - SonusModular.Paramath
 - SonusModular.Piconoise
 - SonusModular.Pusher
 - SonusModular.Ringo
 - SonusModular.Scramblase
 - SonusModular.Twoff
 - SonusModular.Yabp
 - SubmarineFree.AG106
 - SubmarineFree.BB120
 - SubmarineFree.FF110
 - SubmarineFree.FF120
 - SubmarineFree.FF212
 - SubmarineFree.LA108
 - SubmarineFree.LD106
 - SubmarineFree.NG112
 - SubmarineFree.OG106
 - SubmarineFree.PG112
 - SubmarineFree.PO101
 - SubmarineFree.PO102
 - SubmarineFree.PO204
 - SubmarineFree.WK101
 - SubmarineFree.WK205
 - SubmarineFree.XF101
 - SubmarineFree.XF102
 - SubmarineFree.XF104
 - SubmarineFree.XF201
 - SubmarineFree.XF202
 - SubmarineFree.XG106
 - SubmarineFree.BP101
 - SubmarineFree.BP102
 - SubmarineFree.BP104
 - SubmarineFree.BP108
 - SubmarineFree.BP110
 - SubmarineFree.BP112
 - SubmarineFree.BP116
 - SubmarineFree.BP120
 - SubmarineFree.BP124
 - SubmarineFree.BP132
 - Template.MyModule
 - Valley.Topograph
 - Valley.UGraph
 - Valley.Dexter
 - Valley.Plateau
 - VultModules.Debriatus
 - VultModules.Lateralus
 - VultModules.Rescomb
 - VultModules.Splie
 - VultModules.Stabile
 - VultModules.Tangents
 - VultModules.Tohe
 - VultModules.Trummor

Please notice that the Audible/Mutable Instruments modules appear under a different name in the UI.
For example, "Clouds" is listed as "Texture Synthesizer".

# How to build

Prerequisites:
- GNU Bash (tested with MSYS1.0 / bash v2.04.0(1))
- GNU make (tested with v3.79.1)
- Microsoft Visual Studio C++ compiler toolchain (tested with v19.00.24225.1)
- Steinberg VST2.4 SDK

If you want to build the dependent libraries, you may need additional SDKs.
Precompiled libs can be found in the `dep/lib/msvc/` folder.

```
$ git clone https://github.com/bsp2/VeeSeeVSTRack.git
```
```
$ cd VeeSeeVSTRack/
```

<unpack `dep/dep.7z`>
(contains the source codes and MSVC-precompiled libraries)

Edit `dep/yac/install_msvc.mk` and adjust the `LIB_INSTALL_PREFIX`, `WINDDK_PATH`, `VCTK`, `W32API_INC`, `W32API_LIB` as required.

```
$ alias m="make -j 20 makefile.msvc"
$ m lib
$ cd plugins
$ m bin
$ cd ..
```
Edit `vst2_common_msvc_pre.mk` and adjust the VST2 SDK path
```
$ m bin
```
If the build succeeded, the effect and instrument plugin DLLs can now be found in the `vst2_bin/` folder.

Last but not least, please don't ask me for the VST2 SDK.
It is not permitted to redistribute it and Steinberg has discontinued it.
I heard that the `aeffect.h` / `aeffectx.h` files are still included in the VST3 SDK.


# Changelog
see [vst2_bin/CHANGELOG_VST.txt](vst2_bin/CHANGELOG_VST.txt)

# VCV Rack

For more info about VCV rack, see https://vcvrack.com/

# Support

Keep in mind that this is NOT AN OFFICIAL VCV RACK RELEASE.
Please DO NOT contact the VCV Rack team if you need any support.
You may get some support at https://www.kvraudio.com/forum/viewtopic.php?f=23&t=507216

~bsp
