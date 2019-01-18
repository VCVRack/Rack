#include "Core.hpp"


void init(rack::Plugin *p) {
	p->slug = "Core";
	p->version = TOSTRING(VERSION);
	p->name = "Core";
	p->author = "VCV";
	p->license = "LGPL-3.0-only and BSD-3-Clause";
	p->authorEmail = "contact@vcvrack.com";
	p->pluginUrl = "https://vcvrack.com/";
	p->authorUrl = "https://vcvrack.com/";
	p->manualUrl = "https://vcvrack.com/manual/Core.html";
	p->sourceUrl = "https://github.com/VCVRack/Rack";

	modelAudioInterface->name = "Audio";
	modelAudioInterface->description = "";
	modelAudioInterface->tags = {"External"};
	p->addModel(modelAudioInterface);

	modelMIDIToCVInterface->name = "MIDI-1";
	modelMIDIToCVInterface->description = "";
	modelMIDIToCVInterface->tags = {"External", "MIDI"};
	p->addModel(modelMIDIToCVInterface);

	modelQuadMIDIToCVInterface->name = "MIDI-4";
	modelQuadMIDIToCVInterface->description = "";
	modelQuadMIDIToCVInterface->tags = {"External", "MIDI", "Quad"};
	p->addModel(modelQuadMIDIToCVInterface);

	modelMIDICCToCVInterface->name = "MIDI-CC";
	modelMIDICCToCVInterface->description = "";
	modelMIDICCToCVInterface->tags = {"External", "MIDI"};
	p->addModel(modelMIDICCToCVInterface);

	modelMIDITriggerToCVInterface->name = "MIDI-Trig";
	modelMIDITriggerToCVInterface->description = "";
	modelMIDITriggerToCVInterface->tags = {"External", "MIDI"};
	p->addModel(modelMIDITriggerToCVInterface);

	modelCV_MIDI->name = "CV-MIDI";
	modelCV_MIDI->description = "";
	modelCV_MIDI->tags = {"External", "MIDI"};
	p->addModel(modelCV_MIDI);

	modelBlank->name = "Blank";
	modelBlank->description = "";
	modelBlank->tags = {"Blank"};
	p->addModel(modelBlank);

	modelNotes->name = "Notes";
	modelNotes->description = "";
	modelNotes->tags = {"Blank"};
	p->addModel(modelNotes);
}
