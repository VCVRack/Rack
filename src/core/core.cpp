#include "core.hpp"


void init(rack::Plugin *plugin) {
	plugin->slug = "Core";
	plugin->name = "Core";
	createModel<AudioInterfaceWidget>(plugin, "AudioInterface", "Audio Interface");
	createModel<MidiInterfaceWidget>(plugin, "MidiInterface", "MIDI Interface");
}
