#include "mtsch.hpp"

RACK_PLUGIN_MODEL_DECLARE(mtsch_plugins, Sum);
RACK_PLUGIN_MODEL_DECLARE(mtsch_plugins, Rationals);
RACK_PLUGIN_MODEL_DECLARE(mtsch_plugins, TriggerPanic);

RACK_PLUGIN_INIT(mtsch_plugins) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/mtsch/mtsch-vcvrack-plugins");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/mtsch/mtsch-vcvrack-plugins");

   RACK_PLUGIN_MODEL_ADD(mtsch_plugins, Sum);
   RACK_PLUGIN_MODEL_ADD(mtsch_plugins, Rationals);
   RACK_PLUGIN_MODEL_ADD(mtsch_plugins, TriggerPanic);
}
