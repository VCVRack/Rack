#include "core.hpp"


void init(rack::Plugin *p) {
	p->slug = "Core";
	p->addModel(createModel<AudioInterfaceWidget>("Core", "Core", "AudioInterface", "Audio Interface", EXTERNAL_TAG));
	p->addModel(createModel<MidiToCVWidget>("Core", "Core", "MIDIToCVInterface", "MIDI-to-CV Interface", MIDI_TAG, EXTERNAL_TAG));

	p->addModel(createModel<MIDICCToCVWidget>("Core", "Core", "MIDICCToCVInterface", "MIDI CC-to-CV Interface", MIDI_TAG, EXTERNAL_TAG));
	p->addModel(createModel<MIDIClockToCVWidget>("Core", "Core", "MIDIClockToCVInterface", "MIDI Clock-to-CV Interface", MIDI_TAG, EXTERNAL_TAG, CLOCK_TAG));
	p->addModel(createModel<MIDITriggerToCVWidget>("Core", "Core", "MIDITriggerToCVInterface", "MIDI Trigger-to-CV Interface", MIDI_TAG, EXTERNAL_TAG));
	p->addModel(createModel<QuadMidiToCVWidget>("Core", "Core", "QuadMIDIToCVInterface", "Quad MIDI-to-CV Interface", MIDI_TAG, EXTERNAL_TAG, QUAD_TAG));

	// p->addModel(createModel<BridgeWidget>("Core", "Core", "Bridge", "Bridge"));
	p->addModel(createModel<BlankWidget>("Core", "Core", "Blank", "Blank", BLANK_TAG));
	p->addModel(createModel<NotesWidget>("Core", "Core", "Notes", "Notes", BLANK_TAG));
}
