#pragma once

#include "util/math.hpp"


namespace rack {


/** Turns HIGH when value reaches 1.f, turns LOW when value reaches 0.f. */
struct SchmittTrigger {
	// UNKNOWN is used to represent a stable state when the previous state is not yet set
	enum State {
		UNKNOWN,
		LOW,
		HIGH
	};
	State state;

	SchmittTrigger() {
		reset();
	}
	void reset() {
		state = UNKNOWN;
	}
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
};


/** When triggered, holds a high value for a specified time before going low again */
struct PulseGenerator {
	float time;
	float triggerDuration;

	PulseGenerator() {
		reset();
	}
	/** Immediately resets the state to LOW */
	void reset() {
		time = 0.f;
		triggerDuration = 0.f;
	}
	/** Advances the state by `deltaTime`. Returns whether the pulse is in the HIGH state. */
	bool process(float deltaTime) {
		time += deltaTime;
		return time < triggerDuration;
	}
	/** Begins a trigger with the given `triggerDuration`. */
	void trigger(float triggerDuration) {
		// Keep the previous triggerDuration if the existing pulse would be held longer than the currently requested one.
		if (time + triggerDuration >= this->triggerDuration) {
			time = 0.f;
			this->triggerDuration = triggerDuration;
		}
	}
};


} // namespace rack
