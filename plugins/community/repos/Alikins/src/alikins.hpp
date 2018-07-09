#include "rack.hpp"

const int MOMENTARY_BUTTONS = 13;
const int INPUT_SOURCES = 1;
const int GATE_LENGTH_INPUTS = 5;
using namespace rack;

RACK_PLUGIN_DECLARE(Alikins);

#ifdef USE_VST2
#define plugin "Alikins"
#endif // USE_VST2
