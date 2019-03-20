
#include "Southpole.hpp"

RACK_PLUGIN_MODEL_DECLARE(Southpole_parasites, Annuli);
RACK_PLUGIN_MODEL_DECLARE(Southpole_parasites, Smoke);
RACK_PLUGIN_MODEL_DECLARE(Southpole_parasites, Splash);

RACK_PLUGIN_INIT(Southpole_parasites) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/gbrandt1/southpole-vcvrack");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/gbrandt1/southpole-vcvrack/blob/master/README.md");

   RACK_PLUGIN_MODEL_ADD(Southpole_parasites, Annuli);
   RACK_PLUGIN_MODEL_ADD(Southpole_parasites, Smoke);  // crashes
   RACK_PLUGIN_MODEL_ADD(Southpole_parasites, Splash);
}
