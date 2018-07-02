#include "Befaco.hpp"


RACK_PLUGIN_MODEL_DECLARE(Befaco, ABC);
RACK_PLUGIN_MODEL_DECLARE(Befaco, DualAtenuverter);
RACK_PLUGIN_MODEL_DECLARE(Befaco, EvenVCO);
RACK_PLUGIN_MODEL_DECLARE(Befaco, Mixer);
RACK_PLUGIN_MODEL_DECLARE(Befaco, Rampage);
RACK_PLUGIN_MODEL_DECLARE(Befaco, SlewLimiter);
RACK_PLUGIN_MODEL_DECLARE(Befaco, SpringReverb);

RACK_PLUGIN_INIT(Befaco) {
   RACK_PLUGIN_INIT_ID();

	RACK_PLUGIN_MODEL_ADD(Befaco, EvenVCO);
	RACK_PLUGIN_MODEL_ADD(Befaco, Rampage);
	RACK_PLUGIN_MODEL_ADD(Befaco, ABC);
	RACK_PLUGIN_MODEL_ADD(Befaco, SpringReverb);
	RACK_PLUGIN_MODEL_ADD(Befaco, Mixer);
	RACK_PLUGIN_MODEL_ADD(Befaco, SlewLimiter);
	RACK_PLUGIN_MODEL_ADD(Befaco, DualAtenuverter);

	springReverbInit();
}
