#include "dhe-modules.h"

RACK_PLUGIN_MODEL_DECLARE(DHE_Modules, BoosterStage);
RACK_PLUGIN_MODEL_DECLARE(DHE_Modules, Cubic);
RACK_PLUGIN_MODEL_DECLARE(DHE_Modules, Func);
RACK_PLUGIN_MODEL_DECLARE(DHE_Modules, Func6);
RACK_PLUGIN_MODEL_DECLARE(DHE_Modules, Hostage);
RACK_PLUGIN_MODEL_DECLARE(DHE_Modules, Ranger);
RACK_PLUGIN_MODEL_DECLARE(DHE_Modules, Stage);
RACK_PLUGIN_MODEL_DECLARE(DHE_Modules, Swave);
RACK_PLUGIN_MODEL_DECLARE(DHE_Modules, Tapers);
RACK_PLUGIN_MODEL_DECLARE(DHE_Modules, Upstage);
RACK_PLUGIN_MODEL_DECLARE(DHE_Modules, Xycloid);

RACK_PLUGIN_INIT(DHE_Modules) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.4");

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/dhemery/DHE-Modules/");
   // https://dhemery.github.io/DHE-Modules/

   RACK_PLUGIN_MODEL_ADD(DHE_Modules, BoosterStage);
   RACK_PLUGIN_MODEL_ADD(DHE_Modules, Cubic);
   RACK_PLUGIN_MODEL_ADD(DHE_Modules, Func);
   RACK_PLUGIN_MODEL_ADD(DHE_Modules, Func6);
   RACK_PLUGIN_MODEL_ADD(DHE_Modules, Hostage);
   RACK_PLUGIN_MODEL_ADD(DHE_Modules, Ranger);
   RACK_PLUGIN_MODEL_ADD(DHE_Modules, Stage);
   RACK_PLUGIN_MODEL_ADD(DHE_Modules, Swave);
   RACK_PLUGIN_MODEL_ADD(DHE_Modules, Tapers);
   RACK_PLUGIN_MODEL_ADD(DHE_Modules, Upstage);
   RACK_PLUGIN_MODEL_ADD(DHE_Modules, Xycloid);
}
