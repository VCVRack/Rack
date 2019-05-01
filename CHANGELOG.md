
### 1.0.0 (in development)

- Added undo/redo history
- Added polyphonic cables
- Added multithreading to engine
- Added module expander support
- Added parameter labels, units, and descriptions
- Added parameter tooltips for quickly viewing parameter values
- Added parameter context menu for entering numerical values, unmapping, etc
- Changed parameter initialization to double-click
- Added ability to Ctrl-click on an input port to clone the existing cable
- Added module "force" dragging when holding Ctrl
- Added ability to disable modules with a context menu item and key command Ctrl-E
- Added sample rates up to 768,000 Hz (16 x 48,000 Hz)
- Overhauled Module Browser with visual previews of modules
- Added plugin info sub-menu to module context menu with links to manual, website, source code, etc.
- Added factory presets to module context menu if plugin supplies a folder of presets
- Added default template patch
- Added menu item to save the current patch as the template
- Added "frameRateLimit" and "frameRateSync" for setting maximum screen refresh rate and to toggle vertical sync
- Added textual menu bar, rearranged menu items, removed icons
- Made CPU timer display microseconds and percentage instead of millisamples
- Added engine real-time priority setting
- Made rack infinite in all four directions
- Added bus board graphic to rack
- Added key command Ctrl-- and Ctrl-=, or Ctrl-scroll, for zooming the rack
- Fixed draw order of cable plugs and wires
- Made Gamepad MIDI driver generate MIDI CC instead of MIDI notes for buttons
- Fixed Unicode user directories on Windows

- Core
	- Added Core CV-MIDI, CV-CC, and CV-Gate for sending MIDI to external devices
	- Added Core MIDI-Map for mapping MIDI CC parameters directly to Rack parameters
	- Added polyphony to Core MIDI-CV
	- Added MPE mode to Core MIDI-CV
	- Added "Panic" button to all MIDI modules to reset performance state

- API
	- Added [`simd.hpp`](include/dsp/simd.hpp) for generically handling arithmetic and math functions for vectors of floats, accelerated with SSE
	- Added `dsp::VuMeter2`
	- Added `dsp::Timer` and `dsp::Counter`
	- Overhauled event system with many new events
	- etc

- Licenses
	- Collected all license statements into new [LICENSE.md](LICENSE.md) file
	- Licensed Core panel graphics under CC BY-NC-ND 4.0

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

### 0.5.0 (2017-11-21)

- Added zoom scaling from 25% to 200%
- Automatically scroll when dragging cables to the edge of the screen
- Added Quad MIDI-to-CV Interface, CC-to-CV, Clock-to-CV, and Trigger-to-CV MIDI interfaces
- Improved support for ASIO, WASAPI, DirectSound, Core Audio, and ALSA audio drivers
- New module browser with search and tags
- Enhanced LED emulation in graphics engine
- File > New attempts to load "template.vcv" in the "Documents/Rack" folder if it exists

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

### 0.3.2 (2017-09-25)

- Added key commands
- Fixed "invisible knobs/ports" rendering bug for ~2010 Macs
- Added "allowCursorLock" to settings.json (set to "false" for touch screen support)
- Fixed 100% CPU issue when no audio device is selected
- Added vector scaling panels

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
