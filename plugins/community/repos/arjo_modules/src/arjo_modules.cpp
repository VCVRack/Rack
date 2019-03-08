#include "arjo_modules.hpp"

RACK_PLUGIN_MODEL_DECLARE(arjo_modules, Seq);
RACK_PLUGIN_MODEL_DECLARE(arjo_modules, Count);
RACK_PLUGIN_MODEL_DECLARE(arjo_modules, Switch);

RACK_PLUGIN_INIT(arjo_modules) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.0");
   RACK_PLUGIN_INIT_WEBSITE("https://github.com/ArjoNagelhout/arjo_modules");

	RACK_PLUGIN_MODEL_ADD(arjo_modules, Seq);
	RACK_PLUGIN_MODEL_ADD(arjo_modules, Count);
	RACK_PLUGIN_MODEL_ADD(arjo_modules, Switch);
}
