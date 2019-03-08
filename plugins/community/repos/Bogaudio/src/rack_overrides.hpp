#pragma once

#include "rack.hpp"

using namespace rack;

namespace bogaudio {

struct Trigger : SchmittTrigger {
	float _highThreshold;
	float _lowThreshold;

	Trigger(float highThreshold = 1.0f, float lowThreshold = 0.1f)
	: _highThreshold(highThreshold)
	, _lowThreshold(lowThreshold)
	{
		reset();
	}

	bool process(float in) {
		switch (state) {
			case LOW:
				if (in >= _highThreshold) {
					state = HIGH;
					return true;
				}
				break;
			case HIGH:
				if (in <= _lowThreshold) {
					state = LOW;
				}
				break;
			default:
				if (in >= _highThreshold) {
					state = HIGH;
				}
				else if (in <= _lowThreshold) {
					state = LOW;
				}
				break;
		}
		return false;
	}
};

} // namespace bogaudio
