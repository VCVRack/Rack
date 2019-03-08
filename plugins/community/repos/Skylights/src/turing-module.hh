#pragma once

#include "skylights.hh"
#include "dsp/digital.hpp"
#include "bit-spigot.hh"

namespace rack_plugin_Skylights {

struct turing_module : Module {
   enum ParamIds {
      P_WRITE,
      P_LENGTH,
      P_MODE,
      P_POLE,
      P_SCALE,
      NUM_PARAMS
   };
   enum InputIds {
      I_CLOCK,
      I_MODE,
      NUM_INPUTS
   };
   enum OutputIds {
      O_VOLTAGE,
      O_EXPANSION,
      O_GATE,
      O_PULSE,
      NUM_OUTPUTS
   };
   enum LightIds {
      L_LIGHT1, L_LIGHT2, L_LIGHT3, L_LIGHT4,
      L_LIGHT5, L_LIGHT6, L_LIGHT7, L_LIGHT8,
      NUM_LIGHTS
   };

   uint16_t m_sequence;
   bit_spigot m_spigot;
   rack::SchmittTrigger m_clock_trigger;

   turing_module();
   virtual ~turing_module();

   json_t *toJson() override;
   void fromJson(json_t* root) override;
  
   void step() override;
   void onReset() override;
   void onRandomize() override;
};

} // namespace rack_plugin_Skylights
