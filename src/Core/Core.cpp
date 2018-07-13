#include "global_pre.hpp"
#include "Core.hpp"
#include "global.hpp"

#ifdef RACK_HOST

RACK_PLUGIN_MODEL_DECLARE(Core, AudioInterface);
RACK_PLUGIN_MODEL_DECLARE(Core, MIDIToCVInterface);
RACK_PLUGIN_MODEL_DECLARE(Core, QuadMIDIToCVInterface);
RACK_PLUGIN_MODEL_DECLARE(Core, MIDICCToCVInterface);
RACK_PLUGIN_MODEL_DECLARE(Core, MIDITriggerToCVInterface);
RACK_PLUGIN_MODEL_DECLARE(Core, Blank);
RACK_PLUGIN_MODEL_DECLARE(Core, Notes);

#undef SLUG
#define SLUG Core
RACK_PLUGIN_INIT(Core) {
   RACK_PLUGIN_INIT_ID_INTERNAL;

	RACK_PLUGIN_MODEL_ADD(Core, AudioInterface);
	RACK_PLUGIN_MODEL_ADD(Core, MIDIToCVInterface);
	RACK_PLUGIN_MODEL_ADD(Core, QuadMIDIToCVInterface);
	RACK_PLUGIN_MODEL_ADD(Core, MIDICCToCVInterface);
	RACK_PLUGIN_MODEL_ADD(Core, MIDITriggerToCVInterface);
	RACK_PLUGIN_MODEL_ADD(Core, Blank);
	RACK_PLUGIN_MODEL_ADD(Core, Notes);
}

#endif // RACK_HOST
