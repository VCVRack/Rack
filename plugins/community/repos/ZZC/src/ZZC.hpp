
#ifndef ZZC_H
#define ZZC_H

#include "rack.hpp"
// #include "window.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(ZZC);

#if defined(USE_VST2) && !defined(RACK_PLUGIN_SHARED)
#define plugin "ZZC"
#endif // USE_VST2
using namespace rack;

#include "../src/shared.hpp"
#include "../../ZZC/src/widgets.hpp"

#endif // ZZC_H
