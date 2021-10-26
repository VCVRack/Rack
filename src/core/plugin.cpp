#include "plugin.hpp"


namespace rack {
namespace core {


void init(rack::Plugin* p) {
	p->addModel(modelAudio2);
	p->addModel(modelAudio8);
	p->addModel(modelAudio16);
	p->addModel(modelMIDI_CV);
	p->addModel(modelMIDICC_CV);
	p->addModel(modelMIDI_Gate);
	p->addModel(modelMIDIMap);
	p->addModel(modelCV_MIDI);
	p->addModel(modelCV_MIDICC);
	p->addModel(modelGate_MIDI);
	p->addModel(modelBlank);
	p->addModel(modelNotes);
}


} // namespace core
} // namespace rack
