
### 0.6.2 (2018-07-13)

- Added module presets
- Added [command line arguments](https://vcvrack.com/manual/Installing.html#command-line-usage) for setting Rack directories
- Improved UI/engine communication stability
- [VCV Bridge](https://vcvrack.com/manual/Bridge.html) 0.6.2
	- Added VST FX plugin

### 0.6.1 (2018-06-17)

- Added gamepad MIDI driver
- Added computer keyboard MIDI driver
- Added JACK support on Linux
- Added velocity mode to MIDI-Trig
- Added MIDI multiplexing so multiple MIDI modules can use the same MIDI device on Windows
- Made Module Browser layout more compact
- Added power meter
- Added icons to toolbar
- [VCV Bridge](https://vcvrack.com/manual/Bridge.html) 0.6.1
	- Replaced VST effect plugin with VST instrument plugin with audio inputs
	- Added MIDI support
	- Added DAW transport and clock

### 0.6.0 (2018-03-29)

- Released [*VCV Bridge*](https://vcvrack.com/manual/Bridge.html) for interfacing Rack with your DAW
	- VST/AU effect plugins (Mac and 32/64-bit Windows) for using Rack as a send/return on a DAW track
	- Enables future VSTi/AU instrument plugins with MIDI and DAW clock transport to be added in a later Rack 0.6.* update
- Updated [*Plugin Manager*](https://vcvrack.com/plugins.html) to handle open-source plugins
	- Potentially all plugins can be added with help from the [VCV Community](https://github.com/VCVRack/community/issues/248)
- New *Module Browser* for adding modules to the rack
	- Launch by right-clicking on the rack or pressing <enter>
	- Add "favorite" modules by clicking on the star button
	- Navigate modules with arrow keys or mouse
- Redesigned [Core](https://vcvrack.com/manual/Core.html) modules
	- Access to audio channels beyond the first 8 inputs/outputs
	- Improved AUDIO stability
	- Added retrigger output to MIDI-1
	- Merged MIDI clock module with MIDI-1
	- Fixed MIDI-4 sustain pedal in polyphonic modes
- Improved sample rate conversion performance, is disabled entirely when not needed
- Patch cable colors are saved to patch files
- Added highlighting for active patch cables when hovering mouse over port
- Added shadows to knobs and ports
- Added File > "Disconnect cables"
- Released [Rack SDK](https://github.com/VCVRack/Rack/issues/258#issuecomment-376293898) for compiling plugins without compiling Rack


### 0.5.1 (2017-12-19)

- Added Plugin Manager support
- Fixed metadata panel in the Add Module window

- Fundamental
	- Added Sequential Switch 1 & 2


### 0.5.0 (2017-11-21)

- Added zoom scaling from 25% to 200%
- Automatically scroll when dragging cables to the edge of the screen
- Added Quad MIDI-to-CV Interface, CC-to-CV, Clock-to-CV, and Trigger-to-CV MIDI interfaces
- Improved support for ASIO, WASAPI, DirectSound, Core Audio, and ALSA audio drivers
- New module browser with search and tags
- Enhanced LED emulation in graphics engine
- File > New attempts to load "template.vcv" in the "Documents/Rack" folder if it exists

- New Grayscale plugin with Algorhythm, Binary, and Binary² modules

- Audible Instruments
	- Added extra blend mode functions, alternative modes, and quality settings to Texture Synthesizer
	- Added bonus modes and "Disastrous Peace" mode to Resonator
	- Added Low CPU mode to Macro Oscillator
	- Merged Tidal Modulator and Wavetable Oscillator into a single module
	- Fixed Keyframer/Mixer keyframes and channel settings saving

- Fundamental
	- Added 8vert, 8-channel attenuverter
	- Added Unity, 2-channel mixer
	- Changed LED functions in ADSR


### 0.4.0 (2017-10-13)

- Cables can now stack on output ports
- Added sub-menus for each plugin, includes optional plugin metadata like URLs
- Added MIDI CC-to-CV Interface, updated MIDI-to-CV Interface
- Added new scrolling methods: middle-click-and-drag, shift-scroll, and arrow keys
- Added engine pausing in sample rate menu
- Added resizable blank to Core
- Added LEDs on plugs
- Support for AMD Phenom II processors
- Use self-contained Mac app bundle, no need for a Rack folder

- Fundamental
	- Added Lissajous mode to Scope
	- Added two LFOs and VCO-2

- Befaco
	- Added Rampage

- Audible Instruments
	- Added Keyframer/Mixer


### 0.3.2 (2017-09-25)

- Added key commands
- Fixed "invisible knobs/ports" rendering bug for ~2010 Macs
- Added "allowCursorLock" to settings.json (set to "false" for touch screen support)
- Fixed 100% CPU issue when no audio device is selected
- Added vector scaling panels
- Audible Instruments:
	- Added alternative resonator models to Modal Synthesizer
- Fundamental:
	- Fixed Drive CV input of VCF
	- Reverted SEQ3 to continuous gates


### 0.3.1 (2017-09-13)

- Fixed Windows open dialog current working directory graphics problem
- Ctrl/Cmd-C/V to copy/paste from text and password fields
- Automatically remembers settings and login token between launches
- Removes .zip after downloading and extracting plugin
- Ctrl-click to right click on Mac
- Fixed menu "flicker" when menu cannot fit in window
- tweaks to Fundamental and Audible Instruments plugins


### 0.3.0 (2017-09-10)

- Knobcon public Beta release
