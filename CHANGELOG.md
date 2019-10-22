#### Preface

In this document, Mod is Ctrl on Windows/Linux and Cmd on Mac.

### 1.1.6 (in development)
- Add ability for plugins to use LuaJIT on Mac.
- Core
	- MIDI-Map: Don't move param until the first MIDI CC command is sent.
- API
	- Remove support for namespaced `rack::APP`, `rack::DEBUG`, etc macros. Use namespace-less `APP`, `DEBUG`, etc instead.
	- Add `dsp::IIRFilter`.

### 1.1.5 (2019-09-29)
- Swap order of tags and brands in Module Browser.
- Add View > Frame rate menu bar item.
- Hide menu and scrollbars when fullscreen.
- Add key command (F3) for engine CPU meter.
- Add numpad key commands.
- Automatically unzip update on Mac.
- Stop worker threads when engine is paused to save CPU.
- Core
	- Disable smoothing for MIDI CC buttons in MIDI-Map.
	- Fix sustain pedal release bug when using polyphonic mode in MIDI-CV.
- API
	- Add libsamplerate library.

### 1.1.4 (2019-08-22)
- Fix parameter smoothing of MIDI-Map.
- Sort modules within plugin in the Module Browser according to plugin rather than alphabetically.
- Fix bug where knobs sometimes jump while dragging.
- Reimplement CPU meter to measure thread runtime, not real time.
- Fix crashes when deleting/duplicating modules while dragging modules/cables in certain cases.
- API
	- Add `dsp::BiquadFilter`.
	- Add `dsp/approx.hpp` with approximate math functions.
	- Add `simd::tan()`, `atan()`, and `atan2()`.
	- Add `string::toBase64()` and `fromBase64()`.

### 1.1.3 (2019-07-23)
- Include root certificate bundle for libcurl on all OS's.
- Revert to OpenSSL from Schannel on Windows.
- Bundle unsigned Fundamental package on Mac.

### 1.1.2 (2019-07-20)
- Add app notarization on Mac, which removes the "Apple cannot check for malicious software" message on launch.
- Write stack trace to log.txt and display dialog box when Rack crashes.
- Re-enable JACK MIDI driver on Linux.
- Fix scroll speed for mice and trackpads on Mac.
- Fix undo history bug when dragging patch file to the Rack window.
- Fix crash when pasting presets to an empty rack space with Mod-V.
- Fix module expanders being assigned incorrectly when loading presets.
- Add check for duplicate port IDs for modules.

### 1.1.1 (2019-07-01)
- Add auto-updating on Windows when Help > Update VCV Rack is clicked.
- Fix extension filters in open/save dialog box on Windows.
- Fix expanders not attaching when force-dragging modules.

### 1.1.0 (2019-06-27)
- Request microphone access on Mac to prevent Mac 10.14+ from blocking audio input.
- Clear filters in Module Browser when backspace is pressed while the search field is empty.
- Add Mod-0 key command to set zoom to 100%.
- Emulate middle-click with Ctrl-shift-click on Mac.
- Fix MIDI receiving in Bridge MIDI driver.
- Fix opening/saving UTF-8 filenames on Windows.
- Fix bug where cable ID's were not being set in .vcv patches.
- Plugin API
	- Add `string::absolutePath()`.
	- Use namespace for Core plugin to avoid name clashes.

### 1.0.0 (2019-06-19)
- Add polyphonic cables.
- Add multithreading to engine.
- Add undo/redo history.
- Add module expander support.
- Add parameter labels, units, and descriptions.
- Add parameter tooltips for quickly viewing parameter values.
- Add parameter context menu for entering numerical values, unmapping, etc.
- Change parameter initialization to double-click.
- Add ability to Mod-click on an input port to clone the existing cable.
- Add module "force" dragging when holding Mod.
- Add ability to disable modules with a context menu item and key command Mod-E.
- Add sample rates up to 768,000 Hz (16 x 48,000 Hz).
- Overhaul Module Browser with visual previews of modules.
- Add plugin info sub-menu to module context menu with links to manual, website, source code, etc.
- Add factory presets to module context menu if plugin supplies a folder of presets.
- Add default template patch.
- Add menu item to save the current patch as the template.
- Add "frameRateLimit" and "frameRateSync" for setting maximum screen refresh rate and to toggle vertical sync.
- Add "autosavePeriod" for setting the frequency of autosaves in seconds.
- Add textual menu bar, rearranged menu items, removed icons.
- Make CPU timer display microseconds and percentage instead of millisamples.
- Add engine real-time priority setting.
- Make rack infinite in all four directions.
- Add bus board graphic to rack.
- Add key command Mod-`-` and Mod-`=`, or Mod-scroll, for zooming the rack.
- Fix draw order of cable plugs and wires.
- Make Gamepad MIDI driver generate MIDI CC instead of MIDI notes for buttons.
- Add Numpad keyboard MIDI device.
- Fix Unicode user directories on Windows.
- Add ability to change cable colors in `settings.json`.
- Add `-p X` flag for dumping a screenshot of each available module.
- Allow user to see changelogs of plugins before updating their plugin library.
- Allow user to update individual plugins.
- Core
	- Add Audio-16 with 16/16 inputs/outputs.
	- Add CV-MIDI, CV-CC, and CV-Gate for sending MIDI to external devices.
	- Add MIDI-Map for mapping MIDI CC parameters directly to Rack parameters.
	- Add polyphony to MIDI-CV.
	- Add MPE mode to MIDI-CV.
	- Add "Panic" button to all MIDI modules to reset performance state.
- Plugin API
	- Add [`helper.py`](helper.py) for creating and manipulating plugins with the command-line.
	- Add [`simd.hpp`](include/dsp/simd.hpp) for generically handling arithmetic and math functions for vectors of floats, accelerated with SSE.
	- Add `dsp::VuMeter2`.
	- Add `dsp::Timer` and `dsp::Counter`.
	- Overhaul event system with many new events.
	- etc. See more at https://vcvrack.com/manual/Migrate1.html.
- Licenses
	- Relicense Rack to GPLv3 with the VCV Rack Non-Commercial Plugin License Exception and a commercial licensing option.
	- Collect all license statements into new [LICENSE.md](LICENSE.md) file.
	- License Core panel graphics under CC BY-NC-ND 4.0.

### 0.6.2 (2018-07-13)
- Add module presets.
- Add [command line arguments](https://vcvrack.com/manual/Installing.html#command-line-usage) for setting Rack directories.
- Improve UI/engine communication stability.
- [VCV Bridge](https://vcvrack.com/manual/Bridge.html) 0.6.2
	- Add VST FX plugin.

### 0.6.1 (2018-06-17)
- Add gamepad MIDI driver.
- Add computer keyboard MIDI driver.
- Add JACK support on Linux.
- Add velocity mode to MIDI-Trig.
- Add MIDI multiplexing so multiple MIDI modules can use the same MIDI device on Windows.
- Make Module Browser layout more compact.
- Add power meter.
- Add icons to toolbar.
- [VCV Bridge](https://vcvrack.com/manual/Bridge.html) 0.6.1
	- Replace VST effect plugin with VST instrument plugin with audio inputs.
	- Add MIDI support.
	- Add DAW transport and clock.

### 0.6.0 (2018-03-29)
- Release [*VCV Bridge*](https://vcvrack.com/manual/Bridge.html) for interfacing Rack with your DAW.
	- VST/AU effect plugins (Mac and 32/64-bit Windows) for using Rack as a send/return on a DAW track.
	- Enables future VSTi/AU instrument plugins with MIDI and DAW clock transport to be added in a later Rack 0.6.* update.
- Updated [*Plugin Manager*](https://vcvrack.com/plugins.html) to handle open-source plugins.
	- Potentially all plugins can be added with help from the [VCV Community](https://github.com/VCVRack/community/issues/248).
- New *Module Browser* for adding modules to the rack.
	- Launch by right-clicking on the rack or pressing <enter>.
	- Add "favorite" modules by clicking on the star button.
	- Navigate modules with arrow keys or mouse.
- Redesign [Core](https://vcvrack.com/manual/Core.html) modules.
	- Access to audio channels beyond the first 8 inputs/outputs.
	- Improve AUDIO stability.
	- Add retrigger output to MIDI-1.
	- Merged MIDI clock module with MIDI-1.
	- Fix MIDI-4 sustain pedal in polyphonic modes.
- Improve sample rate conversion performance, is disabled entirely when not needed.
- Patch cable colors are saved to patch files.
- Add highlighting for active patch cables when hovering mouse over port.
- Add shadows to knobs and ports.
- Add File > "Disconnect cables".
- Release [Rack SDK](https://github.com/VCVRack/Rack/issues/258#issuecomment-376293898) for compiling plugins without compiling Rack.

### 0.5.1 (2017-12-19)
- Add Plugin Manager support.
- Fix metadata panel in the Add Module window.

### 0.5.0 (2017-11-21)
- Add zoom scaling from 25% to 200%.
- Automatically scroll when dragging cables to the edge of the screen.
- Add Quad MIDI-to-CV Interface, CC-to-CV, Clock-to-CV, and Trigger-to-CV MIDI interfaces.
- Improve support for ASIO, WASAPI, DirectSound, Core Audio, and ALSA audio drivers.
- New module browser with search and tags.
- Enhanced LED emulation in graphics engine.
- File > New attempts to load "template.vcv" in the "Documents/Rack" folder if it exists.

### 0.4.0 (2017-10-13)
- Cables can now stack on output ports.
- Add sub-menus for each plugin, includes optional plugin metadata like URLs.
- Add MIDI CC-to-CV Interface, updated MIDI-to-CV Interface.
- Add new scrolling methods: middle-click-and-drag, shift-scroll, and arrow keys.
- Add engine pausing in sample rate menu.
- Add resizable blank to Core.
- Add LEDs on plugs.
- Support for AMD Phenom II processors.
- Use self-contained Mac app bundle, no need for a Rack folder.

### 0.3.2 (2017-09-25)
- Add key commands.
- Fix "invisible knobs/ports" rendering bug for ~2010 Macs.
- Add "allowCursorLock" to settings.json (set to "false" for touch screen support).
- Fix 100% CPU issue when no audio device is selected.
- Add vector scaling panels.

### 0.3.1 (2017-09-13)
- Fix Windows open dialog current working directory graphics problem.
- Mod-C/Mod-V to copy/paste from text and password fields.
- Automatically remember settings and login token between launches.
- Remove .zip after downloading and extracting plugin.
- Mod-click to right click on Mac.
- Fix menu "flicker" when menu cannot fit in window.
- tweaks to Fundamental and Audible Instruments plugins.

### 0.3.0 (2017-09-10)
- Knobcon public Beta release.
