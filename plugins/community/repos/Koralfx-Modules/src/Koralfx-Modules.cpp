#include "Koralfx-Modules.hpp"

RACK_PLUGIN_MODEL_DECLARE(Koralfx, Beatovnik);
RACK_PLUGIN_MODEL_DECLARE(Koralfx, Mixovnik);
RACK_PLUGIN_MODEL_DECLARE(Koralfx, Nullovnik4);
RACK_PLUGIN_MODEL_DECLARE(Koralfx, Nullovnik6);
RACK_PLUGIN_MODEL_DECLARE(Koralfx, Presetovnik);
RACK_PLUGIN_MODEL_DECLARE(Koralfx, Quantovnik);
RACK_PLUGIN_MODEL_DECLARE(Koralfx, Scorovnik);

RACK_PLUGIN_INIT(Koralfx) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/koralfx/Koralfx-Modules");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/koralfx/Koralfx-Modules/blob/master/README.md");
   // 0.6.9

   RACK_PLUGIN_MODEL_ADD(Koralfx, Beatovnik);
   RACK_PLUGIN_MODEL_ADD(Koralfx, Mixovnik);
   RACK_PLUGIN_MODEL_ADD(Koralfx, Nullovnik4);
   RACK_PLUGIN_MODEL_ADD(Koralfx, Nullovnik6);
   RACK_PLUGIN_MODEL_ADD(Koralfx, Presetovnik);
   RACK_PLUGIN_MODEL_ADD(Koralfx, Quantovnik);
	RACK_PLUGIN_MODEL_ADD(Koralfx, Scorovnik);
}
