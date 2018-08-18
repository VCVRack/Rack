#pragma once

#include "asset.hpp"
#include "rack.hpp"

using namespace rack;

#define plugin "SynthKit"

struct RCKnobRed : RoundKnob {
  RCKnobRed() {
    setSVG(SVG::load(assetPlugin(plugin, "res/KnobRed.svg")));
  }
};

struct RCKnobRedSnap : RCKnobRed {
  RCKnobRedSnap ( ) {
    snap = true;
  }
};

struct RCKnobRedLarge : RoundKnob {
  RCKnobRedLarge() {
    setSVG(SVG::load(assetPlugin(plugin, "res/KnobRedLarge.svg")));
  }
};

struct RCKnobRedLargeSnap : RCKnobRedLarge {
  RCKnobRedLargeSnap ( ) {
    snap = true;
  }
};

struct RCKnobRedSmall : RoundKnob {
  RCKnobRedSmall() {
    setSVG(SVG::load(assetPlugin(plugin, "res/KnobRedSmall.svg")));
  }
};

struct RCKnobRedSmallSnap : RCKnobRedSmall {
  RCKnobRedSmallSnap() {
    snap = true;
  }
};

struct RCKnobWhiteLarge : RoundKnob {
  RCKnobWhiteLarge() {
    setSVG(SVG::load(assetPlugin(plugin, "res/KnobWhiteLarge.svg")));
  }
};

struct RCKnobWhiteLargeSnap : RCKnobWhiteLarge {
  RCKnobWhiteLargeSnap() {
    snap = true;
  }
};

struct RCKnobWhite : RoundKnob {
  RCKnobWhite() {
    setSVG(SVG::load(assetPlugin(plugin, "res/KnobWhite.svg")));
  }
};

struct RCKnobWhiteSnap : RCKnobWhite {
  RCKnobWhiteSnap() {
    snap = true;
  }
};

struct RCKnobWhiteSmall : RoundKnob {
  RCKnobWhiteSmall() {
    setSVG(SVG::load(assetPlugin(plugin, "res/KnobWhiteSmall.svg")));
  }
};

struct RCKnobWhiteSmallSnap : RCKnobWhiteSmall {
  RCKnobWhiteSmallSnap() {
    snap = true;
  }
};
