#include "SynthKit.hpp"

RACK_PLUGIN_MODEL_DECLARE(SynthKit, Addition);
RACK_PLUGIN_MODEL_DECLARE(SynthKit, Subtraction);
RACK_PLUGIN_MODEL_DECLARE(SynthKit, And);
RACK_PLUGIN_MODEL_DECLARE(SynthKit, Or);
RACK_PLUGIN_MODEL_DECLARE(SynthKit, M1x8);
RACK_PLUGIN_MODEL_DECLARE(SynthKit, M1x8CV);
RACK_PLUGIN_MODEL_DECLARE(SynthKit, ClockDivider);
RACK_PLUGIN_MODEL_DECLARE(SynthKit, RotatingClockDivider);
RACK_PLUGIN_MODEL_DECLARE(SynthKit, RotatingClockDivider2);
RACK_PLUGIN_MODEL_DECLARE(SynthKit, PrimeClockDivider);
RACK_PLUGIN_MODEL_DECLARE(SynthKit, FibonacciClockDivider);
RACK_PLUGIN_MODEL_DECLARE(SynthKit, Seq4);
RACK_PLUGIN_MODEL_DECLARE(SynthKit, Seq8);

RACK_PLUGIN_INIT(SynthKit) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/JerrySievert/SynthKit");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/JerrySievert/SynthKit/blob/master/docs/README.md");

   RACK_PLUGIN_MODEL_ADD(SynthKit, Addition);
   RACK_PLUGIN_MODEL_ADD(SynthKit, Subtraction);
   RACK_PLUGIN_MODEL_ADD(SynthKit, And);
   RACK_PLUGIN_MODEL_ADD(SynthKit, Or);
   RACK_PLUGIN_MODEL_ADD(SynthKit, M1x8);
   RACK_PLUGIN_MODEL_ADD(SynthKit, M1x8CV);
   RACK_PLUGIN_MODEL_ADD(SynthKit, ClockDivider);
   RACK_PLUGIN_MODEL_ADD(SynthKit, RotatingClockDivider);
   RACK_PLUGIN_MODEL_ADD(SynthKit, RotatingClockDivider2);
   RACK_PLUGIN_MODEL_ADD(SynthKit, PrimeClockDivider);
   RACK_PLUGIN_MODEL_ADD(SynthKit, FibonacciClockDivider);
   RACK_PLUGIN_MODEL_ADD(SynthKit, Seq4);
   RACK_PLUGIN_MODEL_ADD(SynthKit, Seq8);
}
