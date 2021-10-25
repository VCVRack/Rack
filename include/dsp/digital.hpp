#pragma once
#include <dsp/common.hpp>


namespace rack {
namespace dsp {


/** Detects when a boolean changes from false to true */
struct BooleanTrigger {
	bool state = true;

	void reset() {
		state = true;
	}

	bool process(bool state) {
		bool triggered = (state && !this->state);
		this->state = state;
		return triggered;
	}
};


/** Turns HIGH when value reaches a threshold (default 0.f), turns LOW when value reaches a threshold (default 1.f).
*/
template <typename T = float>
struct TSchmittTrigger {
	T state;
	TSchmittTrigger() {
		reset();
	}
	void reset() {
		state = T::mask();
	}
	T process(T in, T offThreshold = 0.f, T onThreshold = 1.f) {
		T on = (in >= onThreshold);
		T off = (in <= offThreshold);
		T triggered = ~state & on;
		state = on | (state & ~off);
		return triggered;
	}
	T isHigh() {
		return state;
	}
};


template <>
struct TSchmittTrigger<float> {
	bool state = true;

	void reset() {
		state = true;
	}

	/** Updates the state of the Schmitt Trigger given a value.
	Returns true if triggered, i.e. the value increases from 0 to 1.
	If different trigger thresholds are needed, use

		process(in, 0.1f, 2.f)

	for example.
	*/
	bool process(float in, float offThreshold = 0.f, float onThreshold = 1.f) {
		if (state) {
			// HIGH to LOW
			if (in <= offThreshold) {
				state = false;
			}
		}
		else {
			// LOW to HIGH
			if (in >= onThreshold) {
				state = true;
				return true;
			}
		}
		return false;
	}

	bool isHigh() {
		return state;
	}
};

typedef TSchmittTrigger<> SchmittTrigger;


/** When triggered, holds a high value for a specified time before going low again */
struct PulseGenerator {
	float remaining = 0.f;

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


/** Accumulates a timer when process() is called. */
template <typename T = float>
struct TTimer {
	T time = 0.f;

	void reset() {
		time = 0.f;
	}

	/** Returns the time since last reset or initialization. */
	T process(T deltaTime) {
		time += deltaTime;
		return time;
	}

	T getTime() {
		return time;
	}
};

typedef TTimer<> Timer;


/** Counts calls to process(), returning true every `division` calls.
Example:

	if (divider.process()) {
		// Runs every `division` calls
	}
*/
struct ClockDivider {
	uint32_t clock = 0;
	uint32_t division = 1;

	void reset() {
		clock = 0;
	}

	void setDivision(uint32_t division) {
		this->division = division;
	}

	uint32_t getDivision() {
		return division;
	}

	uint32_t getClock() {
		return clock;
	}

	/** Returns true when the clock reaches `division` and resets. */
	bool process() {
		clock++;
		if (clock >= division) {
			clock = 0;
			return true;
		}
		return false;
	}
};


} // namespace dsp
} // namespace rack
