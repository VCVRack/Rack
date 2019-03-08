#include "Noobhour.hpp"

RACK_PLUGIN_MODEL_DECLARE(noobhour, Baseliner);
RACK_PLUGIN_MODEL_DECLARE(noobhour, Bsl1r);
RACK_PLUGIN_MODEL_DECLARE(noobhour, Customscaler);

RACK_PLUGIN_INIT(noobhour) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.2");
   RACK_PLUGIN_INIT_WEBSITE("https://github.com/NicolasNeubauer/noobhour_modules/");

	RACK_PLUGIN_MODEL_ADD(noobhour, Baseliner);
	RACK_PLUGIN_MODEL_ADD(noobhour, Bsl1r);
	RACK_PLUGIN_MODEL_ADD(noobhour, Customscaler);
}
