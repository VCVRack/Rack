# VCV Rack changelog

In this document, Ctrl means Cmd on Mac.

### 2.3.0 (2023-03-08)
- Add "View > Themes" menu with new UI themes: light and high-contrast dark.
- Fix file permissions resulting in error when loading certain patches.
- Don't select modules on click when module positions are locked.
- Fix small memory leak when pasting modules or selections.
- Fix incorrect panel scale when moving window between screens with different scale.
- Rack Pro
	- Don't force MIDI output message channel to 1 in VST3 adapter. Offer all 16 channels.
	- Fix aftertouch and polyphonic pressure on all MIDI channels in VST3 and CLAP adapters.
- API
	- Make `ParamQuantity::set/getValue()` set/get the Param's target value of the Engine's per-sample smoothing algorithm instead of the Param's immediate value. Add `ParamQuantity::set/getImmediateValue()`. Deprecate `ParamQuantity::set/getSmoothValue()`.
	- Add `dsp::polyDirect()`, `dsp::polyHorner()`, and `dsp::polyEstrin()`.
	- Rename `dsp::approxExp2_taylor5()` to `dsp::exp2_taylor5()` and improve polynomial coefficients.
	- Add `color::lerp()`.
	- Add `BooleanTrigger::processEvent()` and `SchmittTrigger::processEvent()`.
	- Add `get()` helper function for `std::vector`.

### 2.2.3 (2023-01-25)
- Place module selection nearest to mouse position when pasted and nearest to the center of the rack viewport when imported.
- Allow custom menu items to be appended to port's context menu.
- Fix ignored MIDI input messages while using small audio block sizes.
- Rack Pro
	- Fix hang when dialog box opens on Windows, such as after syncing library plugins.
	- Enable Loopback MIDI driver in all plugin adapters.
	- Fix MIDI CC output of VST3 adapter.
	- Fix Windows installer not overwriting existing VST3 plugin bundle on Windows.
	- Store and recall window size of VST3.
	- Fix MIDI clock input in CLAP adapter.
	- Make CLAP adapter a Note Effect and Audio Effect as well as an Instrument.
- API
	- Make unarchiver handle zero-byte files as a special case by deleting destination files instead of overwriting them. This allows plugin packages to remove old presets by including a zero-byte file with its filename.
	- Add `ModuleWidget::getModule<TModule>()` convenience method.

### 2.2.2 (2022-12-27)
- Display Rack edition, version, OS, CPU, and plugin type in menu bar to help with troubleshooting.
- Add long-form command line options.
- Zero audio output of all channels in `audio::Device::processBuffer()` before writing, to avoid sending uninitialized values to audio device.
- Rack Pro
	- Fix blank plugin window on certain Linux Nvidia graphics drivers.
- API
	- Don't include SIMDE headers on x64, fixing symbol conflicts when plugins include x64 intrinsic headers.
	- Don't export symbols from libarchive, zstd, rtaudio, and rtmidi to avoid conflicts with hosts that use these libraries. Rack plugins can no longer link to these libraries.
	- Rename plugin binary to `plugin-arm64.dylib` on Mac ARM64 so multiple plugin architectures can coexist in the same Rack user folder.

### 2.2.1 (2022-12-07)
- Add `CROSS_COMPILE` environment variable to specify target triplet for building plugins for non-native architectures.
- Accept `aarch64` in target triplet as alias for ARM64.
- Rack Pro
	- Re-enable rtaudio and rtmidi in plugin adapters, so hardware audio/MIDI devices can be used directly by Rack.
	- Fix VST3 not receiving MIDI CC input from DAWs.
	- Fix VST3 not sending MIDI output to some DAWs.
	- Fix VST3 in Bitwig resetting parameters when adding/removing modules.
	- Fix VST3 adapter not loading on Bitwig Linux and possibly other hosts, by providing VST3 as a folder bundle instead of a single file.
	- Fix CLAP adapter not loading on Windows, by statically linking libgcc.

### 2.2.0 (2022-11-23)
- Add MIDI Loopback driver, allowing modules with MIDI ports to send MIDI messages to other modules.
- Improve fuzzy search in module browser.
- Allow building on ARM64 CPUs.
- Rename plugin packages to `SLUG-VERSION-OS-CPU.vcvplugin`.
- Rack Pro
	- Add VST3, Audio Unit, and CLAP plugin adapters.
	- Add framerate setting to plugins.
- API
	- Add `system::sleep()`.
	- Make `random::get()`, `uniform()`, etc use global random state instead of thread-local.

### 2.1.2 (2022-07-04)
- Add old module moving behavior (nearest and force-moving), available by disabling "View > Auto-squeeze algorithm".
- Reorganized View menu.
- Add version comparator so Rack only updates plugins or itself if the remote version is "greater" than the local version, as defined by the `string::Version` documentation.
- Add file association to Mac, so double-clicking a `.vcv` patch file in Finder opens Rack and loads the patch.
- Fix expanders not updating (connecting or disconnecting to adjacent modules) when cloning or removing modules.
- Fix VCV Audio 2 VU meter light thresholds not matching label.

### 2.1.1 (2022-05-21)
- Allow changing cable colors with port menu.
- Fix placement bugs and improve behavior when moving or duplicating modules.
- Fix probabilistic crash when undoing a module paste action.
- Rack Pro
	- Fix VST2 window size not being remembered on Mac.
- API
	- Make `SvgButton` dispatch `ActionEvent` only on left mouse down, instead of left/right mouse down and drag drop.

### 2.1.0 (2022-02-26)
- Change behavior of force-moving modules so that other modules return to original position while dragging.
- Update to RtMidi 5.0.0.
- Update to RtAudio 5.2.0.
- Update GLFW.
- Fix plug graphic sometimes being incorrectly rotated.
- Core
	- Add "Pitch bend range" setting to MIDI to CV module.
	- Fix MIDI to CV incorrectly setting pitch wheel and mod wheel outputs in MPE mode.
- Rack Pro
	- Fix crash if generating hundreds of MIDI output messages per process block in VST2.

### 2.0.6 (2022-01-15)
- Add "Save a copy" to File menu.
- Remember CPU meter state across launches.
- Use audio device's suggested sample rate as initial sample rate.
- Add more logging of initialization/destruction of subsystems and module instantiation.
- Make MIDI input queue thread-safe, fixing probabilistic crash when processing MIDI input.
- Rack Pro
	- Generate MIDI Start message in DAWs like Cubase that pre-roll transport.
	- Generate MIDI Song Position Pointer messages when DAW is playing.
	- Fix VST2 input/output channel labels being truncated in Ableton Live.
	- Improve VST2 audio performance by avoiding unnecessary copying of buffers.

### 2.0.5 (2022-01-01)
- Swap order of parameter name and module name in MIDI-Map.
- Parse all note names from c0 to b9 and accidentals like c# and eb in parameter entry field.
- Tweak framebuffer render-skipping algorithm to always render at least 1 framebuffer after the frame deadline, to prevent framebuffers from never being rendered.
- Set audio device default sample rate to 44100, and block size to 256 except DirectSound to 1024.
- Fix file dialog truncating Unicode filenames on Mac.

### 2.0.4 (2021-12-18)
- Fix hang when initializing Audio module.
- Fix hidden window after closing while minimized and reopening.
- Move Import Selection menu item to File menu bar.
- Switch from GTK3 to `zenity` for opening dialogs on Linux.
- Implement prompt dialog on Windows.
- Make Windows installer add Rack to list of CFA allowed apps, allowing Rack to write to "My Documents" folder if Controlled Folder Access is enabled.
- Rack Pro
	- Improve stability of DAW MIDI clock.
	- Fix graphics glitch when duplicating module with Ctrl+D in Ableton Live.
	- Re-enable patch load error dialog.
- SDK
	- Compile with `-fno-omit-frame-pointer`.

### 2.0.3b (2021-12-09)
- Rack Pro
	- Use separate template patch when running as a plugin.

### 2.0.3 (2021-12-09)
- Fix MIDI-Map not accepting MIDI CC.
- Fix order of Audio-16 outputs 13/14 and 15/16.
- Clear patch before removing autosave dir when loading patch. This fixes inability to load patches on Windows when modules hold file handles to patch storage.
- Fix crash when patch attempts to add a cable that connects to a valid Port but an inexistent PortWidget (e.g. if a module defines a Port but omits the PortWidget).
- Upgrade from ustar to pax tar patch format. Don't store actual uid/gid, just set to 0.
- Rack Pro
	- Add external audio/MIDI drivers in plugin, except ASIO on Windows.
	- Show machine ID in splash window for offline activation.
	- Prevent "Reserved" plugin parameter from being changed when moving module parameters.

### 2.0.2 (2021-12-06)
- Turn off lights if module is bypassed.
- Fix Font and Image not loading UTF-8 filenames on Windows.
- Fix plugins not loading if their fallback alias exists.
- Fix crash when sometimes unsetting audio interface on Mac.
- Rack Pro
	- Save/restore plugin window size in patch.
	- Fix crash on scan in Renoise.

### 2.0.1 (2021-12-02)
- Fix network connection timeout.
- Flush log file when logging to avoid truncated logs.

### 2.0.0 (2021-11-30)
- Redesign Module Browser with compact layout, adjustable zoom levels, sorting options, intelligent searching, and multiple tag selection.
- Redesign component graphics and Core panels by [Pyer](https://www.pyer.be/).
- Add port tooltips with name, voltage, and list of connected ports.
- Add port context menu with ability to create cables with a particular color, drag any cable from the stack, or delete the top cable.
- Evaluate mathematical expressions (such as `1+2*3`) in parameter context menu fields.
- Add multiple module selection by highlighting a rectangle or Shift-clicking on a module panel.
- Add importing/exporting module selections to new `.vcvs` file format.
- Add module whitelist to Module Browser which synchronizes individual modules chosen in the VCV Library.
- Add favorite modules filter to Module Browser.
- Restructure engine to no longer use an "engine thread".
	- Improve engine performance and latency by no longer requiring thread synchronization between the engine thread and audio thread. The engine now runs directly on the audio thread.
	- Add support for multiple simultaneous audio devices.
	- Add "Master module" context menu item to VCV Audio modules to select which audio device clocks the engine.
	- Allow other modules to be the primary module, such as VCV Recorder for rendering audio faster than real-time.
	- Remove "Real-time priority" menu item, since the thread priority is now managed elsewhere (RtAudio, etc).
	- Remove engine pausing as it no longer makes sense with the new engine architecture.
- Replace module disabling with bypassing, which directly routes certain inputs to outputs if specified by the plugin.
- Duplicate cables patched to inputs when a module is duplicated.
- Add module tags to module context menu.
- Add module manual URL (if plugin developer supplies it) to module context menu item.
- Add quick access to user module patches from `<Rack user dir>/presets/<plugin slug>/<module slug>` to module context menu. Supports subdirectories.
- Add infinity and NaN protection to cables, so they won't propagate non-finite values from badly behaving modules.
- Add basic headless support with the `-h` flag.
- Add multiple parameter dragging modes: scaled linear, absolute rotary, and relative rotary.
- Add "knobLinearSensitivity" property to settings.
- Add timestamps to MIDI messages.
- Allow sending and receiving SysEx messages through MIDI drivers.
- Allow scrolling with Alt+click and drag.
- Add "File > Open recent" menu item for opening recent patches.
- Add "Preset > Save template" to module context menu which saves the default module preset to load when a new instance is added to the rack.
- Break Rack executable into libRack shared library and lightweight standalone Rack executable.
- Add support for 1/2x and 1/4x low-fidelity sample rates to engine and "Engine > Sample rates" menu.
- Add Escape key command for existing fullscreen, in case F11 doesn't work.
- Copy cable color when cloning cables with Ctrl+click.
- Fix key commands on AZERTY, Dvorak, and all other keyboard layouts.
- Add Mouse device to Computer keyboard/mouse MIDI driver.
- Make scrollbar mouse interaction similar to modern OS behavior.
- Re-render framebuffers when subpixel offset changes, fixing bug that makes ports and knobs appear slightly offset at certain zoom levels.
- Use new `.vcv` patch format, an archive (POSIX tar compressed with Zstandard) of a `patch.json` file, module patch assets, and potentially other future files.
- Use randomly-generated 53-bit IDs to identify modules and cables in the patch.
- Use a fuzzy search algorithm for searching modules in the Module Browser.
- Add tips window which appears when Rack launches or when choosing "Help > Tips".
- Check for Library updates every few seconds to avoid needing to restart Rack to check for updates.
- When clicking on a module in the Module Browser and immediately releasing, place the module in the last cursor position in the rack, rather than the current cursor position.
- When clicking and dragging a module from the Module Browser, fix the mouse handle offset to the center of the module.
- Allow darkening rack brightness.
- Draw spotlight near cursor when rack brightness is lowered.
- Improve light rendering with a more physical blending algorithm.
- Add engine CPU meter and framerate meter to menu bar.
- Allow zooming rack with extra mouse buttons 4 and 5.
- Add `"pixelRatio"` to settings for forcing the UI pixel scale.
- If Ctrl+clicking on any menu item, the menu stays open (except for some menu items like "Delete Module").
- Add Ctrl+F1 key command when cursor is hovering a module to open its user manual.
- Redesign CPU meter with percentage history graph.
- Add PulseAudio driver on Linux.
- Distribute .pkg installer on Mac.
- Check for VCV Library updates when Library menu is opened, rather than at time intervals.

- Core
	- Add Audio-2 module with stereo input/output, a level knob, and VU meters.
	- Add DC blocker setting to Audio modules. On Audio-2, enable it by default.
	- Add MPE mode to MIDI-CC and MIDI-Gate.
	- Add mode to MIDI-CC to process 14-bit MIDI CC via MSB/LSB.
	- Use MIDI timestamps in MIDI-CV, MIDI-CC, MIDI-Gate, and MIDI-Map to improve overall timing and drastically reduce clock jitter.
	- Add red clip lights to VCV Audio-8/16 when signal reaches beyond ±10V.
	- Reset notes in MIDI-CV and MIDI-Gate if an "all notes off" MIDI message is received.
	- Allow disabling smoothing for MIDI-CV (pitch and mod wheel), MIDI-CC, and MIDI-Map.
	- Add several module presets for many Core modules.

- API
	- Add setters/getters for nearly every instance variable in Rack's API. Use these for higher likelihood of stability.
	- Compile Rack and plugins with `-march=nehalem`, enabling (and requiring) up to SSE4.2 and POPCNT instruction sets.
	- Add `Module::configInput()` and `Module::configOutput()` for adding names to ports.
	- Replace `ParamWidget::paramQuantity` with `ParamWidget::getParamQuantity()`.
	- Add `.modules[].manualUrl` to plugin manifest schema.
	- Add `appendAudioMenu()` and `appendMidiMenu()` so plugin developers can develop custom audio/MIDI interfaces without adding an `AudioWidget/MidiWidget` to their panel.
	- Make `Module::toJson()` and `fromJson()` virtual.
	- Add `Module::paramsToJson()` and `paramsFromJson()` virtual methods.
	- Add `SwitchQuantity` and a helper method `Module::configSwitch()` for displaying named values in the parameter context menu. Also add `Module::configButton()` recommended for momentary switches with no value labels.
	- Overhaul Engine threading model to allow as many Engine methods to be called simultaneously as possible, while ensuring that Module events are mutually exclusive to module processing.
	- Add `Engine::getNumModules()` and `Engine::getModuleIds()`.
	- Add `event::KeyBase::keyName`. Plugins should use this instead of `key` for alphanumeric keys in order to support all keyboard layouts.
	- Improve thread safety of `dsp::RingBuffer`.
	- Add several convenient filesystem routines to `system::`.
	- Add `system::getTime()` and `getUnixTime()`.
	- Add `system::readFile()` and `writeFile()`.
	- Move all `string::` functions dealing with filesystem paths to `system::`.
	- Change type of `Module::id` and `Cable::id` from `int` to `int64_t`.
	- Move event classes to inside `widget::Widget` class.
	- Deprecate `Window::loadSvg()`. Un-deprecate `Svg::load()`.
	- `Font` and `Image` can no longer be stored across UI frames. Load them with `APP->window->loadFont()` and `loadImage()` each `draw()` method.
	- Add `Widget::hasChild()`, `addChildBottom()`, `addChildBelow()`, `addChildAbove()`, and `drawChild()`.
	- Add `Module::createPatchStorageDirectory()` and `getPatchStorageDirectory()`.
	- Add `createMenuLabel()`, `createMenuItem()`, `createCheckMenuItem()`, `createBoolMenuItem()`, `createBoolPtrMenuItem()`, `createSubmenuItem()`, `createIndexSubmenuItem()`, and `createIndexPtrSubmenuItem()` to helpers.
	- Add `Module::onReset()` and `onRandomize()`. Overrides default param resetting and randomization behavior if overridden, unless super methods are called.
	- Add `Module::SaveEvent`.
	- Add operator overloads for `Vec`.
	- Add `string::join()`, `split()`, `formatTime()`, and `formatTimeISO()`.
	- Add `random::Xoroshiro128Plus` which can be used like C++ `<random>` classes.
	- Add `Port::getVoltageRMS()`.
	- Add `Widget::drawLayer()` for handling multiple draw passes.
	- Add on/off threshold values to `dsp::SchmittTrigger`.
	- Add `dsp::convert()` template functions for converting audio between normalized floats and integers.
	- Add `bool app::SvgSwitch::latch` setting for latching button switches.
	- Dispatch `Module::SampleRateChange` event when `Module` is added to engine.

### 1.1.6 (2019-11-04)
- Add ability for plugins to use LuaJIT on Mac.
- Fix normal random number generator possibly returning -infinity.
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
- Fix crash when pasting presets to an empty rack space with Ctrl+V.
- Fix module expanders being assigned incorrectly when loading presets.
- Add check for duplicate port IDs for modules.

### 1.1.1 (2019-07-01)
- Add auto-updating on Windows when Help > Update VCV Rack is clicked.
- Fix extension filters in open/save dialog box on Windows.
- Fix expanders not attaching when force-dragging modules.

### 1.1.0 (2019-06-27)
- Request microphone access on Mac to prevent Mac 10.14+ from blocking audio input.
- Clear filters in Module Browser when backspace is pressed while the search field is empty.
- Add Ctrl+0 key command to set zoom to 100%.
- Emulate middle-click with Ctrl+shift-click on Mac.
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
- Add ability to Ctrl+click on an input port to clone the existing cable.
- Add module "force" dragging when holding Ctrl.
- Add ability to disable modules with a context menu item and key command Ctrl+E.
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
- Add key command Ctrl+`-` and Ctrl+`=`, or Ctrl+scroll, for zooming the rack.
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
- Ctrl+C/Ctrl+V to copy/paste from text and password fields.
- Automatically remember settings and login token between launches.
- Remove .zip after downloading and extracting plugin.
- Ctrl+click to right click on Mac.
- Fix menu "flicker" when menu cannot fit in window.
- tweaks to Fundamental and Audible Instruments plugins.

### 0.3.0 (2017-09-10)
- Knobcon public Beta release.
