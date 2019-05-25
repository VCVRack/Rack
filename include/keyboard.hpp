#pragma once
#include <common.hpp>


namespace rack {


/** Computer keyboard MIDI driver
*/
namespace keyboard {


void init();
void press(int key);
void release(int key);


} // namespace keyboard
} // namespace rack
