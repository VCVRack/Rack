#ifndef PLUGIN_SERIALRACKER_H_
#define PLUGIN_SERIALRACKER_H_

#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(SerialRacker);

#ifdef USE_VST2
#define plugin "SerialRacker"
#endif // USE_VST2

#define SET_ROW_NAME(I) "Name for row #" #I

#endif  // PLUGIN_SERIALRACKER_H_
