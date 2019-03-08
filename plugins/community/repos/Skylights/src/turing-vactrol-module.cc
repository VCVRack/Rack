#include "turing-vactrol-module.hh"

namespace rack_plugin_Skylights {

void turing_vactrol_module::step() {
   uint16_t seq = (uint16_t)ceil((inputs[I_EXPANDER].value / 10.0) * 65535.0);

   lights[L_GATE1].value = (seq & 1) > 0 ? 1.0 : 0.0;
   lights[L_GATE2].value = (seq & 2) > 0 ? 1.0 : 0.0;
   lights[L_GATE3].value = (seq & 4) > 0 ? 1.0 : 0.0;
   lights[L_GATE4].value = (seq & 8) > 0 ? 1.0 : 0.0;
   lights[L_GATE5].value = (seq & 16) > 0 ? 1.0 : 0.0;
   lights[L_GATE6].value = (seq & 32) > 0 ? 1.0 : 0.0;
   lights[L_GATE7].value = (seq & 64) > 0 ? 1.0 : 0.0;
   lights[L_GATE8].value = (seq & 128) > 0 ? 1.0 : 0.0;

   outputs[O_LEFT].value = 0.0;
   outputs[O_RIGHT].value = 0.0;

   size_t o = 0;
   for (size_t i = 0;
	i < 4;
	i++)
   {
      if (seq & (1 << o++)) {
	 outputs[O_LEFT].value += params[P_VOL1 + i].value * inputs[I_INPUT1 + i].value;
      }

      if (seq & (1 << o++)) {
	 outputs[O_RIGHT].value += params[P_VOL1 + i].value * inputs[I_INPUT1 + i].value;
      }
   }   
}

turing_vactrol_module::turing_vactrol_module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
}

turing_vactrol_module::~turing_vactrol_module() {
}

} // namespace rack_plugin_Skylights
