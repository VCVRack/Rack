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
	modelAudioInterface->description = "Sends audio and CV to/from an audio device";
	modelAudioInterface->tags = {"External"};
	p->addModel(modelAudioInterface);

	modelMIDI_CV->name = "MIDI-CV";
	modelMIDI_CV->description = "";
	modelMIDI_CV->tags = {"External", "MIDI"};
	p->addModel(modelMIDI_CV);

	modelMIDI_CC->name = "MIDI-CC";
	modelMIDI_CC->description = "";
	modelMIDI_CC->tags = {"External", "MIDI"};
	p->addModel(modelMIDI_CC);

	modelMIDI_Gate->name = "MIDI-Gate";
	modelMIDI_Gate->description = "";
	modelMIDI_Gate->tags = {"External", "MIDI"};
	p->addModel(modelMIDI_Gate);

	modelCV_MIDI->name = "CV-MIDI";
	modelCV_MIDI->description = "";
	modelCV_MIDI->tags = {"External", "MIDI"};
	p->addModel(modelCV_MIDI);

	modelCV_CC->name = "CV-CC";
	modelCV_CC->description = "";
	modelCV_CC->tags = {"External", "MIDI"};
	p->addModel(modelCV_CC);

	modelCV_Gate->name = "CV-Gate";
	modelCV_Gate->description = "";
	modelCV_Gate->tags = {"External", "MIDI"};
	p->addModel(modelCV_Gate);

	modelBlank->name = "Blank";
	modelBlank->description = "";
	modelBlank->tags = {"Blank"};
	p->addModel(modelBlank);

	modelNotes->name = "Notes";
	modelNotes->description = "";
	modelNotes->tags = {"Blank"};
	p->addModel(modelNotes);
}
