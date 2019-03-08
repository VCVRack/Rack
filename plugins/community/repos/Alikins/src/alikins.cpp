#include "alikins.hpp"

RACK_PLUGIN_MODEL_DECLARE(Alikins, BigMuteButton);
RACK_PLUGIN_MODEL_DECLARE(Alikins, ColorPanel);
RACK_PLUGIN_MODEL_DECLARE(Alikins, GateLength);
RACK_PLUGIN_MODEL_DECLARE(Alikins, IdleSwitch);
RACK_PLUGIN_MODEL_DECLARE(Alikins, MomentaryOnButtons);
RACK_PLUGIN_MODEL_DECLARE(Alikins, Reference);
RACK_PLUGIN_MODEL_DECLARE(Alikins, HoveredValue);
RACK_PLUGIN_MODEL_DECLARE(Alikins, InjectValue);
RACK_PLUGIN_MODEL_DECLARE(Alikins, ShiftPedal);
RACK_PLUGIN_MODEL_DECLARE(Alikins, SpecificValue);
RACK_PLUGIN_MODEL_DECLARE(Alikins, ValueSaver);

RACK_PLUGIN_INIT(Alikins) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/alikins/Alikins-rack-plugins");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/alikins/Alikins-rack-plugins/blob/master/README.md");
   RACK_PLUGIN_INIT_VERSION("0.6.6");

   RACK_PLUGIN_MODEL_ADD(Alikins, BigMuteButton);
   RACK_PLUGIN_MODEL_ADD(Alikins, ColorPanel);
   RACK_PLUGIN_MODEL_ADD(Alikins, GateLength);
   RACK_PLUGIN_MODEL_ADD(Alikins, IdleSwitch);
   RACK_PLUGIN_MODEL_ADD(Alikins, MomentaryOnButtons);
   RACK_PLUGIN_MODEL_ADD(Alikins, Reference);
   RACK_PLUGIN_MODEL_ADD(Alikins, HoveredValue);
   RACK_PLUGIN_MODEL_ADD(Alikins, InjectValue);
   RACK_PLUGIN_MODEL_ADD(Alikins, ShiftPedal);
   RACK_PLUGIN_MODEL_ADD(Alikins, SpecificValue);
   RACK_PLUGIN_MODEL_ADD(Alikins, ValueSaver);
}
