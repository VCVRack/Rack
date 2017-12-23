Tip: Use `git checkout v0.4.0` for example to check out any previous version mentioned here.


### v0.5.1 (2017-12-19)

- Added Plugin Manager support
- Fixed metadata panel in the Add Module window

- Fundamental
	- Added Sequential Switch 1 & 2


### v0.5.0 (2017-11-21)

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


### v0.4.0 (2017-10-13)

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


### v0.3.2 (2017-09-25)

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


### v0.3.1 (2017-09-13)

- Fixed Windows open dialog current working directory graphics problem
- Ctrl/Cmd-C/V to copy/paste from text and password fields
- Automatically remembers settings and login token between launches
- Removes .zip after downloading and extracting plugin
- Ctrl-click to right click on Mac
- Fixed menu "flicker" when menu cannot fit in window
- tweaks to Fundamental and Audible Instruments plugins


### v0.3.0 (2017-09-10)

- Knobcon public Beta release
