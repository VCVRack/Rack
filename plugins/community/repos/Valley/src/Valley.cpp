#include "Valley.hpp"

RACK_PLUGIN_MODEL_DECLARE(Valley, Amalgam);
RACK_PLUGIN_MODEL_DECLARE(Valley, Interzone);
RACK_PLUGIN_MODEL_DECLARE(Valley, Topograph);
RACK_PLUGIN_MODEL_DECLARE(Valley, UGraph);
RACK_PLUGIN_MODEL_DECLARE(Valley, Dexter);
RACK_PLUGIN_MODEL_DECLARE(Valley, Plateau);

RACK_PLUGIN_INIT(Valley) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.16");
   RACK_PLUGIN_INIT_WEBSITE("https://github.com/ValleyAudio/ValleyRackFree");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/ValleyAudio/ValleyRackFree/blob/master/README.md");

   RACK_PLUGIN_MODEL_ADD(Valley, Amalgam);
   RACK_PLUGIN_MODEL_ADD(Valley, Interzone);
   RACK_PLUGIN_MODEL_ADD(Valley, Topograph);
   RACK_PLUGIN_MODEL_ADD(Valley, UGraph);
   RACK_PLUGIN_MODEL_ADD(Valley, Dexter);
   RACK_PLUGIN_MODEL_ADD(Valley, Plateau);
}
