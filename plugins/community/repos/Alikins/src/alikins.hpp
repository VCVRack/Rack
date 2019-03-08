#include <math.h>

#include "rack.hpp"

const int MOMENTARY_BUTTONS = 13;
const int INPUT_SOURCES = 1;
const int GATE_LENGTH_INPUTS = 5;
enum VoltageRange {
    MINUS_PLUS_TEN,
    ZERO_TEN,
    MINUS_PLUS_FIVE,
};

const float voltage_min[3] = {-10.0f, 0.0f, -5.0f};
const float voltage_max[3] = {10.0f, 10.0f, 5.0f};

using namespace rack;

RACK_PLUGIN_DECLARE(Alikins);

#ifdef USE_VST2
#define plugin "Alikins"
#endif // USE_VST2
