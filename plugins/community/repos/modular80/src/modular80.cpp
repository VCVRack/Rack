#include "modular80.hpp"

RACK_PLUGIN_MODEL_DECLARE(modular80, Logistiker);

RACK_PLUGIN_INIT(modular80) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/cschol/modular80");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/cschol/modular80/blob/master/README.md");

   RACK_PLUGIN_MODEL_ADD(modular80, Logistiker);
}
