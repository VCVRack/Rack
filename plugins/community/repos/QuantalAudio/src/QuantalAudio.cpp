#include "QuantalAudio.hpp"

RACK_PLUGIN_MODEL_DECLARE(QuantalAudio, MasterMixer);
RACK_PLUGIN_MODEL_DECLARE(QuantalAudio, BufferedMult);
RACK_PLUGIN_MODEL_DECLARE(QuantalAudio, UnityMix);
RACK_PLUGIN_MODEL_DECLARE(QuantalAudio, DaisyChannel);
RACK_PLUGIN_MODEL_DECLARE(QuantalAudio, DaisyMaster);
RACK_PLUGIN_MODEL_DECLARE(QuantalAudio, Horsehair);
RACK_PLUGIN_MODEL_DECLARE(QuantalAudio, Blank1);
RACK_PLUGIN_MODEL_DECLARE(QuantalAudio, Blank3);
RACK_PLUGIN_MODEL_DECLARE(QuantalAudio, Blank5);

RACK_PLUGIN_INIT(QuantalAudio) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.4");
   RACK_PLUGIN_INIT_WEBSITE("https://github.com/sumpygump/quantal-audio");

	RACK_PLUGIN_MODEL_ADD(QuantalAudio, MasterMixer);
	RACK_PLUGIN_MODEL_ADD(QuantalAudio, BufferedMult);
	RACK_PLUGIN_MODEL_ADD(QuantalAudio, UnityMix);
	RACK_PLUGIN_MODEL_ADD(QuantalAudio, DaisyChannel);
	RACK_PLUGIN_MODEL_ADD(QuantalAudio, DaisyMaster);
	RACK_PLUGIN_MODEL_ADD(QuantalAudio, Horsehair);
	RACK_PLUGIN_MODEL_ADD(QuantalAudio, Blank1);
	RACK_PLUGIN_MODEL_ADD(QuantalAudio, Blank3);
	RACK_PLUGIN_MODEL_ADD(QuantalAudio, Blank5);
}
