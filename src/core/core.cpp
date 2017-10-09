#include "core.hpp"


void init(rack::Plugin *plugin) {
	plugin->slug = "Core";
	plugin->name = "Core";
	plugin->homepageUrl = "https://vcvrack.com/";
	createModel<AudioInterfaceWidget>(plugin, "AudioInterface", "Audio Interface");
	createModel<MidiInterfaceWidget>(plugin, "MidiInterface", "MIDI Interface");
	// createModel<BridgeWidget>(plugin, "Bridge", "Bridge");
	createModel<BlankWidget>(plugin, "Blank", "Blank");
}
