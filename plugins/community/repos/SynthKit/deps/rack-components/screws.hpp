#pragma once

#include "asset.hpp"
#include "rack.hpp"

using namespace rack;

#define plugin "SynthKit"

struct JLHHexScrew : SVGScrew {
  JLHHexScrew() {
    sw->setSVG(SVG::load(assetPlugin(plugin, "res/JLHHexScrew.svg")));
    box.size = sw->box.size;
  }
};
