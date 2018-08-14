#include "21kHz.hpp"

RACK_PLUGIN_MODEL_DECLARE(21kHz, PalmLoop);
RACK_PLUGIN_MODEL_DECLARE(21kHz, D_Inf);

RACK_PLUGIN_INIT(21kHz) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/21kHz/21kHz-rack-plugins");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/21kHz/21kHz-rack-plugins");

	RACK_PLUGIN_MODEL_ADD(21kHz, PalmLoop);
	RACK_PLUGIN_MODEL_ADD(21kHz, D_Inf);
}
