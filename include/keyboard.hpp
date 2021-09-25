#pragma once
#include <common.hpp>
#include <math.hpp>


namespace rack {
/** Computer keyboard MIDI driver */
namespace keyboard {


void init();
void press(int key);
void release(int key);
/** pos is in the unit box. */
void mouseMove(math::Vec pos);


} // namespace keyboard
} // namespace rack
