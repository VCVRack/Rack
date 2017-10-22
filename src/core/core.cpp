#include "core.hpp"
#include "MidiIO.hpp"


void init(rack::Manufacturer *m) {
	m->slug = "Core";
	m->name = "Core";
	m->homepageUrl = "https://vcvrack.com/";
	m->addModel(createModel<AudioInterfaceWidget>("AudioInterface", "Audio Interface"));
	m->addModel(createModel<MidiToCVWidget>("MIDIToCVInterface", "MIDI-to-CV Interface"));
	m->addModel(createModel<MIDICCToCVWidget>("MIDICCToCVInterface", "MIDI CC-to-CV Interface"));
	m->addModel(createModel<MIDIClockToCVWidget>("MIDIClockToCVInterface", "MIDI Clock-to-CV Interface"));
	m->addModel(createModel<MIDITriggerToCVWidget>("MIDITriggerToCVInterface", "MIDI Trigger-to-CV Interface"));
	// m->addModel(createModel<BridgeWidget>("Bridge", "Bridge"));
	m->addModel(createModel<BlankWidget>("Blank", "Blank"));
	m->addModel(createModel<NotesWidget>("Notes", "Notes"));
}
