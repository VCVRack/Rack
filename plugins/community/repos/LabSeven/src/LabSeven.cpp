#include "LabSeven.hpp"

RACK_PLUGIN_MODEL_DECLARE(LabSeven, LS3340VCO);

RACK_PLUGIN_INIT(LabSeven) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.2");
   RACK_PLUGIN_INIT_WEBSITE("https://github.com/LabSevenDevVCVRack/LabSeven_VCVRack_modules");

	RACK_PLUGIN_MODEL_ADD(LabSeven, LS3340VCO);
}
