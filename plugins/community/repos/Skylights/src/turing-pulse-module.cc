#include "turing-pulse-module.hh"

#include <iostream>

namespace rack_plugin_Skylights {

void turing_pulse_module::step() {
   uint16_t seq = (uint16_t)ceil((inputs[I_EXPANDER].value / 10.0) * 65535.0);

   outputs[O_GATE1      ].value	= ((seq & 1 ) == 1  ? 10.0 : 0.0);
   outputs[O_GATE2      ].value	= ((seq & 2 ) == 2  ? 10.0 : 0.0);
   outputs[O_GATE3      ].value	= ((seq & 4 ) == 4  ? 10.0 : 0.0);
   outputs[O_GATE4      ].value	= ((seq & 8 ) == 8  ? 10.0 : 0.0);
   outputs[O_GATE5      ].value	= ((seq & 16) == 16 ? 10.0 : 0.0);
   outputs[O_GATE6      ].value	= ((seq & 32) == 32 ? 10.0 : 0.0);
   outputs[O_GATE7      ].value	= ((seq & 64) == 64 ? 10.0 : 0.0);
   outputs[O_GATE1P2    ].value	= ((seq & 3 ) == 3  ? 10.0 : 0.0);
   outputs[O_GATE2P4    ].value	= ((seq & 10) == 10 ? 10.0 : 0.0);
   outputs[O_GATE4P7    ].value	= ((seq & 72) == 72 ? 10.0 : 0.0);
   outputs[O_GATE1P2P4P7].value = ((seq & 75) == 75 ? 10.0 : 0.0);

   if (inputs[I_PULSE].active) {
     outputs[O_GATE1      ].value *= inputs[I_PULSE].value;
     outputs[O_GATE2      ].value *= inputs[I_PULSE].value;
     outputs[O_GATE3      ].value *= inputs[I_PULSE].value;
     outputs[O_GATE4      ].value *= inputs[I_PULSE].value;
     outputs[O_GATE5      ].value *= inputs[I_PULSE].value;
     outputs[O_GATE6      ].value *= inputs[I_PULSE].value;
     outputs[O_GATE7      ].value *= inputs[I_PULSE].value;
     outputs[O_GATE1P2    ].value *= inputs[I_PULSE].value;
     outputs[O_GATE2P4    ].value *= inputs[I_PULSE].value;
     outputs[O_GATE4P7    ].value *= inputs[I_PULSE].value;
     outputs[O_GATE1P2P4P7].value *= inputs[I_PULSE].value;
   }

   lights[L_GATE1      ].value = outputs[O_GATE1      ].value;
   lights[L_GATE2      ].value = outputs[O_GATE2      ].value;
   lights[L_GATE3      ].value = outputs[O_GATE3      ].value;
   lights[L_GATE4      ].value = outputs[O_GATE4      ].value;
   lights[L_GATE5      ].value = outputs[O_GATE5      ].value;
   lights[L_GATE6      ].value = outputs[O_GATE6      ].value;
   lights[L_GATE7      ].value = outputs[O_GATE7      ].value;
   lights[L_GATE1P2    ].value = outputs[O_GATE1P2    ].value;
   lights[L_GATE2P4    ].value = outputs[O_GATE2P4    ].value;
   lights[L_GATE4P7    ].value = outputs[O_GATE4P7    ].value;
   lights[L_GATE1P2P4P7].value = outputs[O_GATE1P2P4P7].value;
}

turing_pulse_module::turing_pulse_module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
}

turing_pulse_module::~turing_pulse_module() {
}

} // namespace rack_plugin_Skylights
