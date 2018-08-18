#include "JE.hpp"

RACK_PLUGIN_MODEL_DECLARE(JE, RingModulator);
RACK_PLUGIN_MODEL_DECLARE(JE, SimpleWaveFolder);

RACK_PLUGIN_INIT(JE) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/eres-j/VCVRack-plugin-JE");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/eres-j/VCVRack-plugin-JE");

   RACK_PLUGIN_MODEL_ADD(JE, RingModulator);
   RACK_PLUGIN_MODEL_ADD(JE, SimpleWaveFolder);
}
