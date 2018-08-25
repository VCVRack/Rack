#include "Stochasm.hpp"

RACK_PLUGIN_MODEL_DECLARE(com_soundchasing_stochasm, Resonator);

RACK_PLUGIN_INIT(com_soundchasing_stochasm) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://soundchasing.com/uncertainty");
   RACK_PLUGIN_INIT_MANUAL("https://soundchasing.com/stochasm");

   RACK_PLUGIN_MODEL_ADD(com_soundchasing_stochasm, Resonator);
}
