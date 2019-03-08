/*

LFSR group. Linear Feedback Shift Register sequencers for VCV Rack

Robert A Moeser

Use however you want, but it would be better if you didn't learn anything from
this. I am not a C++ programmer, I am not a musician.

If you do make things starting here, leave my name around. My mom likes that.

*/


#include "LFSR.hpp"

RACK_PLUGIN_MODEL_DECLARE(alto777_LFSR, FG8);
RACK_PLUGIN_MODEL_DECLARE(alto777_LFSR, Psychtone);

RACK_PLUGIN_MODEL_DECLARE(alto777_LFSR, Amuse);
RACK_PLUGIN_MODEL_DECLARE(alto777_LFSR, a7Utility);
RACK_PLUGIN_MODEL_DECLARE(alto777_LFSR, cheapFX);
RACK_PLUGIN_MODEL_DECLARE(alto777_LFSR, Divada);
RACK_PLUGIN_MODEL_DECLARE(alto777_LFSR, YASeq3);

RACK_PLUGIN_INIT(alto777_LFSR) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/alto777/LFSR");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/alto777/LFSR/README.md");
   // 0.6.21

	RACK_PLUGIN_MODEL_ADD(alto777_LFSR, FG8);
	RACK_PLUGIN_MODEL_ADD(alto777_LFSR, Psychtone);

 	RACK_PLUGIN_MODEL_ADD(alto777_LFSR, Amuse);
	RACK_PLUGIN_MODEL_ADD(alto777_LFSR, a7Utility);
	RACK_PLUGIN_MODEL_ADD(alto777_LFSR, cheapFX);
	RACK_PLUGIN_MODEL_ADD(alto777_LFSR, Divada);
	RACK_PLUGIN_MODEL_ADD(alto777_LFSR, YASeq3);
}
