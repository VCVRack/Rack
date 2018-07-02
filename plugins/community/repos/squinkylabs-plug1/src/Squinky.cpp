// plugin main
#include "Squinky.hpp"

RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, Booty);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, Vocal);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, VocalFilter);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, ColoredNoise);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, Tremolo);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, CPU_Hog);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, ThreadBoost);

RACK_PLUGIN_INIT(squinkylabs_plug1) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, Booty);
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, Vocal);
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, VocalFilter);
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, ColoredNoise);
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, Tremolo);
#ifdef _CPU_HOG
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, CPU_Hog);
#endif
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, ThreadBoost);
}
