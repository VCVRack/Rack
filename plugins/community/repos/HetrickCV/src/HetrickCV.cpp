#include "HetrickCV.hpp"


RACK_PLUGIN_MODEL_DECLARE(HetrickCV, TwoToFour);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, AnalogToDigital);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, ASR);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, Bitshift);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, BlankPanel);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, Boolean3);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, Comparator);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, Contrast);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, Crackle);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, Delta);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, DigitalToAnalog);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, Dust);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, Exponent);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, FlipFlop);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, FlipPan);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, GateJunction);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, LogicCombine);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, RandomGates);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, Rotator);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, Scanner);
RACK_PLUGIN_MODEL_DECLARE(HetrickCV, Waveshape);

RACK_PLUGIN_INIT(HetrickCV) {
   RACK_PLUGIN_INIT_ID();

	RACK_PLUGIN_INIT_WEBSITE("https://github.com/mhetrick/hetrickcv");
 	RACK_PLUGIN_INIT_MANUAL("https://github.com/mhetrick/hetrickcv/blob/master/README.md");

   RACK_PLUGIN_MODEL_ADD(HetrickCV, TwoToFour);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, AnalogToDigital);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, ASR);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, Bitshift);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, BlankPanel);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, Boolean3);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, Comparator);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, Contrast);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, Crackle);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, Delta);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, DigitalToAnalog);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, Dust);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, Exponent);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, FlipFlop);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, FlipPan);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, GateJunction);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, LogicCombine);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, RandomGates);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, Rotator);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, Scanner);
	RACK_PLUGIN_MODEL_ADD(HetrickCV, Waveshape);
	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables within this file or the individual module files to reduce startup times of Rack.
}
