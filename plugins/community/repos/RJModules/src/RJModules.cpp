#include "RJModules.hpp"
#include "VAStateVariableFilter.h"

// Generators
RACK_PLUGIN_MODEL_DECLARE(RJModules, Supersaw);
RACK_PLUGIN_MODEL_DECLARE(RJModules, TwinLFO);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Noise);
RACK_PLUGIN_MODEL_DECLARE(RJModules, RangeLFO);

// FX
RACK_PLUGIN_MODEL_DECLARE(RJModules, BitCrush);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Widener);
RACK_PLUGIN_MODEL_DECLARE(RJModules, FilterDelay);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Sidechain);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Stutter);

// Filters
RACK_PLUGIN_MODEL_DECLARE(RJModules, Filter);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Filters);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Notch);

// Numerical
RACK_PLUGIN_MODEL_DECLARE(RJModules, Integers);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Floats);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Randoms);

// Mix
RACK_PLUGIN_MODEL_DECLARE(RJModules, LRMixer);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Mono);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Volumes);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Panner);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Panners);

// Live
RACK_PLUGIN_MODEL_DECLARE(RJModules, BPM);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Button);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Buttons);

// Util
RACK_PLUGIN_MODEL_DECLARE(RJModules, Splitter);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Splitters);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Displays);
RACK_PLUGIN_MODEL_DECLARE(RJModules, Range);

RACK_PLUGIN_INIT(RJModules) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/Miserlou/RJModules");

   // Generators
   RACK_PLUGIN_MODEL_ADD(RJModules, Supersaw);
   RACK_PLUGIN_MODEL_ADD(RJModules, TwinLFO);
   RACK_PLUGIN_MODEL_ADD(RJModules, Noise);
   RACK_PLUGIN_MODEL_ADD(RJModules, RangeLFO);

   // FX
   RACK_PLUGIN_MODEL_ADD(RJModules, BitCrush);
   RACK_PLUGIN_MODEL_ADD(RJModules, Widener);
   RACK_PLUGIN_MODEL_ADD(RJModules, FilterDelay);
   RACK_PLUGIN_MODEL_ADD(RJModules, Sidechain);
   RACK_PLUGIN_MODEL_ADD(RJModules, Stutter);

   // Filters
   RACK_PLUGIN_MODEL_ADD(RJModules, Filter);
   RACK_PLUGIN_MODEL_ADD(RJModules, Filters);
   RACK_PLUGIN_MODEL_ADD(RJModules, Notch);

   // Numerical
   RACK_PLUGIN_MODEL_ADD(RJModules, Integers);
   RACK_PLUGIN_MODEL_ADD(RJModules, Floats);
   RACK_PLUGIN_MODEL_ADD(RJModules, Randoms);

   // Mix
   RACK_PLUGIN_MODEL_ADD(RJModules, LRMixer);
   RACK_PLUGIN_MODEL_ADD(RJModules, Mono);
   RACK_PLUGIN_MODEL_ADD(RJModules, Volumes);
   RACK_PLUGIN_MODEL_ADD(RJModules, Panner);
   RACK_PLUGIN_MODEL_ADD(RJModules, Panners);

   // Live
   RACK_PLUGIN_MODEL_ADD(RJModules, BPM);
   RACK_PLUGIN_MODEL_ADD(RJModules, Button);
   RACK_PLUGIN_MODEL_ADD(RJModules, Buttons);

   // Util
   RACK_PLUGIN_MODEL_ADD(RJModules, Splitter);
   RACK_PLUGIN_MODEL_ADD(RJModules, Splitters);
   RACK_PLUGIN_MODEL_ADD(RJModules, Displays);
   RACK_PLUGIN_MODEL_ADD(RJModules, Range);
}
