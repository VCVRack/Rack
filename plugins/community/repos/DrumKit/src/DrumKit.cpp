#include "DrumKit.hpp"

RACK_PLUGIN_MODEL_DECLARE(DrumKit, BD9);
RACK_PLUGIN_MODEL_DECLARE(DrumKit, Snare);
RACK_PLUGIN_MODEL_DECLARE(DrumKit, ClosedHH);
RACK_PLUGIN_MODEL_DECLARE(DrumKit, OpenHH);
RACK_PLUGIN_MODEL_DECLARE(DrumKit, DMX);

RACK_PLUGIN_INIT(DrumKit) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/JerrySievert/DrumKit");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/JerrySievert/DrumKit/blob/master/README.md");

   RACK_PLUGIN_MODEL_ADD(DrumKit, BD9);
   RACK_PLUGIN_MODEL_ADD(DrumKit, Snare);
   RACK_PLUGIN_MODEL_ADD(DrumKit, ClosedHH);
   RACK_PLUGIN_MODEL_ADD(DrumKit, OpenHH);
   RACK_PLUGIN_MODEL_ADD(DrumKit, DMX);
}
