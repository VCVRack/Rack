#pragma once

#include "asset.hpp"
#include "rack.hpp"

using namespace rack;

#define plugin "SynthKit"

struct RCJackSmallRed : SVGPort {
  RCJackSmallRed() {
    background->svg = SVG::load(assetPlugin(plugin, "res/JackSmallRed.svg"));
    background->wrap();
    box.size = background->box.size;
  }
};

struct RCJackSmallGrey : SVGPort {
  RCJackSmallGrey() {
    background->svg = SVG::load(assetPlugin(plugin, "res/JackSmallGrey.svg"));
    background->wrap();
    box.size = background->box.size;
  }
};

struct RCJackSmallDark : SVGPort {
  RCJackSmallDark() {
    background->svg = SVG::load(assetPlugin(plugin, "res/JackSmallDark.svg"));
    background->wrap();
    box.size = background->box.size;
  }
};


struct RCJackSmallLight : SVGPort {
  RCJackSmallLight() {
    background->svg = SVG::load(assetPlugin(plugin, "res/JackSmallLight.svg"));
    background->wrap();
    box.size = background->box.size;
  }
};
