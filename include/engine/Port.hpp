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
	/** Number of polyphonic channels
	May be 0 to PORT_MAX_CHANNELS.
	*/
	int channels = 1;
	/** Whether a cable is plugged in */
	bool active = false;
	Light plugLights[2];

	float getVoltage(int channel = 0) {
		return values[channel];
	}

	void setVoltage(float voltage, int channel = 0) {
		values[channel] = voltage;
	}
};


} // namespace rack
