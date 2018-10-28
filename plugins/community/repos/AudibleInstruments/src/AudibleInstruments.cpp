#include "AudibleInstruments.hpp"


RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Braids);
RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Elements);
RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Tides);
RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Clouds);
RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Warps);
RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Rings);
RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Links);
RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Kinks);
RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Shades);
RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Branches);
RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Blinds);
RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Veils);
RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Frames);
RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Marbles);
RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Plaits);
RACK_PLUGIN_MODEL_DECLARE(AudibleInstruments, Stages);

RACK_PLUGIN_INIT(AudibleInstruments) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.3");

	RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Braids);
	RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Elements);
	RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Tides);
	RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Clouds);
	RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Warps);
	RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Rings);
	RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Links);
	RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Kinks);
	RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Shades);
	RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Branches);
	RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Blinds);
	RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Veils);
	RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Frames);
	// RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Peaks);
   RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Marbles);
   RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Plaits);
   RACK_PLUGIN_MODEL_ADD(AudibleInstruments, Stages);
}
