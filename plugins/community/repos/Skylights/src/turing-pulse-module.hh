#pragma once

#include "skylights.hh"

namespace rack_plugin_Skylights {

struct turing_pulse_module : Module {
   enum ParamIds {
      NUM_PARAMS
   };
   enum InputIds {
      I_EXPANDER,
      I_PULSE,
      NUM_INPUTS
   };
   enum OutputIds {
      O_GATE1,	 
      O_GATE2,	 
      O_GATE3,	 
      O_GATE4,	 
      O_GATE5,	 
      O_GATE6,	 
      O_GATE7,	 
      O_GATE1P2,
      O_GATE2P4,
      O_GATE4P7,
      O_GATE1P2P4P7,
      NUM_OUTPUTS
   };
   enum LightIds {
      L_GATE1,	 
      L_GATE2,	 
      L_GATE3,	 
      L_GATE4,	 
      L_GATE5,	 
      L_GATE6,	 
      L_GATE7,	 
      L_GATE1P2,
      L_GATE2P4,
      L_GATE4P7,
      L_GATE1P2P4P7,
      NUM_LIGHTS
   };

   turing_pulse_module();
   virtual ~turing_pulse_module();
  
   void step() override;
};

} // namespace rack_plugin_Skylights
