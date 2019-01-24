#pragma once
#include "common.hpp"
#include "engine/Light.hpp"


namespace rack {


static const int PORT_MAX_CHANNELS = 16;


struct Port {
	/** Voltage of the port */
	union {
		float values[PORT_MAX_CHANNELS] = {};
		/** DEPRECATED. Use getVoltage() and setVoltage() instead. */
		float value;
	};
	/** Number of polyphonic channels
	May be 0 to PORT_MAX_CHANNELS.
	*/
	union {
		uint8_t channels = 1;
		/** DEPRECATED. Use isActive() instead. */
		bool active;
	};
	/** For rendering plug lights on cables
	Green for positive, red for negative, and blue for polyphonic
	*/
	Light plugLights[3];

	float getVoltage(int channel = 0) {
		return values[channel];
	}

	void setVoltage(float voltage, int channel = 0) {
		values[channel] = voltage;
	}

	void setChannels(int channels) {
		// Set higher channel values to 0
		for (int c = channels; c < this->channels; c++) {
			values[c] = 0.f;
		}
		this->channels = channels;
	}

	int getChannels() {
		return channels;
	}

	bool isActive() {
		return channels;
	}

	void step();
};


} // namespace rack
