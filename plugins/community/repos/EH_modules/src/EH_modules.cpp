#include "EH_modules.h"

RACK_PLUGIN_MODEL_DECLARE(EH_modules, FV1Emu);

RACK_PLUGIN_INIT(EH_modules) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.1");
   RACK_PLUGIN_INIT_WEBSITE("https://github.com/eh2k/fv1-emu/");

	RACK_PLUGIN_MODEL_ADD(EH_modules, FV1Emu);
}
