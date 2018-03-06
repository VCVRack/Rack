#include "Core.hpp"


void init(rack::Plugin *p) {
	p->slug = "Core";
	p->version = TOSTRING(VERSION);

	p->addModel(modelAudioInterface);
	p->addModel(modelMIDIToCVInterface);
	p->addModel(modelQuadMIDIToCVInterface);
	p->addModel(modelMIDICCToCVInterface);
	p->addModel(modelMIDITriggerToCVInterface);
	p->addModel(modelBlank);
	p->addModel(modelNotes);
}
