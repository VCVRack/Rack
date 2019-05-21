#include "SubmarineUtility.hpp"

RACK_PLUGIN_MODEL_DECLARE(SubmarineUtility, ModBrowser);
RACK_PLUGIN_MODEL_DECLARE(SubmarineUtility, WireManager);

RACK_PLUGIN_INIT(SubmarineUtility) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.2");
   RACK_PLUGIN_INIT_WEBSITE("https://github.com/david-c14/SubmarineUtility");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/david-c14/SubmarineUtility");

   RACK_PLUGIN_MODEL_ADD(SubmarineUtility, ModBrowser);
   RACK_PLUGIN_MODEL_ADD(SubmarineUtility, WireManager);
}
