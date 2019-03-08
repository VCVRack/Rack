#include "turing-module.hh"

#include <cmath>

namespace rack_plugin_Skylights {

const double SEMITONE = 1.0 / 12.0;

void turing_module::step() {
   double mode;
   if (inputs[I_MODE].active)
      mode = inputs[I_MODE].value;
   else
      mode = params[P_MODE].value;

   bool hot = m_sequence & 0x1;
   outputs[O_GATE].value = hot ? 10.0 : 0.0;
   outputs[O_PULSE].value =
     min(outputs[O_GATE].value * inputs[I_CLOCK].value, 10.0);

   // check for clock advance
   auto was_high = m_clock_trigger.isHigh();
   m_clock_trigger.process(inputs[I_CLOCK].value);
   if (!was_high && was_high != m_clock_trigger.isHigh()) {
     // clock was advanced

     // write knob always zeroes our input
     if (params[P_WRITE].value > 0.9) hot = false;
     else if (mode > 0.9) {
	 // leave hot alone
      } else if (mode > 0.55) {
	 // inverts about 13% of the time
	 const size_t TRIES = 3;
	 bool should_flip = true;
	 for (size_t i = 0;
	      i <= TRIES;
	      i++)
	 {
	    should_flip = (should_flip == m_spigot.next());
	    if (!should_flip) break;
	 }
	 hot = should_flip ? !hot : hot;
      } else if (mode > 0.10) {
	 // 50/50 invert
	 bool should_invert = m_spigot.next();
	 hot = should_invert ? !hot : hot;
      } else {
	 // always invert
	 hot = !hot;
      }

      // compute an advance mask based on step length, [2, 16)
      uint16_t mask = 0;
      size_t steps = 0;
      for (double i = 0;
	   i < params[P_LENGTH].value;
	   i += 1)
      {
	 mask <<= 1;
	 mask |= 0x1;
	 steps++;
      }

      uint16_t seq = m_sequence & mask;
      seq >>= 1;
      seq |= (hot ? 1 : 0) << (steps - 1);
      m_sequence &= ~mask;
      m_sequence += seq;

      uint8_t signal_d = m_sequence & 0xFF;
      double signal_a = (((double)signal_d) / 255.0);
      outputs[O_VOLTAGE].value =
	(signal_a * params[P_SCALE].value) // signal scaled by scale knob
	- (5.0 * params[P_POLE].value);    // shift to bi-polar on request

      // expander is always 10v unipolar
      outputs[O_EXPANSION].value = (((double)m_sequence) / 65535.0) * 10.0;

      for (size_t i = 0;
	   i < 8;
	   i++)
      {
	 lights[L_LIGHT8-i].value = ((m_sequence & (1 << i)) > 0) ? 1.0 : 0.0;
      }
   }
}

turing_module::turing_module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS), m_sequence(0) {
}

turing_module::~turing_module() {
}

json_t* turing_module::toJson() {
   auto map = json_object();

   json_object_set_new(map, "sequence", json_integer(m_sequence));

   return map;
}

void turing_module::fromJson(json_t *root) {
   if (!root) return;

   auto seqo = json_object_get(root, "sequence");
   if (json_is_number(seqo)) {
      m_sequence = (uint16_t)json_integer_value(seqo);
   }
}

void turing_module::onReset() {
  Module::onReset();
  m_sequence = 0;
}

void turing_module::onRandomize() {
  Module::onRandomize();
  m_sequence = 0;
  for (size_t i = 0;
       i < 16;
       i++)
  {
     m_sequence += m_spigot.next() << i;
  }
}

} // namespace rack_plugin_Skylights
