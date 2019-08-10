#include "plugin.hpp"


namespace rack {
namespace core {


void init(rack::Plugin* p) {
	p->addModel(modelAudioInterface);
	p->addModel(modelAudioInterface16);
	p->addModel(modelMIDI_CV);
	p->addModel(modelMIDI_CC);
	p->addModel(modelMIDI_Gate);
	p->addModel(modelMIDI_Map);
	p->addModel(modelCV_MIDI);
	p->addModel(modelCV_CC);
	p->addModel(modelCV_Gate);
	p->addModel(modelBlank);
	p->addModel(modelNotes);
}


} // namespace core
} // namespace rack
