#include "core.hpp"


void init(rack::Plugin *p) {
	p->slug = "Core";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif

	p->addModel(createModel<AudioInterfaceWidget>("Core", "AudioInterface", "Audio Interface", EXTERNAL_TAG));

	// p->addModel(createModel<MidiToCVWidget>("Core", "MIDIToCVInterface", "MIDI-to-CV Interface", MIDI_TAG, EXTERNAL_TAG));
	// p->addModel(createModel<MIDICCToCVWidget>("Core", "MIDICCToCVInterface", "MIDI CC-to-CV Interface", MIDI_TAG, EXTERNAL_TAG));
	// p->addModel(createModel<MIDIClockToCVWidget>("Core", "MIDIClockToCVInterface", "MIDI Clock-to-CV Interface", MIDI_TAG, EXTERNAL_TAG, CLOCK_TAG));
	// p->addModel(createModel<MIDITriggerToCVWidget>("Core", "MIDITriggerToCVInterface", "MIDI Trigger-to-CV Interface", MIDI_TAG, EXTERNAL_TAG));
	// p->addModel(createModel<QuadMidiToCVWidget>("Core", "QuadMIDIToCVInterface", "Quad MIDI-to-CV Interface", MIDI_TAG, EXTERNAL_TAG, QUAD_TAG));

	// p->addModel(createModel<BridgeWidget>("Core", "Bridge", "Bridge"));
	p->addModel(createModel<BlankWidget>("Core", "Blank", "Blank", BLANK_TAG));
	p->addModel(createModel<NotesWidget>("Core", "Notes", "Notes", BLANK_TAG));
}
