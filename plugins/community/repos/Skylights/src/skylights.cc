#include "skylights.hh"
#include "recorder-module.hh"

// RACK_PLUGIN_MODEL_DECLARE(Skylights, recorder_model);
RACK_PLUGIN_MODEL_DECLARE(Skylights, whatnote_model);
RACK_PLUGIN_MODEL_DECLARE(Skylights, turing_model);
RACK_PLUGIN_MODEL_DECLARE(Skylights, turing_volts_model);
RACK_PLUGIN_MODEL_DECLARE(Skylights, turing_pulse_model);
RACK_PLUGIN_MODEL_DECLARE(Skylights, turing_vactrol_model);

RACK_PLUGIN_INIT(Edge) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.3");
   RACK_PLUGIN_INIT_WEBSITE("https://github.com/Skrylar/skylights-vcv/");

	// RACK_PLUGIN_MODEL_ADD(Skylights, recorder_model);
	RACK_PLUGIN_MODEL_ADD(Skylights, whatnote_model);
	RACK_PLUGIN_MODEL_ADD(Skylights, turing_model);
	RACK_PLUGIN_MODEL_ADD(Skylights, turing_volts_model);
	RACK_PLUGIN_MODEL_ADD(Skylights, turing_pulse_model);
	RACK_PLUGIN_MODEL_ADD(Skylights, turing_vactrol_model);
}
