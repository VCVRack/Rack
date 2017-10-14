#include "core.hpp"
#include "MidiInterface.hpp"


void init(rack::Plugin *plugin) {
	plugin->slug = "Core";
	plugin->name = "Core";
	plugin->homepageUrl = "https://vcvrack.com/";
	createModel<AudioInterfaceWidget>(plugin, "AudioInterface", "Audio Interface");
	createModel<MidiToCVWidget>(plugin, "MIDIToCVInterface", "MIDI-to-CV Interface");
	createModel<MIDICCToCVWidget>(plugin, "MIDICCToCVInterface", "MIDI CC-to-CV Interface");
	createModel<MIDIClockToCVWidget>(plugin, "MIDIClockToCVInterface", "MIDI Clock-to-CV Interface");
	createModel<MIDITriggerToCVWidget>(plugin, "MIDITriggerToCVInterface", "MIDI Trigger-to-CV Interface");
	// createModel<BridgeWidget>(plugin, "Bridge", "Bridge");
	createModel<BlankWidget>(plugin, "Blank", "Blank");
}
