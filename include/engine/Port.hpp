#pragma once
#include "common.hpp"
#include "engine/Light.hpp"


namespace rack {


static const int PORT_MAX_CHANNELS = 16;


struct Port {
	/** Voltage of the port */
	union {
		float value;
		float values[PORT_MAX_CHANNELS];
	};
	/** Number of polyphonic channels */
	int numChannels = 1;
	/** Whether a wire is plugged in */
	bool active = false;
	Light plugLights[2];

	float getValue(int index = 0) {
		return values[index];
	}

	void setValue(float value, int index = 0) {
		this->values[index] = value;
	}
};


} // namespace rack
