#include "common.hpp"

using namespace rack_plugin_TheXOR;

// #include "Klee.hpp"
// #include "M581.hpp"
// #include "Z8K.hpp"
// #include "Renato.hpp"
// #include "Spiralone.hpp"
// #include "pwmClock.hpp"
// #include "quantizer.hpp"
// #include "burst.hpp"
// #include "uncert.hpp"
// #include "attenuator.hpp"
// #include "boole.hpp"
// #include "mplex.hpp"
// #include "switch.hpp"

#ifdef LPTEST_MODULE
#include "lpTestModule.hpp"
#endif 

#ifdef OSCTEST_MODULE
#include "oscTestModule.hpp"
#endif 

RACK_PLUGIN_MODEL_DECLARE(TheXOR, Klee);
RACK_PLUGIN_MODEL_DECLARE(TheXOR, M581);
RACK_PLUGIN_MODEL_DECLARE(TheXOR, Z8K);
RACK_PLUGIN_MODEL_DECLARE(TheXOR, Renato);
RACK_PLUGIN_MODEL_DECLARE(TheXOR, Spiralone);
RACK_PLUGIN_MODEL_DECLARE(TheXOR, Burst);
RACK_PLUGIN_MODEL_DECLARE(TheXOR, Uncertain);
RACK_PLUGIN_MODEL_DECLARE(TheXOR, PwmClock);
RACK_PLUGIN_MODEL_DECLARE(TheXOR, Quantizer);
RACK_PLUGIN_MODEL_DECLARE(TheXOR, Attenuator);
RACK_PLUGIN_MODEL_DECLARE(TheXOR, Boole);
RACK_PLUGIN_MODEL_DECLARE(TheXOR, Switch);
RACK_PLUGIN_MODEL_DECLARE(TheXOR, Mplex);

RACK_PLUGIN_INIT(TheXOR) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/The-XOR/VCV-Sequencers");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/The-XOR/VCV-Sequencers/blob/master/README.md");

	RACK_PLUGIN_MODEL_ADD(TheXOR, Klee);
	RACK_PLUGIN_MODEL_ADD(TheXOR, M581);
	RACK_PLUGIN_MODEL_ADD(TheXOR, Z8K);
	RACK_PLUGIN_MODEL_ADD(TheXOR, Renato);
	RACK_PLUGIN_MODEL_ADD(TheXOR, Spiralone);
	RACK_PLUGIN_MODEL_ADD(TheXOR, Burst);
	RACK_PLUGIN_MODEL_ADD(TheXOR, Uncertain);
	RACK_PLUGIN_MODEL_ADD(TheXOR, PwmClock);
	RACK_PLUGIN_MODEL_ADD(TheXOR, Quantizer);
	RACK_PLUGIN_MODEL_ADD(TheXOR, Attenuator);
	RACK_PLUGIN_MODEL_ADD(TheXOR, Boole);
	RACK_PLUGIN_MODEL_ADD(TheXOR, Switch);
	RACK_PLUGIN_MODEL_ADD(TheXOR, Mplex);

#ifdef LPTEST_MODULE
	p->addModel(Model::create<LaunchpadTest, LaunchpadTestWidget>("TheXOR", "LaunchpadTest", "Launchpad Test", DIGITAL_TAG));
#endif

#ifdef OSCTEST_MODULE
	p->addModel(Model::create<OscTest, OscTestWidget>("TheXOR", "OSCTest", "OSC Test", DIGITAL_TAG));
#endif
}
