#pragma once
#include <jansson.h>

#include <common.hpp>
#include <ui/common.hpp>
#include <svg.hpp> // for mm2px(), etc


namespace rack {


/** Rack-specific GUI widgets and functions that control and offer feedback for the rack state.
*/
namespace app {


// A 1HPx3U module should be 15x380 pixels. Thus the width of a module should be a factor of 15.
static const float RACK_GRID_WIDTH = 15;
static const float RACK_GRID_HEIGHT = 380;
static const math::Vec RACK_GRID_SIZE = math::Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
static const math::Vec RACK_OFFSET = RACK_GRID_SIZE.mult(math::Vec(2000, 100));


} // namespace app
} // namespace rack
