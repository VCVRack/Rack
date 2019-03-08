#include "turing-volts-module.hh"

namespace rack_plugin_Skylights {

void turing_volts_module::step() {
   uint16_t seq = (uint16_t)ceil((inputs[I_EXPANDER].value / 10.0) * 65535.0);

   double signal = 0;

   for (size_t i = 0;
	i < 5;
	i++)
   {
      double here = (((seq & (1 << i)) > 0.0) ? 1.0 : 0.0)
	 * params[P_VOL1 + i].value;
      lights[L_LIGHT1 + i].value = fabs(here);
      signal += here;
   }

   outputs[O_VOLTAGE].value = signal * 2.0;
}

turing_volts_module::turing_volts_module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
}

turing_volts_module::~turing_volts_module() {
}

} // namespace rack_plugin_Skylights
