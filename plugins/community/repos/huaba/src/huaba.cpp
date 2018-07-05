#include "huaba.hpp"
#include "eq3.hpp"
#include "abbus.hpp"

RACK_PLUGIN_MODEL_DECLARE(huaba, EQ3);
RACK_PLUGIN_MODEL_DECLARE(huaba, ABBus);

RACK_PLUGIN_INIT(huaba) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_MODEL_ADD(huaba, EQ3);
   RACK_PLUGIN_MODEL_ADD(huaba, ABBus);
}
