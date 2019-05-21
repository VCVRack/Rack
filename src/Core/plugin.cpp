#include "plugin.hpp"


void init(rack::Plugin *p) {
	p->slug = "Core";
	p->version = TOSTRING(VERSION);
	p->license = "BSD-3-Clause";
	p->name = "Core";
	p->brand = "Core";
	p->author = "VCV";
	p->authorEmail = "contact@vcvrack.com";
	p->authorUrl = "https://vcvrack.com/";
	p->pluginUrl = "https://vcvrack.com/";
	p->manualUrl = "https://vcvrack.com/manual/Core.html";
	p->sourceUrl = "https://github.com/VCVRack/Rack";

	modelAudioInterface->name = "Audio 8";
	modelAudioInterface->description = "Sends audio and CV to/from an audio device";
	modelAudioInterface->tags = {"External"};
	p->addModel(modelAudioInterface);

	modelAudioInterface16->name = "Audio 16";
	modelAudioInterface16->description = "Sends audio and CV to/from an audio device";
	modelAudioInterface16->tags = {"External"};
	p->addModel(modelAudioInterface16);

	modelMIDI_CV->name = "MIDI-CV";
	modelMIDI_CV->description = "Converts MIDI from an external device to CV and gates";
	modelMIDI_CV->tags = {"External", "MIDI"};
	p->addModel(modelMIDI_CV);

	modelMIDI_CC->name = "MIDI-CC";
	modelMIDI_CC->description = "Converts MIDI CC from an external device to CV";
	modelMIDI_CC->tags = {"External", "MIDI"};
	p->addModel(modelMIDI_CC);

	modelMIDI_Gate->name = "MIDI-Gate";
	modelMIDI_Gate->description = "Converts MIDI notes from an external device to gates";
	modelMIDI_Gate->tags = {"External", "MIDI"};
	p->addModel(modelMIDI_Gate);

	modelMIDI_Map->name = "MIDI-Map";
	modelMIDI_Map->description = "";
	modelMIDI_Map->tags = {"External", "MIDI"};
	p->addModel(modelMIDI_Map);

	modelCV_MIDI->name = "CV-MIDI";
	modelCV_MIDI->description = "Converts CV to MIDI and sends to an external device";
	modelCV_MIDI->tags = {"External", "MIDI"};
	p->addModel(modelCV_MIDI);

	modelCV_CC->name = "CV-CC";
	modelCV_CC->description = "Converts CV to MIDI CC and sends to an external device";
	modelCV_CC->tags = {"External", "MIDI"};
	p->addModel(modelCV_CC);

	modelCV_Gate->name = "CV-Gate";
	modelCV_Gate->description = "Converts gates to MIDI notes and sends to an external device";
	modelCV_Gate->tags = {"External", "MIDI"};
	p->addModel(modelCV_Gate);

	modelBlank->name = "Blank";
	modelBlank->description = "A resizable blank panel";
	modelBlank->tags = {"Blank"};
	p->addModel(modelBlank);

	modelNotes->name = "Notes";
	modelNotes->description = "Write text for patch notes or artist attribution";
	modelNotes->tags = {"Blank"};
	p->addModel(modelNotes);
}
