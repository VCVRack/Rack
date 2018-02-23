#include "core.hpp"


void init(rack::Plugin *p) {
	p->slug = "Core";
	p->version = TOSTRING(VERSION);

	p->addModel(modelAudioInterface);
	p->addModel(modelMIDIToCVInterface);
	p->addModel(modelMIDICCToCVInterface);
	p->addModel(modelBlank);
	p->addModel(modelNotes);

	// TODO
	// p->addModel(createModel<MIDICCToCVWidget>("Core", "MIDICCToCVInterface", "MIDI CC-to-CV Interface", MIDI_TAG, EXTERNAL_TAG));
	// p->addModel(createModel<MIDIClockToCVWidget>("Core", "MIDIClockToCVInterface", "MIDI Clock-to-CV Interface", MIDI_TAG, EXTERNAL_TAG, CLOCK_TAG));
	// p->addModel(createModel<MIDITriggerToCVWidget>("Core", "MIDITriggerToCVInterface", "MIDI Trigger-to-CV Interface", MIDI_TAG, EXTERNAL_TAG));
	// p->addModel(createModel<QuadMidiToCVWidget>("Core", "QuadMIDIToCVInterface", "Quad MIDI-to-CV Interface", MIDI_TAG, EXTERNAL_TAG, QUAD_TAG));
}
