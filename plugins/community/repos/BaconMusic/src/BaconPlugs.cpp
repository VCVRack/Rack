#include "BaconPlugs.hpp"

// RACK_PLUGIN_MODEL_DECLARE(BaconMusic, HarMoNee);  // crashes
RACK_PLUGIN_MODEL_DECLARE(BaconMusic, Glissinator);
RACK_PLUGIN_MODEL_DECLARE(BaconMusic, PolyGnome);
RACK_PLUGIN_MODEL_DECLARE(BaconMusic, QuantEyes);
RACK_PLUGIN_MODEL_DECLARE(BaconMusic, SampleDelay);
  
#ifdef BUILD_SORTACHORUS
RACK_PLUGIN_MODEL_DECLARE(BaconMusic, SortaChorus);
#endif
  
RACK_PLUGIN_MODEL_DECLARE(BaconMusic, ChipNoise);
RACK_PLUGIN_MODEL_DECLARE(BaconMusic, ChipWaves);
RACK_PLUGIN_MODEL_DECLARE(BaconMusic, ChipYourWave);

RACK_PLUGIN_MODEL_DECLARE(BaconMusic, KarplusStrongPoly);
  
RACK_PLUGIN_MODEL_DECLARE(BaconMusic, ALingADing);
RACK_PLUGIN_MODEL_DECLARE(BaconMusic, Bitulator);

RACK_PLUGIN_INIT(BaconMusic) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/baconpaul/Bacon Music");
  
   // RACK_PLUGIN_MODEL_ADD(BaconMusic, HarMoNee);  // crashes
   RACK_PLUGIN_MODEL_ADD(BaconMusic, Glissinator);
   RACK_PLUGIN_MODEL_ADD(BaconMusic, PolyGnome);
   RACK_PLUGIN_MODEL_ADD(BaconMusic, QuantEyes);
   RACK_PLUGIN_MODEL_ADD(BaconMusic, SampleDelay);
  
#ifdef BUILD_SORTACHORUS
   RACK_PLUGIN_MODEL_ADD(BaconMusic, SortaChorus);
#endif
  
   RACK_PLUGIN_MODEL_ADD(BaconMusic, ChipNoise);
   RACK_PLUGIN_MODEL_ADD(BaconMusic, ChipWaves);
   RACK_PLUGIN_MODEL_ADD(BaconMusic, ChipYourWave);

   RACK_PLUGIN_MODEL_ADD(BaconMusic, KarplusStrongPoly);
  
   RACK_PLUGIN_MODEL_ADD(BaconMusic, ALingADing);
   RACK_PLUGIN_MODEL_ADD(BaconMusic, Bitulator);
}
