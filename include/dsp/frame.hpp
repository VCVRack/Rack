#pragma once
#include "dsp/common.hpp"


namespace rack {
namespace dsp {


/** Useful for storing arrays of samples in ring buffers and casting them to `float*` to be used by interleaved processors, like SampleRateConverter */
template <size_t CHANNELS>
struct Frame {
	float samples[CHANNELS];
};


} // namespace dsp
} // namespace rack
