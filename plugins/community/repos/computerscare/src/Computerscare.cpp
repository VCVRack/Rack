#include "Computerscare.hpp"

RACK_PLUGIN_MODEL_DECLARE(computerscare, ComputerscareDebug);
RACK_PLUGIN_MODEL_DECLARE(computerscare, ComputerscarePatchSequencer);

RACK_PLUGIN_INIT(computerscare) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/freddyz/computerscare-vcv-modules");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/freddyz/computerscare-vcv-modules");

	RACK_PLUGIN_MODEL_ADD(computerscare, ComputerscareDebug);
	RACK_PLUGIN_MODEL_ADD(computerscare, ComputerscarePatchSequencer);
}
