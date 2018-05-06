#pragma once

#include <stdlib.h>


namespace rack {

/** Useful for storing arrays of samples in ring buffers and casting them to `float*` to be used by interleaved processors, like SampleRateConverter */
template <size_t CHANNELS>
struct Frame {
	float samples[CHANNELS];
};

} // namespace rack
