////////////////////////////////////////////////////////////////////////////////////////////////////
////// Ohmer Modules ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

RACK_PLUGIN_MODEL_DECLARE(Ohmer, KlokSpid);
RACK_PLUGIN_MODEL_DECLARE(Ohmer, RKD);
RACK_PLUGIN_MODEL_DECLARE(Ohmer, RKDBRK);
RACK_PLUGIN_MODEL_DECLARE(Ohmer, Metriks);
RACK_PLUGIN_MODEL_DECLARE(Ohmer, Splitter1x9);
RACK_PLUGIN_MODEL_DECLARE(Ohmer, BlankPanel1);
RACK_PLUGIN_MODEL_DECLARE(Ohmer, BlankPanel2);
RACK_PLUGIN_MODEL_DECLARE(Ohmer, BlankPanel4);
RACK_PLUGIN_MODEL_DECLARE(Ohmer, BlankPanel8);
RACK_PLUGIN_MODEL_DECLARE(Ohmer, BlankPanel16);
RACK_PLUGIN_MODEL_DECLARE(Ohmer, BlankPanel32);

RACK_PLUGIN_INIT(Ohmer) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/DomiKamu/Ohmer-Modules/");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/DomiKamu/Ohmer-Modules/blob/master/README.md");

   RACK_PLUGIN_MODEL_ADD(Ohmer, KlokSpid);
   RACK_PLUGIN_MODEL_ADD(Ohmer, RKD);
   RACK_PLUGIN_MODEL_ADD(Ohmer, RKDBRK);
   RACK_PLUGIN_MODEL_ADD(Ohmer, Metriks);
   RACK_PLUGIN_MODEL_ADD(Ohmer, Splitter1x9);
   RACK_PLUGIN_MODEL_ADD(Ohmer, BlankPanel1);
   RACK_PLUGIN_MODEL_ADD(Ohmer, BlankPanel2);
   RACK_PLUGIN_MODEL_ADD(Ohmer, BlankPanel4);
   RACK_PLUGIN_MODEL_ADD(Ohmer, BlankPanel8);
   RACK_PLUGIN_MODEL_ADD(Ohmer, BlankPanel16);
   RACK_PLUGIN_MODEL_ADD(Ohmer, BlankPanel32);
}
