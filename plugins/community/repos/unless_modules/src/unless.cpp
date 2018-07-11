#include "unless.hpp"

RACK_PLUGIN_MODEL_DECLARE(unless_modules, Piong);
RACK_PLUGIN_MODEL_DECLARE(unless_modules, Markov);

RACK_PLUGIN_INIT(unless_modules) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_MODEL_ADD(unless_modules, Piong);
   RACK_PLUGIN_MODEL_ADD(unless_modules, Markov);
}
