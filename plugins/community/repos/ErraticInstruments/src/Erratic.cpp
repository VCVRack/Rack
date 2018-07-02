#include "Erratic.hpp"

RACK_PLUGIN_MODEL_DECLARE(ErraticInstruments, MPEToCV);
RACK_PLUGIN_MODEL_DECLARE(ErraticInstruments, QuadMPEToCV);

RACK_PLUGIN_INIT(ErraticInstruments) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/bafonso/Erratic");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/bafonso/Erratic/blob/master/README.md");

	RACK_PLUGIN_MODEL_ADD(ErraticInstruments, MPEToCV);
	RACK_PLUGIN_MODEL_ADD(ErraticInstruments, QuadMPEToCV);
	// RACK_PLUGIN_MODEL_ADD(ErraticInstruments, Notes);
}
