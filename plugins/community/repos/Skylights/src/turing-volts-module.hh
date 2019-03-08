#pragma once

#include "skylights.hh"

namespace rack_plugin_Skylights {

struct turing_volts_module : Module {
   enum ParamIds {
      P_VOL1, P_VOL2, P_VOL3, P_VOL4, P_VOL5,
      NUM_PARAMS
   };
   enum InputIds {
      I_EXPANDER,
      NUM_INPUTS
   };
   enum OutputIds {
      O_VOLTAGE,
      NUM_OUTPUTS
   };
   enum LightIds {
      L_LIGHT1, L_LIGHT2, L_LIGHT3, L_LIGHT4, L_LIGHT5,
      NUM_LIGHTS
   };

   turing_volts_module();
   virtual ~turing_volts_module();
  
   void step() override;
};

} // namespace rack_plugin_Skylights
