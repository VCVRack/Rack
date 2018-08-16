#include "Template.hpp"

RACK_PLUGIN_MODEL_DECLARE(PG_Instruments, PGSEQ3);
RACK_PLUGIN_MODEL_DECLARE(PG_Instruments, PGPanner);
RACK_PLUGIN_MODEL_DECLARE(PG_Instruments, PGQuadPanner);
RACK_PLUGIN_MODEL_DECLARE(PG_Instruments, PGOctPanner);
RACK_PLUGIN_MODEL_DECLARE(PG_Instruments, PGVCF);
RACK_PLUGIN_MODEL_DECLARE(PG_Instruments, PGStereoVCF);
RACK_PLUGIN_MODEL_DECLARE(PG_Instruments, PGEcho);
RACK_PLUGIN_MODEL_DECLARE(PG_Instruments, PGStereoEcho);
RACK_PLUGIN_MODEL_DECLARE(PG_Instruments, PGStereoPingPongEcho);

RACK_PLUGIN_INIT(PG_Instruments) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/imekon/PG-Instruments");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/imekon/PG-Instruments/README.md");

	// Add all Models defined throughout the plugin
   RACK_PLUGIN_MODEL_ADD(PG_Instruments, PGSEQ3);
   RACK_PLUGIN_MODEL_ADD(PG_Instruments, PGPanner);
   RACK_PLUGIN_MODEL_ADD(PG_Instruments, PGQuadPanner);
   RACK_PLUGIN_MODEL_ADD(PG_Instruments, PGOctPanner);
   RACK_PLUGIN_MODEL_ADD(PG_Instruments, PGVCF);
   RACK_PLUGIN_MODEL_ADD(PG_Instruments, PGStereoVCF);
   RACK_PLUGIN_MODEL_ADD(PG_Instruments, PGEcho);
   RACK_PLUGIN_MODEL_ADD(PG_Instruments, PGStereoEcho);
   RACK_PLUGIN_MODEL_ADD(PG_Instruments, PGStereoPingPongEcho);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
