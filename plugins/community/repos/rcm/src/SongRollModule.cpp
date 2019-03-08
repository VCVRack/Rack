#include "GVerbWidget.hpp"

#include "../include/SongRoll/SongRollModule.hpp"
#include "../include/SongRoll/SongRollWidget.hpp"

using namespace rack_plugin_rcm;
using namespace SongRoll;

RACK_PLUGIN_MODEL_INIT(rcm, SongRollModule) {
   Model *modelSongRollModule = Model::create<SongRollModule, SongRollWidget>("rcm", "rcm-songroll", "Song Roll", SEQUENCER_TAG);
   return modelSongRollModule;
}
