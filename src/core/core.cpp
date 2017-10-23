#include "core.hpp"
#include "MidiIO.hpp"


void init(rack::Plugin *p) {
	p->slug = "Core";
	p->addModel(createModel<AudioInterfaceWidget>("Core", "Core", "AudioInterface", "Audio Interface"));
	p->addModel(createModel<MidiToCVWidget>("Core", "Core", "MIDIToCVInterface", "MIDI-to-CV Interface"));
	p->addModel(createModel<MIDICCToCVWidget>("Core", "Core", "MIDICCToCVInterface", "MIDI CC-to-CV Interface"));
	p->addModel(createModel<MIDIClockToCVWidget>("Core", "Core", "MIDIClockToCVInterface", "MIDI Clock-to-CV Interface"));
	p->addModel(createModel<MIDITriggerToCVWidget>("Core", "Core", "MIDITriggerToCVInterface", "MIDI Trigger-to-CV Interface"));
	// p->addModel(createModel<BridgeWidget>("Core", "Core", "Bridge", "Bridge"));
	p->addModel(createModel<BlankWidget>("Core", "Core", "Blank", "Blank"));
	p->addModel(createModel<NotesWidget>("Core", "Core", "Notes", "Notes"));
}
