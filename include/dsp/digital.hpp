#pragma once

#include "math.hpp"


namespace rack {


/** Turns high when value reaches the high threshold, turns low when value reaches the low threshold */
struct SchmittTrigger {
	// UNKNOWN is used to represent a stable state when the previous state is not yet set
	enum {UNKNOWN, LOW, HIGH} state = UNKNOWN;
	float low = 0.0;
	float high = 1.0;
	void setThresholds(float low, float high) {
		this->low = low;
		this->high = high;
	}
	/** Returns true if triggered */
	bool process(float in) {
		switch (state) {
			case LOW:
				if (in >= high) {
					state = HIGH;
					return true;
				}
				break;
			case HIGH:
				if (in <= low) {
					state = LOW;
				}
				break;
			default:
				if (in >= high) {
					state = HIGH;
				}
				else if (in <= low) {
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
	float time = 0.0;
	float pulseTime = 0.0;
	bool process(float deltaTime) {
		time += deltaTime;
		return time < pulseTime;
	}
	void trigger(float pulseTime) {
		// Keep the previous pulseTime if the existing pulse would be held longer than the currently requested one.
		if (time + pulseTime >= this->pulseTime) {
			time = 0.0;
			this->pulseTime = pulseTime;
		}
	}
};


} // namespace rack
