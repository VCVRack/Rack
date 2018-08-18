#pragma once

#include <cstdint>

#include "asset.hpp"
#include "rack.hpp"

using namespace rack;

#define plugin "SynthKit"

struct RCSlider : SVGFader {
	RCSlider() {
		Vec margin = Vec(4, 4);
		maxHandlePos = Vec(1.3, -7).plus(margin);
		minHandlePos = Vec(1.3, 79).plus(margin);
		background->svg = SVG::load(assetPlugin(plugin,"res/Slider.svg"));
		background->wrap();
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));
		handle->svg = SVG::load(assetPlugin(plugin,"res/SliderHandle.svg"));
		handle->wrap();
	}
};
