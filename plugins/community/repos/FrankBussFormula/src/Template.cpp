#include "Template.hpp"

RACK_PLUGIN_MODEL_DECLARE(FrankBussFormula, FrankBussFormula);

RACK_PLUGIN_INIT(FrankBussFormula) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/FrankBuss/Formula");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/FrankBuss/Formula");

   RACK_PLUGIN_MODEL_ADD(FrankBussFormula, FrankBussFormula);
}
