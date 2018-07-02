#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <algorithm>

#include "rack.hpp"

#include "dsp/digital.hpp"
#include "trigger_on_load.hpp"
#include "widgets.hpp"
#include "utils.hpp"

using namespace rack;
using namespace bogaudio;

RACK_PLUGIN_DECLARE(Bogaudio);

#ifdef USE_VST2
#define plugin "Bogaudio"
#endif // USE_VST2
