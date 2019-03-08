#include "whatnote-module.hh"

#include <cmath>

namespace rack_plugin_Skylights {

const double SEMITONE = 1.0 / 12.0;

void whatnote_module::step() {
  voltage = inputs[0].value;
  
  // its not valid, so don't analyze it
  if (voltage < -10.0 || voltage > 10.0) {
    octave = -11.0;
    return;
  }

  double y;
  double x = modf(voltage, &y);	// semitones/cents are fractional part
  octave = (int)y + 4;		// octage is integer part

  // and find semitones in there
  if (x < 0.0) {
    octave -= 1.0;
    x = 1.0 + x;
  }

  double z;
  double w = modf(x / SEMITONE, &z);
  
  semitone = z;
  cents = (int)round(w * 100.0);
  if (cents == 100) {
    semitone = (semitone + 1) % 12;
    cents = 0;
  }

  assert(semitone >= 0);
  assert(semitone < 12);
}

whatnote_module::whatnote_module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS), octave(0), semitone(0), cents(0), voltage(0) {
}

whatnote_module::~whatnote_module() {
}

} // namespace rack_plugin_Skylights
