#pragma once
#include "common.hpp"
#include "engine/Light.hpp"


namespace rack {


static const int PORT_MAX_CHANNELS = 16;


struct Port {
	/** Voltage of the port */
	union {
		/** Accessing this directly is deprecated.
		Use getVoltage() and setVoltage() instead
		*/
		float value;
		float values[PORT_MAX_CHANNELS] = {};
	};
	/** Number of polyphonic channels */
	int numChannels = 1;
	/** Whether a cable is plugged in */
	bool active = false;
	Light plugLights[2];

	float getVoltage(int index = 0) {
		return values[index];
	}
	void setVoltage(float voltage, int index = 0) {
		values[index] = voltage;
	}
};


} // namespace rack
