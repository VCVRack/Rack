#pragma once
#include "../common.hpp"
#include "math.hpp"
#include <jansson.h>


namespace rack {


extern const std::string APP_NAME;
extern const std::string APP_VERSION;
extern const std::string API_HOST;

static const float APP_SVG_DPI = 75.0;
static const float MM_PER_IN = 25.4;


/** Converts inch measurements to pixels */
inline float in2px(float in) {
	return in * APP_SVG_DPI;
}

inline math::Vec in2px(math::Vec in) {
	return in.mult(APP_SVG_DPI);
}

/** Converts millimeter measurements to pixels */
inline float mm2px(float mm) {
	return mm * (APP_SVG_DPI / MM_PER_IN);
}

inline math::Vec mm2px(math::Vec mm) {
	return mm.mult(APP_SVG_DPI / MM_PER_IN);
}


// A 1HPx3U module should be 15x380 pixels. Thus the width of a module should be a factor of 15.
static const float RACK_GRID_WIDTH = 15;
static const float RACK_GRID_HEIGHT = 380;
static const math::Vec RACK_GRID_SIZE = math::Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

static const std::string PRESET_FILTERS = "VCV Rack module preset (.vcvm):vcvm";


} // namespace rack
