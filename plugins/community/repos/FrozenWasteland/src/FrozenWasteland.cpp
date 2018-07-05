#include "FrozenWasteland.hpp"

//RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, EchoesThroughEternity);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, BPMLFO);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, BPMLFO2);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, DamianLillard);
//RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, SpectralDelay);
//RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, BleedingEdge);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, EverlastingGlottalStopper);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, HairPick);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, LissajousLFO);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, MrBlueSky);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, TheOneRingModulator);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, PhasedLockedLoop);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, PortlandWeather);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, QuadEuclideanRhythm);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, QuadGolombRulerRhythm);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, QuantussyCell);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, RouletteLFO);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, SeriouslySlowLFO);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, VoxInhumana);
RACK_PLUGIN_MODEL_DECLARE(FrozenWasteland, CDCSeriouslySlowLFO);

RACK_PLUGIN_INIT(FrozenWasteland) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/almostEric/FrozenWasteland");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/almostEric/FrozenWasteland/blob/master/README.md");

	//RACK_PLUGIN_MODEL_ADD(FrozenWasteland, EchoesThroughEternity);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, BPMLFO);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, BPMLFO2);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, DamianLillard);
	//RACK_PLUGIN_MODEL_ADD(FrozenWasteland, SpectralDelay);
	//RACK_PLUGIN_MODEL_ADD(FrozenWasteland, BleedingEdge);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, EverlastingGlottalStopper);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, HairPick);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, LissajousLFO);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, MrBlueSky);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, TheOneRingModulator);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, PhasedLockedLoop);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, PortlandWeather);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, QuadEuclideanRhythm);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, QuadGolombRulerRhythm);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, QuantussyCell);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, RouletteLFO);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, SeriouslySlowLFO);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, VoxInhumana);
	RACK_PLUGIN_MODEL_ADD(FrozenWasteland, CDCSeriouslySlowLFO);
}
