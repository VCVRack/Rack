#include "rcm.h"

using namespace rack_plugin_rcm;

RACK_PLUGIN_MODEL_DECLARE(rcm, CV0to10Module);
RACK_PLUGIN_MODEL_DECLARE(rcm, CV5to5Module);
RACK_PLUGIN_MODEL_DECLARE(rcm, CVMmtModule);
RACK_PLUGIN_MODEL_DECLARE(rcm, CVS0to10Module);
RACK_PLUGIN_MODEL_DECLARE(rcm, CVTglModule);
RACK_PLUGIN_MODEL_DECLARE(rcm, DuckModule);
RACK_PLUGIN_MODEL_DECLARE(rcm, LoadCounterModule);
RACK_PLUGIN_MODEL_DECLARE(rcm, PianoRollModule);

RACK_PLUGIN_INIT(rcm) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.12");
   RACK_PLUGIN_INIT_WEBSITE("https://github.com/Rcomian/rcm-modules/");

	RACK_PLUGIN_MODEL_ADD(rcm, CV0to10Module);
	RACK_PLUGIN_MODEL_ADD(rcm, CV5to5Module);
	RACK_PLUGIN_MODEL_ADD(rcm, CVMmtModule);
	RACK_PLUGIN_MODEL_ADD(rcm, CVS0to10Module);
	RACK_PLUGIN_MODEL_ADD(rcm, CVTglModule);
	RACK_PLUGIN_MODEL_ADD(rcm, DuckModule);
	RACK_PLUGIN_MODEL_ADD(rcm, LoadCounterModule);
	RACK_PLUGIN_MODEL_ADD(rcm, PianoRollModule);

}
