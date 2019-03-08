#include "MicMusic.hpp"

RACK_PLUGIN_MODEL_DECLARE(MicMusic, Distortion);
RACK_PLUGIN_MODEL_DECLARE(MicMusic, Adder);

RACK_PLUGIN_INIT(MicMusic) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.3");
   RACK_PLUGIN_INIT_WEBSITE("https://github.com/very-cool-name/MicMusic-VCV");

	RACK_PLUGIN_MODEL_ADD(MicMusic, Distortion);
	RACK_PLUGIN_MODEL_ADD(MicMusic, Adder);
}
