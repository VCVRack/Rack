#include "rack.hpp"
using namespace rack;

#include "dsp/functions.hpp"
//#include "dsp/resampler.hpp"
#include "dsp/ode.hpp"

namespace rack_plugin_Bark {

/**
Fundamental VCF by Andrew Best
*/

inline float clip(float x) {
	return tanhf(x);
}

struct LadderFilter {
	float omega0;
	float resonance = 1.0f;
	float state[4];
	float input;
	float lowpass;
	float highpass;

	LadderFilter() {
		reset();
		setCutoff(0.f);
	}

	void reset() {
		for (int i = 0; i < 4; i++) {
			state[i] = 0.f;
		}
	}

	void setCutoff(float cutoff) {
		omega0 = 2.f*M_PI * cutoff;
	}

	void process(float input, float dt) {
		ode::stepRK4(0.f, dt, state, 4, [&](float t, const float x[], float dxdt[]) {
			float inputc = clip(input - resonance * x[3]);
			float yc0 = clip(x[0]);
			float yc1 = clip(x[1]);
			float yc2 = clip(x[2]);
			float yc3 = clip(x[3]);

			dxdt[0] = omega0 * (inputc - yc0);
			dxdt[1] = omega0 * (yc0 - yc1);
			dxdt[2] = omega0 * (yc1 - yc2);
			dxdt[3] = omega0 * (yc2 - yc3);
		});

		lowpass = state[3];
		// TODO This is incorrect when `resonance > 0`. Is the math wrong?
		highpass = clip((input - resonance * state[3]) - 4 * state[0] + 6 * state[1] - 4 * state[2] + state[3]);
	}
};

} // namespace rack_plugin_Bark
