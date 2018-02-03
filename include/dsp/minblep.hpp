#pragma once

#include "util/math.hpp"


namespace rack {

// Pre-made minBLEP samples in minBLEP.cpp
extern const float minblep_16_32[];


template<int ZERO_CROSSINGS>
struct MinBLEP {
	float buf[2*ZERO_CROSSINGS] = {};
	int pos = 0;
	const float *minblep;
	int oversample;

	/** Places a discontinuity with magnitude dx at -1 < p <= 0 relative to the current frame */
	void jump(float p, float dx) {
		if (p <= -1 || 0 < p)
			return;
		for (int j = 0; j < 2*ZERO_CROSSINGS; j++) {
			float minblepIndex = ((float)j - p) * oversample;
			int index = (pos + j) % (2*ZERO_CROSSINGS);
			buf[index] += dx * (-1.0 + interpolateLinear(minblep, minblepIndex));
		}
	}
	float shift() {
		float v = buf[pos];
		buf[pos] = 0.0;
		pos = (pos + 1) % (2*ZERO_CROSSINGS);
		return v;
	}
};

} // namespace rack
