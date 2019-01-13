#pragma once
#include "dsp/common.hpp"


namespace rack {
namespace dsp {


template<int ZERO_CROSSINGS>
struct MinBLEP {
	float buf[2 * ZERO_CROSSINGS] = {};
	int pos = 0;
	const float *minblep;
	int oversample;

	/** Places a discontinuity with magnitude `x` at -1 < p <= 0 relative to the current frame */
	void insertDiscontinuity(float p, float x) {
		if (!(-1 < p && p <= 0))
			return;
		for (int j = 0; j < 2 * ZERO_CROSSINGS; j++) {
			float minblepIndex = ((float)j - p) * oversample;
			int index = (pos + j) % (2 * ZERO_CROSSINGS);
			buf[index] += x * (-1.f + math::interpolateLinear(minblep, minblepIndex));
		}
	}

	float process() {
		float v = buf[pos];
		buf[pos] = 0.f;
		pos = (pos + 1) % (2 * ZERO_CROSSINGS);
		return v;
	}
};


} // namespace dsp
} // namespace rack
