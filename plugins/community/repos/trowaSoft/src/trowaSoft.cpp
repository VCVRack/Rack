#include "trowaSoft.hpp"
#include "Widget_multiScope.hpp"
#include "Widget_multiScope_Old.hpp"
#include "TSSequencerModuleBase.hpp"
#include "Module_voltSeq.hpp"
#include "Module_oscCV.hpp"
#include "Module_multiOscillator.hpp"

// Sequencer Modules:
RACK_PLUGIN_MODEL_DECLARE(trowaSoft, TrigSeq);
RACK_PLUGIN_MODEL_DECLARE(trowaSoft, TrigSeq64);
RACK_PLUGIN_MODEL_DECLARE(trowaSoft, VoltSeq);

// Osc <==> CV:
RACK_PLUGIN_MODEL_DECLARE(trowaSoft, OscCV);
	
// Scope Modules:
RACK_PLUGIN_MODEL_DECLARE(trowaSoft, MultiScope);

// Oscillator
RACK_PLUGIN_MODEL_DECLARE(trowaSoft, MultiOscillator);

RACK_PLUGIN_INIT(trowaSoft) {
   RACK_PLUGIN_INIT_ID();

	// Sequencer Modules:
	// Add EXTERNAL_TAG for osc
	// [03/08/2018] Create model objects in module cpp files per forum topic.
	RACK_PLUGIN_MODEL_ADD(trowaSoft, TrigSeq);
	RACK_PLUGIN_MODEL_ADD(trowaSoft, TrigSeq64);
	RACK_PLUGIN_MODEL_ADD(trowaSoft, VoltSeq);

	// Osc <==> CV:
	RACK_PLUGIN_MODEL_ADD(trowaSoft, OscCV);
	
	// Scope Modules:
	RACK_PLUGIN_MODEL_ADD(trowaSoft, MultiScope);

	// Oscillator
	RACK_PLUGIN_MODEL_ADD(trowaSoft, MultiOscillator);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables within this file or the individual module files to reduce startup times of Rack.	
	return;
}
