#include "../rcm.h"
#include "../../include/SongRoll/SongRollModule.hpp"

using namespace rack;

namespace rack_plugin_rcm {

   namespace SongRoll {

SongRollModule::SongRollModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS), runInputActive(false), transport(&songRollData) {
}

void SongRollModule::onReset() {
  transport.reset();
  songRollData.reset();
}

json_t *SongRollModule::toJson() {
  json_t *rootJ = Module::toJson();
  if (rootJ == NULL) {
      rootJ = json_object();
  }

  return rootJ;
}

void SongRollModule::fromJson(json_t *rootJ) {
  Module::fromJson(rootJ);

}

void SongRollModule::step() {
}

   }

} // namespace rack_plugin_rcm
