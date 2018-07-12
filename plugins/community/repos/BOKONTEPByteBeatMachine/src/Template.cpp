#include "Template.hpp"

RACK_PLUGIN_MODEL_DECLARE(BOKONTEPByteBeatMachine, BokontepByteBeatMachine);

RACK_PLUGIN_INIT(BOKONTEPByteBeatMachine) {
   RACK_PLUGIN_INIT_ID();

	RACK_PLUGIN_MODEL_ADD(BOKONTEPByteBeatMachine, BokontepByteBeatMachine);
}
