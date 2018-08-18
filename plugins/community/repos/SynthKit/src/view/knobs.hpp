#pragma once

#include "asset.hpp"
#include "rack.hpp"

using namespace rack;

#define plugin "SynthKit"

struct Knob19 : RoundKnob {
  Knob19() {
    setSVG(SVG::load(assetPlugin(plugin, "res/Knob19.svg")));
  }
};

struct Knob30 : RoundKnob {
  Knob30() {
    setSVG(SVG::load(assetPlugin(plugin, "res/Knob30.svg")));
  }
};

struct Knob19Snap : Knob19 {
  Knob19Snap() {
    snap = true;
  }
};

struct Knob30Snap : Knob30 {
  Knob30Snap() {
    snap = true;
  }
};
