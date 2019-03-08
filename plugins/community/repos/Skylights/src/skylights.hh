#pragma once

// we don't control these, so don't complain to me about them
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#include <vector>
#include <condition_variable>
#include <mutex>

#include "rack.hpp"

// re-entering our zone of concern
#pragma GCC diagnostic pop
#pragma clang diagnostic pop

using namespace rack;

namespace rack_plugin_Skylights {
}

RACK_PLUGIN_DECLARE(Skylights);

#ifdef USE_VST2
#define plugin "Skylights"
#endif // USE_VST2

#include "components.hh"
