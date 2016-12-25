#include "core.hpp"


using namespace rack;

Plugin *coreInit() {
	audioInit();
	midiInit();

	Plugin *plugin = createPlugin("Core", "Core");
	createModel<AudioInterfaceWidget>(plugin, "AudioInterface", "Audio Interface");
	createModel<MidiInterfaceWidget>(plugin, "MidiInterface", "MIDI Interface");
	return plugin;
}
