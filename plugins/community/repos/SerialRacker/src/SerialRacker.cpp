#include "SerialRacker.hpp"

RACK_PLUGIN_MODEL_DECLARE(SerialRacker, MidiMultiplexer);

RACK_PLUGIN_INIT(SerialRacker) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_MODEL_ADD(SerialRacker, MidiMultiplexer);
}
