#pragma once
#include <dsp/common.hpp>


namespace rack {
namespace dsp {


/** Computes the minimum-phase bandlimited step (MinBLEP)
z: number of zero-crossings
o: oversample factor
output: must be length `2 * z * o`.
https://www.cs.cmu.edu/~eli/papers/icmc01-hardsync.pdf
*/
void minBlepImpulse(int z, int o, float* output);


template <int Z, int O, typename T = float>
struct MinBlepGenerator {
	T buf[2 * Z] = {};
	int pos = 0;
	float impulse[2 * Z * O + 1];

	MinBlepGenerator() {
		minBlepImpulse(Z, O, impulse);
		impulse[2 * Z * O] = 1.f;
	}

	/** Places a discontinuity with magnitude `x` at -1 < p <= 0 relative to the current frame */
	void insertDiscontinuity(float p, T x) {
		if (!(-1 < p && p <= 0))
			return;
		for (int j = 0; j < 2 * Z; j++) {
			float minBlepIndex = ((float)j - p) * O;
			int index = (pos + j) % (2 * Z);
			buf[index] += x * (-1.f + math::interpolateLinear(impulse, minBlepIndex));
		}
	}

	T process() {
		T v = buf[pos];
		buf[pos] = T(0);
		pos = (pos + 1) % (2 * Z);
		return v;
	}
};


} // namespace dsp
} // namespace rack
