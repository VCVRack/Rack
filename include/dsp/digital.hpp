#pragma once

#include "util/math.hpp"


namespace rack {


/** Turns high when value reaches 1, turns low when value reaches 0 */
struct SchmittTrigger {
	// UNKNOWN is used to represent a stable state when the previous state is not yet set
	enum State {
		UNKNOWN,
		LOW,
		HIGH
	};
	State state = UNKNOWN;
	/** Updates the state of the Schmitt Trigger given a value.
	Returns true if triggered, i.e. the value increases from 0 to 1.
	If different trigger thresholds are needed, use
		process(rescale(in, low, high, 0.f, 1.f))
	for example.
	*/
	bool process(float in) {
		switch (state) {
			case LOW:
				if (in >= 1.f) {
					state = HIGH;
					return true;
				}
				break;
			case HIGH:
				if (in <= 0.f) {
					state = LOW;
				}
				break;
			default:
				if (in >= 1.f) {
					state = HIGH;
				}
				else if (in <= 0.f) {
					state = LOW;
				}
				break;
		}
		return false;
	}
	bool isHigh() {
		return state == HIGH;
	}
	void reset() {
		state = UNKNOWN;
	}
};


/** When triggered, holds a high value for a specified time before going low again */
struct PulseGenerator {
	float time = 0.f;
	float pulseTime = 0.f;
	bool process(float deltaTime) {
		time += deltaTime;
		return time < pulseTime;
	}
	void trigger(float pulseTime) {
		// Keep the previous pulseTime if the existing pulse would be held longer than the currently requested one.
		if (time + pulseTime >= this->pulseTime) {
			time = 0.f;
			this->pulseTime = pulseTime;
		}
	}
};


} // namespace rack
