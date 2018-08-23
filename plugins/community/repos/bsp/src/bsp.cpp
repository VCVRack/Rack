#include "bsp.hpp"

RACK_PLUGIN_MODEL_DECLARE(bsp, Obxd_VCF);

RACK_PLUGIN_INIT(bsp) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/bsp2/VeeSeeVSTRack/tree/v0.6/plugins/community/repos/bsp");

   RACK_PLUGIN_MODEL_ADD(bsp, Obxd_VCF);
}
