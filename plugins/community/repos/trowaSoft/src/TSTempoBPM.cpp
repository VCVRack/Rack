#include "TSTempoBPM.hpp"

// Notes to multiplier (mult our dT to get BPM).
const TempoDivisor* BPMOptions[TROWA_TEMP_BPM_NUM_OPTIONS] = {
	new TempoDivisor("4", 60.0), // 1/4
	new TempoDivisor("8", 30.0), // 1/8
	new TempoDivisor("8T", 20.0), // 1/8T
	new TempoDivisor("16", 15.0)	// 1/16
};
