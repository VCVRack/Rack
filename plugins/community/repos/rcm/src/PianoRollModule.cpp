#include "GVerbWidget.hpp"

#include "../include/PianoRoll/PianoRollModule.hpp"
#include "../include/PianoRoll/PianoRollWidget.hpp"

using namespace rack_plugin_rcm;

RACK_PLUGIN_MODEL_INIT(rcm, PianoRollModule) {
   Model *modelPianoRollModule = Model::create<PianoRollModule, PianoRollWidget>("rcm", "rcm-pianoroll", "Piano Roll", SEQUENCER_TAG);
   return modelPianoRollModule;
}
