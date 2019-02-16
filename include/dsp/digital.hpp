#pragma once
#include "dsp/common.hpp"


namespace rack {
namespace dsp {


/** Turns HIGH when value reaches 1.f, turns LOW when value reaches 0.f. */
struct SchmittTrigger {
	enum State {
		LOW,
		HIGH,
		UNKNOWN
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
		process(math::rescale(in, low, high, 0.f, 1.f))
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


/** Detects when a boolean changes from false to true */
struct BooleanTrigger {
	bool state;

	BooleanTrigger() {
		reset();
	}

	void reset() {
		state = true;
	}

	bool process(bool state) {
		bool triggered = (state && !this->state);
		this->state = state;
		return triggered;
	}
};


/** When triggered, holds a high value for a specified time before going low again */
struct PulseGenerator {
	float remaining;

	PulseGenerator() {
		reset();
	}

	/** Immediately disables the pulse */
	void reset() {
		remaining = 0.f;
	}

	/** Advances the state by `deltaTime`. Returns whether the pulse is in the HIGH state. */
	bool process(float deltaTime) {
		if (remaining > 0.f) {
			remaining -= deltaTime;
			return true;
		}
		return false;
	}

	/** Begins a trigger with the given `duration`. */
	void trigger(float duration = 1e-3f) {
		// Keep the previous pulse if the existing pulse will be held longer than the currently requested one.
		if (duration > remaining) {
			remaining = duration;
		}
	}
};


struct Timer {
	float time;

	Timer() {
		reset();
	}

	void reset() {
		time = 0.f;
	}

	float process(float deltaTime) {
		time += deltaTime;
		return time;
	}
};


/** Counts the number of `process()` calls.
If `period > 0`, `count` is reset to 0 when that number is reached.
Useful for clock dividing and waiting to fill a fixed buffer.
*/
struct Counter {
	int count;
	int period = 0;

	Counter() {
		reset();
	}

	void reset() {
		count = 0;
	}

	void setPeriod(int period) {
		this->period = period;
		reset();
	}

	/** Returns true when the counter reaches `period` and resets. */
	bool process() {
		count++;
		if (count == period) {
			count = 0;
			return true;
		}
		return false;
	}
};


} // namespace dsp
} // namespace rack
