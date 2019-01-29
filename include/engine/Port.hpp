#pragma once
#include "common.hpp"
#include "engine/Light.hpp"


namespace rack {
namespace engine {


static const int PORT_MAX_CHANNELS = 16;


struct Port {
	/** Voltage of the port */
	union {
		/** Unstable API. Use set/getVoltage() instead. */
		float voltages[PORT_MAX_CHANNELS] = {};
		/** DEPRECATED. Unstable API. Use getVoltage() and setVoltage() instead. */
		float value;
	};
	/** Number of polyphonic channels
	Unstable API. Use set/getChannels() instead.
	May be 0 to PORT_MAX_CHANNELS.
	*/
	uint8_t channels = 1;
	/** Unstable API. Use isConnected() instead. */
	bool active;
	/** For rendering plug lights on cables
	Green for positive, red for negative, and blue for polyphonic
	*/
	Light plugLights[3];

	void setVoltage(float voltage, int channel = 0) {
		voltages[channel] = voltage;
	}

	float getVoltage(int channel = 0) {
		return voltages[channel];
	}

	/** Returns the voltage if `channel` is a valid channel, otherwise returns the first voltage (channel 0) */
	float getPolyVoltage(int channel) {
		return (channel < channels) ? getVoltage(channel) : getVoltage(0);
	}

	/** Returns the voltage if a cable is connected, otherwise returns the given normal voltage */
	float getNormalVoltage(float normalVoltage, int channel = 0) {
		return isConnected() ? getVoltage(channel) : normalVoltage;
	}

	float getNormalPolyVoltage(float normalVoltage, int channel) {
		return isConnected() ? getPolyVoltage(channel) : normalVoltage;
	}

	void setChannels(int channels) {
		// Set higher channel voltages to 0
		for (int c = channels; c < this->channels; c++) {
			voltages[c] = 0.f;
		}
		this->channels = channels;
	}

	int getChannels() {
		return channels;
	}

	bool isConnected() {
		return active;
	}

	void step();

	DEPRECATED float normalize(float normalVoltage) {
		return getNormalVoltage(normalVoltage);
	}
};


struct Output : Port {};
struct Input : Port {};


} // namespace engine
} // namespace rack
