#pragma once
#include "../common.hpp"
#include "math.hpp"
#include <jansson.h>


namespace rack {


/** Rack-specific GUI widgets and functions that control and offer feedback for the rack state.
*/
namespace app {


static const char APP_NAME[] = "VCV Rack";
static const char APP_VERSION[] = TOSTRING(VERSION);
static const char API_URL[] = "https://api.vcvrack.com";

static const float SVG_DPI = 75.0;
static const float MM_PER_IN = 25.4;


/** Converts inch measurements to pixels */
inline float in2px(float in) {
	return in * SVG_DPI;
}

inline math::Vec in2px(math::Vec in) {
	return in.mult(SVG_DPI);
}

/** Converts millimeter measurements to pixels */
inline float mm2px(float mm) {
	return mm * (SVG_DPI / MM_PER_IN);
}

inline math::Vec mm2px(math::Vec mm) {
	return mm.mult(SVG_DPI / MM_PER_IN);
}


// A 1HPx3U module should be 15x380 pixels. Thus the width of a module should be a factor of 15.
static const float RACK_GRID_WIDTH = 15;
static const float RACK_GRID_HEIGHT = 380;
static const math::Vec RACK_GRID_SIZE = math::Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT);


} // namespace app
} // namespace rack
