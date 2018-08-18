#include "common.hpp"

RACK_PLUGIN_MODEL_DECLARE(CastleRocktronics, Cubefader);

RACK_PLUGIN_INIT(CastleRocktronics) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/KieranPringle/CastleRocktronics");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/KieranPringle/CastleRocktronics");

   RACK_PLUGIN_MODEL_ADD(CastleRocktronics, Cubefader);
}
