#pragma once
#include "common.hpp"
#include "engine/Light.hpp"


namespace rack {


struct Input {
	/** Voltage of the port, zero if not plugged in. Read-only by Module */
	float value = 0.f;
	/** Whether a wire is plugged in */
	bool active = false;
	Light plugLights[2];

	/** Returns the value if a wire is plugged in, otherwise returns the given default value */
	float normalize(float normalValue) {
		return active ? value : normalValue;
	}
};


} // namespace rack
