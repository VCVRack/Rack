#include "Autodafe.hpp"
// #include <math.h>
// #include "dsp/digital.hpp"
// #include "dsp/decimator.hpp"
// #include "dsp/fft.hpp"
// #include "dsp/filter.hpp"
// #include "dsp/fir.hpp"
// #include "dsp/frame.hpp"
// #include "dsp/minblep.hpp"
// #include "dsp/ode.hpp"
// #include "dsp/ringbuffer.hpp"
// #include "dsp/samplerate.hpp"

RACK_PLUGIN_MODEL_DECLARE(Autodafe, Multiple18);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, Multiple28);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, LFOWidget);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, Keyboard);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, BPMClock);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, ClockDivider);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, SEQ8);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, SEQ16);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, TriggerSeq);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, FixedFilter);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, MultiModeFilter);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, FormantFilter);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, FoldBack);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, BitCrusher);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, PhaserFx);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, ChorusFx);
RACK_PLUGIN_MODEL_DECLARE(Autodafe, ReverbFx);

RACK_PLUGIN_INIT(Autodafe) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/antoniograzioli/Autodafe");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/antoniograzioli/Autodafe");

   RACK_PLUGIN_MODEL_ADD(Autodafe, Multiple18);
   RACK_PLUGIN_MODEL_ADD(Autodafe, Multiple28);
   RACK_PLUGIN_MODEL_ADD(Autodafe, LFOWidget);
   RACK_PLUGIN_MODEL_ADD(Autodafe, Keyboard);
   RACK_PLUGIN_MODEL_ADD(Autodafe, BPMClock);
   RACK_PLUGIN_MODEL_ADD(Autodafe, ClockDivider);
   RACK_PLUGIN_MODEL_ADD(Autodafe, SEQ8);
   RACK_PLUGIN_MODEL_ADD(Autodafe, SEQ16);
   RACK_PLUGIN_MODEL_ADD(Autodafe, TriggerSeq);
   RACK_PLUGIN_MODEL_ADD(Autodafe, FixedFilter);
   RACK_PLUGIN_MODEL_ADD(Autodafe, MultiModeFilter);
   RACK_PLUGIN_MODEL_ADD(Autodafe, FormantFilter);
   RACK_PLUGIN_MODEL_ADD(Autodafe, FoldBack);
   RACK_PLUGIN_MODEL_ADD(Autodafe, BitCrusher);
   RACK_PLUGIN_MODEL_ADD(Autodafe, PhaserFx);
   RACK_PLUGIN_MODEL_ADD(Autodafe, ChorusFx);
   RACK_PLUGIN_MODEL_ADD(Autodafe, ReverbFx);
}
