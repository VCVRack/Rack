# VeeSeeVSTRack

*VeeSeeVSTRack* is an adaption of VCV Rack for the VST2.4 format.

+ supports multiple instances
+ supports VST MIDI input 
+ supports up to 8 audio outputs
+ supports up to 8 audio inputs
+ supports VST program chunks (=> patches are saved with the DAW's project file or as .fxp files)
+ supports VST host timing (audioMasterGetTime / kVstTempoValid / kVstTransportPlaying, see Core.MIDI-1 module)
+ supports VST parameters (send / recv)
+ supports dynamically loaded plugin DLLs
   - the plugin.dll files are _not_ binary compatible with the VCV Rack plugins !
   - there's a [plugin SDK](#dynamically-loaded-plugins-via-plugin-sdk) (for Microsoft Visual Studio 2017 Community Edition) which can be used to build new plugins without checking out this entire GIT repository
+ supports internal oversampling (up to 16x with configurable quality)
   - the number of oversampled I/O channels can be limited to save CPU time
   - offline rendering uses separate settings (highest quality by default)
+ supports idle-detection
   - wake up on MIDI note on or audio input
+ comes with 827 prebuilt modules

**Windows** version tested in:
  - Eureka (my own work-in-progress VST host)
  - Cockos Reaper
  - Propellerhead Reason 10
  - Steinberg Cubase Pro 9.5.30
  - LMMS 1.2.0-rc6
  - Jeskola Buzz 1.2 (build 1503 / x64)
  - Bitwig Studio
  - FLStudio
  - Studio One 4.1.1
  - according to users: works in Nuendo
  - according to users: works in Ableton Live

**Linux** version tested in:
  - Bitwig Studio 2.4.1
  - Renoise 3.1.0
  - according to users: works in Qtractor 0.9.2


# Downloads

## Windows
- [veeseevstrack_0_6_1_win64_bin-10Mar2019.7z](https://github.com/bsp2/releases/raw/master/vsvr/veeseevstrack_0_6_1_win64_bin-10Mar2019.7z) (64bit)
- [veeseevstrack_0_6_1_win32_bin-10Mar2019.7z](https://github.com/bsp2/releases/raw/master/vsvr/veeseevstrack_0_6_1_win32_bin-10Mar2019.7z) (32bit, experimental)

## Linux
- [veeseevstrack_0_6_1_lin64_bin-08March2019b.tar.gz](http://linux-sound.org/misc/veeseevstrack_0_6_1_lin64_bin-08March2019b.tar.gz) (64bit) (Dave's latest build)
- [veeseevstrack_0_6_1_lin64_bin-31Oct2018.7z](https://github.com/bsp2/releases/raw/master/vsvr/veeseevstrack_0_6_1_lin64_bin-31Oct2018.7z) (64bit/SSE2, beta) (old build)
! note: build the source to get the latest version

## Notes
- The effect plugin can used be as an instrument, too. You just have to send it MIDI events !

- The idle detection is enabled by default. A side effect of this is that e.g. sequencer modules will not play while the plugin is idle. Please turn off the idle detection in the toolbar menu (set it to "Always Active") in this case. Since some of the UI handling is tied to the engine, certain widgets (typically buttons, e.g. the waveform selector in the Macro Oscillator module) will not work while the plugin is idle. Play some notes (or turn off the idle detection) in this case.

- The binary releases come with 702 precompiled modules.

- To make the Linux version work in a [VirtualBox](https://www.virtualbox.org/) VM, the `fbo` and `fbo_shared` options in `settings.json` have to be set to `false`


# Installation
Extract the 7zip archive and move the `vst2_bin/` folder to a VST2 plugin directory.
Alternatively, add the folder to your DAW's plugin directory list.


# Changelog
see [vst2_bin/CHANGELOG_VST.txt](vst2_bin/CHANGELOG_VST.txt)


# Demo Video

Here are some demo videos of it:
 - https://vimeo.com/277703414 (early version)
 - https://vimeo.com/287875320 (guitar synthesis sound demo).
 - https://vimeo.com/288076107 (tuned delay line ambient pad drone)
 - https://vimeo.com/288594338 (RMS filter/pwm envelope follower + compressor demo)
 - https://vimeo.com/288968750 (DownSampler module)


# Add-on modules

The binary distribution contains the following (34) dynamically loaded add-on modules:
 - bsp.AttenuMixer
 - bsp.Bias
 - bsp.DownSampler
 - bsp.Legato
 - bsp.Obxd_VCF
 - bsp.Rescaler
 - bsp.RMS
 - bsp.Scanner
 - bsp.Sway
 - bsp.TunedDelayLine
 - dBiz.dBizBlank
 - dBiz.Contorno
 - dBiz.Chord
 - dBiz.Bene
 - dBiz.Bene2
 - dBiz.BenePads
 - dBiz.DAOSC
 - dBiz.Divider
 - dBiz.DualFilter
 - dBiz.DVCO
 - dBiz.FourSeq
 - dBiz.Multiple
 - dBiz.PerfMixer
 - dBiz.Remix
 - dBiz.SmiX
 - dBiz.SubMix
 - dBiz.SuHa
 - dBiz.Transpose
 - dBiz.TROSC
 - dBiz.Utility
 - dBiz.Util2
 - dBiz.VCA530
 - dBiz.Verbo
 - Template_shared.MyModule


The following (793) add-on modules are statically linked with the VST plugin:
 - 21kHz.D_Inf
 - 21kHz.PalmLoop
 - Alikins.IdleSwitch
 - Alikins.MomentaryOnButtons
 - Alikins.BigMuteButton
 - Alikins.ColorPanel
 - Alikins.GateLength
 - Alikins.SpecificValue
 - Alikins.Reference
 - Alikins.HoveredValue
 - Alikins.InjectValue
 - Alikins.ShiftPedal
 - Alikins.ValueSaver
 - alto777_LFSR.FG8
 - alto777_LFSR.Psychtone
 - alto777_LFSR.Amuse
 - alto777_LFSR.a7Utility
 - alto777_LFSR.cheapFX
 - alto777_LFSR.Divada
 - alto777_LFSR.YASeq3
 - AmalgamatedHarmonics.Arpeggiator
 - AmalgamatedHarmonics.Arpeggiator2
 - AmalgamatedHarmonics.Circle
 - AmalgamatedHarmonics.Imperfect
 - AmalgamatedHarmonics.Imperfect2
 - AmalgamatedHarmonics.Progress
 - AmalgamatedHarmonics.Ruckus
 - AmalgamatedHarmonics.ScaleQuantizer
 - AmalgamatedHarmonics.ScaleQuantizer2
 - AmalgamatedHarmonics.SLN
 - AmalgamatedHarmonics.Arp31
 - AmalgamatedHarmonics.Arp32
 - AmalgamatedHarmonics.Bombe
 - AmalgamatedHarmonics.Chord
 - AmalgamatedHarmonics.Galaxy
 - AmalgamatedHarmonics.Generative
 - arjo_modules.Seq
 - arjo_modules.Count
 - arjo_modules.Switch
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
 - AS.ReScale
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
 - AS.TriggersMKIII
 - AS.VCA
 - AS.WaveShaper
 - AS.StereoVUmeter
 - AS.Mixer2ch
 - AS.Mixer4ch
 - AS.BPMCalc2
 - AS.ZeroCV2T
 - AudibleInstruments.Blinds
 - AudibleInstruments.Braids
 - AudibleInstruments.Branches
 - AudibleInstruments.Clouds
 - AudibleInstruments.Elements
 - AudibleInstruments.Frames
 - AudibleInstruments.Kinks
 - AudibleInstruments.Links
 - AudibleInstruments.Marbles
 - AudibleInstruments.Plaits
 - AudibleInstruments.Rings
 - AudibleInstruments.Shades
 - AudibleInstruments.Stages
 - AudibleInstruments.Tides
 - AudibleInstruments.Veils
 - AudibleInstruments.Warps
 - Autodafe.Multiple18
 - Autodafe.Multiple28
 - Autodafe.LFOWidget
 - Autodafe.Keyboard
 - Autodafe.BPMClock
 - Autodafe.ClockDivider
 - Autodafe.SEQ8
 - Autodafe.SEQ16
 - Autodafe.TriggerSeq
 - Autodafe.FixedFilter
 - Autodafe.MultiModeFilter
 - Autodafe.FormantFilter
 - Autodafe.FoldBack
 - Autodafe.BitCrusher
 - Autodafe.PhaserFx
 - Autodafe.ChorusFx
 - Autodafe.ReverbFx
 - BaconMusic.Glissinator
 - BaconMusic.PolyGnome
 - BaconMusic.QuantEyes
 - BaconMusic.SampleDelay
 - BaconMusic.SortaChorus
 - BaconMusic.ChipNoise
 - BaconMusic.ChipWaves
 - BaconMusic.ChipYourWave
 - BaconMusic.KarplusStrongPoly
 - BaconMusic.ALingADing
 - BaconMusic.Bitulator
 - Bark.TrimLFO
 - Bark.TrimLFObpm
 - Bark.QuadLogic
 - Bark.Panel6
 - Bark.OneBand
 - Befaco.ABC
 - Befaco.DualAtenuverter
 - Befaco.EvenVCO
 - Befaco.Mixer
 - Befaco.Rampage
 - Befaco.SlewLimiter
 - Befaco.SpringReverb
 - Bidoo.DTROY
 - Bidoo.BORDL
 - Bidoo.MU
 - Bidoo.TOCANTE
 - Bidoo.CHUTE
 - Bidoo.LATE
 - Bidoo.LOURDE
 - Bidoo.ACNE
 - Bidoo.DUKE
 - Bidoo.MOIRE
 - Bidoo.FORK
 - Bidoo.TIARE
 - Bidoo.CLACOS
 - Bidoo.ANTN
 - Bidoo.LIMBO
 - Bidoo.PERCO
 - Bidoo.BAR
 - Bidoo.ZINC
 - Bidoo.VOID
 - Bidoo.SIGMA
 - Bidoo.CANARD
 - Bidoo.DFUZE
 - Bidoo.OUAIVE
 - Bidoo.RABBIT
 - Bidoo.ZOUMAI
 - Bidoo.MS
 - Bidoo.EMILE
 - Bidoo.GARCON
 - Bidoo.PENEQUE
 - Bidoo.LIMONADE
 - Bidoo.FFILTR
 - Bidoo.HCTIP
 - Bidoo.REI
 - Bidoo.CURT
 - Bidoo.BISTROT
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
 - BogAudio.AD
 - BogAudio.AMRM
 - BogAudio.Matrix88
 - BogAudio.UMix
 - Bogaudio.AnalyzerXL
 - Bogaudio.Blank3
 - Bogaudio.Blank6
 - Bogaudio.Clpr
 - Bogaudio.Cmp
 - Bogaudio.LLFO
 - Bogaudio.Lmtr
 - Bogaudio.Mute8
 - Bogaudio.Nsgt
 - Bogaudio.Pressor
 - Bogaudio.Slew
 - CastleRocktronics.Cubefader
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
 - cf.ALGEBRA
 - cf.BUFFER
 - cf.CHOKE
 - cf.CUTS
 - cf.DISTO
 - cf.PLAY
 - cf.VARIABLE
 - com-soundchasing-stochasm.Resonator
 - computerscare.ComputerscareDebug
 - computerscare.ComputerscarePatchSequencer
 - DHE-Modules.BoosterStage
 - DHE-Modules.Cubic
 - DHE-Modules.Hostage
 - DHE-Modules.Stage
 - DHE-Modules.Swave
 - DHE-Modules.Upstage
 - DHE-Modules.Func
 - DHE-Modules.Func6
 - DHE-Modules.Ranger
 - DHE-Modules.Tapers
 - DHE-Modules.Xycloid
 - DrumKit.BD9
 - DrumKit.Snare
 - DrumKit.ClosedHH
 - DrumKit.OpenHH
 - DrumKit.DMX
 - Edge.WTFDoveVCO
 - Edge.K_Rush
 - EH_modules.FV1Emu
 - ESeries.E340
 - ErraticInstruments.MPEToCV
 - ErraticInstruments.QuadMPEToCV
 - FrankBuss.Formula
 - FrozenWasteland.BPMLFO
 - FrozenWasteland.BPMLFO2
 - FrozenWasteland.DamianLillard
 - FrozenWasteland.EverlastingGlottalStopper
 - FrozenWasteland.HairPick
 - FrozenWasteland.LissajousLFO
 - FrozenWasteland.MrBlueSky
 - FrozenWasteland.TheOneRingModulator
 - FrozenWasteland.PhasedLockedLoop
 - FrozenWasteland.PortlandWeather
 - FrozenWasteland.QuadEuclideanRhythm
 - FrozenWasteland.QuadGolombRulerRhythm
 - FrozenWasteland.QuantussyCell
 - FrozenWasteland.RouletteLFO
 - FrozenWasteland.SeriouslySlowLFO
 - FrozenWasteland.VoxInhumana
 - FrozenWasteland.CDCSeriouslySlowLFO
 - Fundamental.8vert
 - Fundamental.ADSR
 - Fundamental.Delay
 - Fundamental.LFO
 - Fundamental.LFO2
 - Fundamental.Mutes
 - Fundamental.SEQ3
 - Fundamental.SequentialSwitch1
 - Fundamental.SequentialSwitch2
 - Fundamental.Scope
 - Fundamental.Unity
 - Fundamental.VCA
 - Fundamental.VCA-1
 - Fundamental.VCF
 - Fundamental.VCMixer
 - Fundamental.VCO
 - Fundamental.VCO2
 - Fundamental.Octave
 - Geodesics.BlackHoles
 - Geodesics.Pulsars
 - Geodesics.Branes
 - Geodesics.Ions
 - Geodesics.BlankLogo
 - Geodesics.BlankInfo
 - Geodesics.Entropia
 - Gratrix.VCO_F1
 - Gratrix.VCO_F2
 - Gratrix.VCF_F1
 - Gratrix.VCA_F1
 - Gratrix.ADSR_F1
 - Gratrix.Chord_G1
 - Gratrix.Octave_G1
 - Gratrix.Fade_G1
 - Gratrix.Fade_G2
 - Gratrix.Binary_G1
 - Gratrix.Seq_G1
 - Gratrix.Keys_G1
 - Gratrix.VU_G1
 - Gratrix.Blank_03
 - Gratrix.Blank_06
 - Gratrix.Blank_09
 - Gratrix.Blank_12
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
 - huaba.EQ3
 - huaba.ABBus
 - ImpromptuModular.Tact
 - ImpromptuModular.TwelveKey
 - ImpromptuModular.Clocked
 - ImpromptuModular.MidiFile
 - ImpromptuModular.PhraseSeq16
 - ImpromptuModular.PhraseSeq32
 - ImpromptuModular.GateSeq64
 - ImpromptuModular.WriteSeq32
 - ImpromptuModular.WriteSeq64
 - ImpromptuModular.BigButtonSeq
 - ImpromptuModular.SemiModularSynth
 - ImpromptuModular.BlankPanel
 - ImpromptuModular.BigButtonSeq2
 - ImpromptuModular.Foundry
 - ImpromptuModular.FourView
 - ImpromptuModular.Tact1
 - JE.SimpleWaveFolder
 - JE.RingModulator
 - JW_Modules.Cat
 - JW_Modules.BouncyBalls
 - JW_Modules.FullScope
 - JW_Modules.GridSeq
 - JW_Modules.Quantizer
 - JW_Modules.MinMax
 - JW_Modules.NoteSeq
 - JW_Modules.SimpleClock
 - JW_Modules.ThingThing
 - JW_Modules.WavHead
 - JW_Modules.XYPad
 - JW-Modules.BlankPanelSmall
 - JW-Modules.BlankPanelMedium
 - JW-Modules.BlankPanelLarge
 - Koralfx.Beatovnik
 - Koralfx.Mixovnik
 - Koralfx.Nullovnik4
 - Koralfx.Nullovnik6
 - Koralfx.Presetovnik
 - Koralfx.Quantovnik
 - Koralfx.Scorovnik
 - LabSeven.LS3340VCO
 - LindenbergResearch.SimpleFilter
 - LindenbergResearch.MS20Filter
 - LindenbergResearch.AlmaFilter
 - LindenbergResearch.ReShaper
 - LindenbergResearch.BlankPanel
 - LindenbergResearch.BlankPanelM1
 - LindenbergResearch.BlankPanelWood
 - LindenbergResearch.VCO
 - LindenbergResearch.Westcoast (preview)
 - LindenbergResearch.Korg35 (WIP)
 - LindenbergResearch.DiodeVCF
 - LindenbergResearch.Speck (WIP)
 - LOGinstruments.constant
 - LOGinstruments.constant2
 - LOGinstruments.Speck
 - LOGinstruments.Britix
 - LOGinstruments.Compa
 - LOGinstruments.LessMess
 - LOGinstruments.Velvet
 - LOGinstruments.Crystal
 - mental.MentalSubMixer
 - mental.MentalMults
 - mental.MentalMixer
 - mental.MentalFold
 - mental.MentalClip
 - mental.MentalGates
 - mental.MentalABSwitches
 - mental.MentalQuantiser
 - mental.MentalChord
 - mental.MentalMuxes
 - mental.MentalLogic
 - mental.MentalButtons
 - mental.MentalSums
 - mental.MentalPitchShift
 - mental.MentalClockDivider
 - mental.MentalCartesian
 - mental.MentalPatchMatrix
 - mental.MentalBinaryDecoder
 - mental.MentalSwitch8
 - mental.MentalMux8
 - mental.MentalCounters
 - mental.MentalKnobs
 - mental.MentalGateMaker
 - mental.MentalMasterClock
 - mental.MentalPatchNotes
 - mental.MentalQuadLFO
 - mental.MentalRadioButtons
 - MicMusic.Distortion
 - MicMusic.Adder
 - ML_modules.OctaPlus
 - ML_modules.OctaTimes
 - moDllz.MIDIPoly
 - moDllz.TwinGlider
 - moDllz.MIDIdualCV
 - moDllz.XBender
 - modular80.Logistiker
 - modular80.Nosering
 - modular80.RadioMusic
 - mscHack.MasterClockx4
 - mscHack.Seq_3x16x16
 - mscHack.SEQ_6x32x16
 - mscHack.Seq_Triad2
 - mscHack.SEQ_Envelope_8
 - mscHack.Maude_221
 - mscHack.ARP700
 - mscHack.SynthDrums
 - mscHack.XFade
 - mscHack.Mix_1x4_Stereo
 - mscHack.Mix_2x4_Stereo
 - mscHack.Mix_4x4_Stereo
 - mscHack.Mix_24_4_4
 - mscHack.StepDelay
 - mscHack.PingPong
 - mscHack.Osc_3Ch
 - mscHack.Compressor
 - mscHack.Alienz
 - mscHack.ASAF8
 - mscHack.Dronez
 - mscHack.Mixer_9_3_4
 - mscHack.Mixer_16_4_4
 - mscHack.Mixer_24_4_4
 - mscHack.Morze
 - mscHack.OSC_WaveMorph_3
 - mscHack.Windz
 - mtsch_plugins.Sum
 - mtsch_plugins.Rationals
 - mtsch_plugins.TriggerPanic
 - ML_modules.Quantizer
 - ML_modules.Quantum
 - ML_modules.TrigBuf
 - ML_modules.SeqSwitch
 - ML_modules.SeqSwitch2
 - ML_modules.ShiftRegister
 - ML_modules.ShiftRegister2
 - ML_modules.FreeVerb
 - ML_modules.Sum8
 - ML_modules.Sum8mk2
 - ML_modules.SH8
 - ML_modules.Constants
 - ML_modules.Counter
 - ML_modules.TrigDelay
 - ML_modules.BPMdetect
 - ML_modules.VoltMeter
 - ML_modules.OctaFlop
 - ML_modules.OctaTrig
 - ML_modules.OctaSwitch
 - ML_modules.TrigSwitch
 - ML_modules.TrigSwitch2
 - ML_modules.TrigSwitch3
 - ML_modules.TrigSwitch3_2
 - NauModular.Tension
 - NauModular.Function
 - NauModular.Perlin
 - NauModular.S_h_it
 - NauModular.BitHammer
 - NauModular.Osc
 - NauModular.Mrcheb
 - NauModular.Pith
 - Nohmad.Noise
 - Nohmad.StrangeAttractors
 - noobhour.Baseliner
 - noobhour.Bsl1r
 - noobhour.Customscaler
 - Ohmer.KlokSpid
 - Ohmer.RKD
 - Ohmer.RKDBRK
 - Ohmer.Metriks
 - Ohmer.Splitter1x9
 - Ohmer.BlankPanel1
 - Ohmer.BlankPanel2
 - Ohmer.BlankPanel4
 - Ohmer.BlankPanel8
 - Ohmer.BlankPanel16
 - Ohmer.BlankPanel32
 - PG-Instruments.PGSEQ3
 - PG-Instruments.PGPanner
 - PG-Instruments.PGQuadPanner
 - PG-Instruments.PGOctPanner
 - PG-Instruments.PGVCF
 - PG-Instruments.PGStereoVCF
 - PG-Instruments.PGEcho
 - PG-Instruments.PGStereoEcho
 - PG-Instruments.PGStereoPingPongEcho
 - QuantalAudio.MasterMixer
 - QuantalAudio.BufferedMult
 - QuantalAudio.UnityMix
 - QuantalAudio.DaisyChannel
 - QuantalAudio.DaisyMaster
 - QuantalAudio.Horsehair
 - QuantalAudio.Blank1
 - QuantalAudio.Blank3
 - QuantalAudio.Blank5
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
 - rcm.CV0to10Module
 - rcm.CV5to5Module
 - rcm.CVMmtModule
 - rcm.CVS0to10Module
 - rcm.CVTglModule
 - rcm.DuckModule
 - rcm.LoadCounter
 - rcm.PianoRollModule
 - RJModules.Supersaw
 - RJModules.TwinLFO
 - RJModules.Noise
 - RJModules.RangeLFO
 - RJModules.BitCrush
 - RJModules.Widener
 - RJModules.FilterDelay
 - RJModules.Sidechain
 - RJModules.Stutter
 - RJModules.Filter
 - RJModules.Filters
 - RJModules.Notch
 - RJModules.Integers
 - RJModules.Floats
 - RJModules.Randoms
 - RJModules.LRMixer
 - RJModules.Mono
 - RJModules.Volumes
 - RJModules.Panner
 - RJModules.Panners
 - RJModules.BPM
 - RJModules.Button
 - RJModules.Buttons
 - RJModules.Splitter
 - RJModules.Splitters
 - RJModules.Displays
 - RJModules.Range
 - RJModules.ThreeXOSC
 - RJModules.BPF
 - RJModules.Buffers
 - RJModules.Chord
 - RJModules.ChordSeq
 - RJModules.Glides
 - RJModules.MetaKnob
 - RJModules.Octaves
 - RJModules.RandomFilter
 - RJModules.Riser
 - SerialRacker.MidiMultiplexer
 - Skylights.whatnote
 - Skylights.turing
 - Skylights.turing_volts
 - Skylights.turing_pulse
 - Skylights.turing_vactrol
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
 - Southpole.Abr
 - Southpole.Annuli
 - Southpole.Aux
 - Southpole.Balaclava
 - Southpole.Bandana
 - Southpole.Blank1HP
 - Southpole.Blank2HP
 - Southpole.Blank4HP
 - Southpole.Blank8HP
 - Southpole.Blank16HP
 - Southpole.Blank42HP
 - Southpole.But
 - Southpole.CornrowsX
 - Southpole.DeuxEtageres
 - Southpole.Etagere
 - Southpole.Falls
 - Southpole.Ftagn
 - Southpole.Fuse
 - Southpole.Gnome
 - Southpole.Piste
 - Southpole.Pulse
 - Southpole.Rakes
 - Southpole.Riemann
 - Southpole.Smoke
 - Southpole.Snake
 - Southpole.Sns
 - Southpole.Splash
 - Southpole.Sssh
 - Southpole.Wriggle
 - Southpole-parasites.Annuli
 - Southpole-parasites.Splash
 - squinkylabs-plug1.Booty
 - squinkylabs-plug1.CHB
 - squinkylabs-plug1.ColoredNoise
 - squinkylabs-plug1.CPU_Hog
 - squinkylabs-plug1.DG
 - squinkylabs-plug1.EV
 - squinkylabs-plug1.EV3
 - squinkylabs-plug1.FunV (crash)
 - squinkylabs-plug1.GMR
 - squinkylabs-plug1.Gray
 - squinkylabs-plug1.LFN
 - squinkylabs-plug1.Shaper (crash)
 - squinkylabs-plug1.Super
 - squinkylabs-plug1.ThreadBoost
 - squinkylabs-plug1.Tremolo
 - squinkylabs-plug1.Vocal
 - squinkylabs-plug1.VocalFilter
 - squinkylabs-plug1.CHBg
 - squinkylabs-plug1.KS
 - squinkylabs-plug1.Sequencer
 - SubmarineFree.AG104
 - SubmarineFree.AG106
 - SubmarineFree.AO106
 - SubmarineFree.AO112
 - SubmarineFree.AO118
 - SubmarineFree.AO124
 - SubmarineFree.AO136
 - SubmarineFree.BB120
 - SubmarineFree.EO102
 - SubmarineFree.FF110
 - SubmarineFree.FF120
 - SubmarineFree.FF212
 - SubmarineFree.LA108
 - SubmarineFree.LD103
 - SubmarineFree.LD106
 - SubmarineFree.NG106
 - SubmarineFree.NG112
 - SubmarineFree.OG104
 - SubmarineFree.OG106
 - SubmarineFree.PG104
 - SubmarineFree.PG112
 - SubmarineFree.PO101
 - SubmarineFree.PO102
 - SubmarineFree.PO204
 - SubmarineFree.SS112
 - SubmarineFree.SS208
 - SubmarineFree.SS212
 - SubmarineFree.SS220
 - SubmarineFree.SS221
 - SubmarineFree.TD116
 - SubmarineFree.TD202
 - SubmarineFree.TF101
 - SubmarineFree.TM105
 - SubmarineFree.WK101
 - SubmarineFree.WK205
 - SubmarineFree.XF101
 - SubmarineFree.XF102
 - SubmarineFree.XF104
 - SubmarineFree.XF201
 - SubmarineFree.XF202
 - SubmarineFree.XG104
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
 - SynthKit.Addition
 - SynthKit.Subtraction
 - SynthKit.And
 - SynthKit.Or
 - SynthKit.M1x8
 - SynthKit.M1x8CV
 - SynthKit.ClockDivider
 - SynthKit.RotatingClockDivider
 - SynthKit.RotatingClockDivider2
 - SynthKit.PrimeClockDivider
 - SynthKit.FibonacciClockDivider
 - SynthKit.Seq4
 - SynthKit.Seq8
 - Template.MyModule
 - TheXOR.Klee
 - TheXOR.M581
 - TheXOR.Z8K
 - TheXOR.Renato
 - TheXOR.Spiralone
 - TheXOR.Burst
 - TheXOR.Uncertain
 - TheXOR.PwmClock
 - TheXOR.Quantizer
 - TheXOR.Attenuator
 - TheXOR.Boole
 - TheXOR.Switch
 - TheXOR.Mplex
 - trowaSoft.TrigSeq
 - trowaSoft.TrigSeq64
 - trowaSoft.VoltSeq
 - trowaSoft.OscCV
 - trowaSoft.MultiScope
 - trowaSoft.MultiOscillator
 - unless_modules.Piong
 - unless_modules.Markov
 - Valley.Topograph
 - Valley.UGraph
 - Valley.Dexter
 - Valley.Plateau
 - Valley.Amalgam
 - Valley.Interzone


# License
All additional source code added by me is placed under the [MIT](https://en.wikipedia.org/wiki/MIT_License) license.


# How to build (Windows)

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

Edit `dep/yac/install_msvc.mk` and adjust the `LIB_INSTALL_PREFIX`, `WINDDK_PATH`, `VCTK`, `W32API_INC`, `W32API_LIB` as required.

EDIT `setenv_msvc.sh` and adjust the `VST2_SDK_DIR` as required.

```
$ alias m="make -j 20 makefile.msvc"
$ . setenv_msvc.sh
$ m all
```
If the build succeeded, the effect and instrument plugin DLLs can now be found in the `vst2_bin/` folder.

Last but not least, please don't ask me for the VST2 SDK.
It is not permitted to redistribute it and Steinberg has discontinued it.
I heard that the `aeffect.h` / `aeffectx.h` files are still included in the VST3 SDK.

## Dynamically loaded plugins
```
$ m clean
$ m shared_lib
```
(creates `plugins/Rack_shared.lib`)

```
$ cd plugins/community/repos/<yourplugin>
$ m bin
$ mv <yourplugin.dll> ../../../../vst2_bin/plugins/<yourpluginname>/plugin.dll
```
(and don't forget to copy the `res/` directory to `vst2_bin/plugins/`!)


## Dynamically loaded plugins (via plugin SDK)

1. Install the `Microsoft Visual Studio 2017 Community Edition` IDE
2. Download the [VeeSeeVSTRack plugin SDK](https://github.com/bsp2/releases/raw/master/vsvr/VeeSeeVSTRack_SDK-31Oct2018.7z)
3. Open the solution file (`example\Template_shared\vs2017\Template_shared\Template_shared.sln`)
4. Make sure that the `Release` / `x64` configuration is selected
5. Rebuild the solution to create the "plugin.dll" file.


# How to build (Linux)

Prerequisites:
- Standard GNU build environment (bash, make, GCC/G++)
- GTK+ headers+libs
- OpenGL headers+libs
- Steinberg VST2.4 SDK

```
$ git clone https://github.com/bsp2/VeeSeeVSTRack.git
```
```
$ cd VeeSeeVSTRack/
```

EDIT `setenv_linux.sh` and adjust the `VST2_SDK_DIR` as required.

```
$ . setenv_linux.sh
$ alias m="make -j 20 makefile.linux"
$ m all
```


# VCV Rack

For more info about VCV rack, see https://vcvrack.com/

# Support

Keep in mind that this is NOT AN OFFICIAL VCV RACK RELEASE.
Please DO NOT contact the VCV Rack team if you need any support.
You may get some support at https://www.kvraudio.com/forum/viewtopic.php?f=23&t=507216

~bsp
