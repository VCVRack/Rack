#include "LindenbergResearch.hpp"

using namespace rack;

RACK_PLUGIN_MODEL_DECLARE(LindenbergResearch, SimpleFilter);
RACK_PLUGIN_MODEL_DECLARE(LindenbergResearch, MS20Filter);
RACK_PLUGIN_MODEL_DECLARE(LindenbergResearch, AlmaFilter);
RACK_PLUGIN_MODEL_DECLARE(LindenbergResearch, ReShaper);
RACK_PLUGIN_MODEL_DECLARE(LindenbergResearch, Westcoast);

RACK_PLUGIN_MODEL_DECLARE(LindenbergResearch, VCO);

RACK_PLUGIN_MODEL_DECLARE(LindenbergResearch, BlankPanel);
RACK_PLUGIN_MODEL_DECLARE(LindenbergResearch, BlankPanelM1);
// RACK_PLUGIN_MODEL_DECLARE(LindenbergResearch, BlankPanelSmall);

RACK_PLUGIN_INIT(LindenbergResearch) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/lindenbergresearch/LRTRack");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/lindenbergresearch/LRTRack");

   RACK_PLUGIN_MODEL_ADD(LindenbergResearch, SimpleFilter);
   RACK_PLUGIN_MODEL_ADD(LindenbergResearch, MS20Filter);
   RACK_PLUGIN_MODEL_ADD(LindenbergResearch, AlmaFilter);
   RACK_PLUGIN_MODEL_ADD(LindenbergResearch, ReShaper);
   RACK_PLUGIN_MODEL_ADD(LindenbergResearch, Westcoast);

   RACK_PLUGIN_MODEL_ADD(LindenbergResearch, VCO);

   RACK_PLUGIN_MODEL_ADD(LindenbergResearch, BlankPanel);
   RACK_PLUGIN_MODEL_ADD(LindenbergResearch, BlankPanelM1);
   // RACK_PLUGIN_MODEL_ADD(LindenbergResearch, BlankPanelSmall); // crash
}
