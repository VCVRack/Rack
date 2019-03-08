// plugin main
#include "Squinky.hpp"
#include "ctrl/SqHelper.h"

RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, Blank);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, Booty);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, CHB);
#ifdef _DG
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, DG);
#endif
#ifdef _EV3
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, EV3);
#endif
#ifdef _EV
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, EV);
#endif
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, FunV);
#ifdef _GMR
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, GMR);
#endif
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, Gray);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, LFN);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, Shaper);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, Super);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, Vocal);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, VocalFilter);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, ColoredNoise);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, Tremolo);
#ifdef _CPU_HOG
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, CPU_Hog);
#endif
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, ThreadBoost);

#ifdef _CH10
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, CH10);
#endif
#ifdef _CHBG
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, CHBg);
#endif
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, KS);
RACK_PLUGIN_MODEL_DECLARE(squinkylabs_plug1, Sequencer);

RACK_PLUGIN_INIT(squinkylabs_plug1) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.14");

   // RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, Blank); // crashes
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, Booty);
#ifdef _DG
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, DG);
#endif
#ifdef _EV3
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, EV3);
#endif
#ifdef _EV
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, EV);
#endif
   // RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, FunV); // crashes (read from 0xfffffff)
#ifdef _GMR
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, GMR);
#endif
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, Gray);
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, LFN);
   // RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, Shaper); // crashes (read from 0x00000010)
   // RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, Super); // assert !widget->parent
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, Vocal);
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, VocalFilter);
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, ColoredNoise);
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, Tremolo);
#ifdef _CPU_HOG
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, CPU_Hog);
#endif
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, ThreadBoost);


#ifdef _CH10
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, CH10);
#endif
   // RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, CHB);  // assert !widget->parent
#ifdef _CHBG
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, CHBg);
#endif
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, KS);
   RACK_PLUGIN_MODEL_ADD(squinkylabs_plug1, Sequencer);
}

const NVGcolor SqHelper::COLOR_WHITE = nvgRGB(0xff, 0xff, 0xff);
const NVGcolor SqHelper::COLOR_BLACK = nvgRGB(0,0,0);
