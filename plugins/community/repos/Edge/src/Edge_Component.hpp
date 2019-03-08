#pragma once
#include "componentlibrary.hpp"
#include <vector>
#include <jansson.h>
#include "widgets.hpp"
#include <iostream>

using namespace std;
using namespace rack;

namespace rack_plugin_Edge {

    struct EdgeBlueKnob : RoundKnob {
        EdgeBlueKnob() {
            setSVG(SVG::load(assetPlugin(plugin,"res/RoundBlueKnob.svg")));
        }
    };

    struct EdgeRedKnob : RoundKnob {
        EdgeRedKnob() {
            setSVG(SVG::load(assetPlugin(plugin,"res/RoundRedKnob.svg")));
        }
    };

} // namespace rack_plugin_Edge
