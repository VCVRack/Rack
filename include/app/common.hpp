#pragma once
#include <jansson.h>

#include <common.hpp>
#include <math.hpp>
#include <ui/common.hpp>


namespace rack {


/** Rack's custom UI widgets that control the Rack state and engine.
*/
namespace app {


// A 1HPx3U module should be 15x380 pixels. Thus the width of a module should be a factor of 15.
static const float RACK_GRID_WIDTH = 15;
static const float RACK_GRID_HEIGHT = 380;
static const math::Vec RACK_GRID_SIZE = math::Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
static const math::Vec RACK_OFFSET = RACK_GRID_SIZE.mult(math::Vec(2000, 100));


} // namespace app
} // namespace rack
