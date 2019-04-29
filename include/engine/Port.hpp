#pragma once
#include "common.hpp"
#include "engine/Light.hpp"


namespace rack {
namespace engine {


static const int PORT_MAX_CHANNELS = 16;


struct alignas(32) Port {
	/** Voltage of the port. */
	union {
		/** Unstable API. Use getVoltage() and setVoltage() instead. */
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
	bool active = false;
	/** For rendering plug lights on cables.
	Green for positive, red for negative, and blue for polyphonic.
	*/
	Light plugLights[3];

	void setVoltage(float voltage, int channel = 0) {
		voltages[channel] = voltage;
	}

	/** Returns the voltage of the given channel.
	Because of proper bookkeeping, all channels higher than the input port's number of channels should be 0V.
	*/
	float getVoltage(int channel = 0) {
		return voltages[channel];
	}

	/** Returns the given channel's voltage if the port is polyphonic, otherwise returns the first voltage (channel 0). */
	float getPolyVoltage(int channel) {
		return (channels == 1) ? getVoltage(0) : getVoltage(channel);
	}

	/** Returns the voltage if a cable is connected, otherwise returns the given normal voltage. */
	float getNormalVoltage(float normalVoltage, int channel = 0) {
		return isConnected() ? getVoltage(channel) : normalVoltage;
	}

	float getNormalPolyVoltage(float normalVoltage, int channel) {
		return isConnected() ? getPolyVoltage(channel) : normalVoltage;
	}

	/** Reads all voltage values from an array of size `channels` */
	void setVoltages(const float *voltages) {
		for (int c = 0; c < channels; c++) {
			this->voltages[c] = voltages[c];
		}
	}

	/** Writes all voltage values to an array of size `channels` */
	void getVoltages(float *voltages) {
		for (int c = 0; c < channels; c++) {
			voltages[c] = this->voltages[c];
		}
	}

	/** Sets the number of polyphony channels. */
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

	/** Returns if a cable is connected to the Port.
	You can use this for skipping code that generates output voltages.
	*/
	bool isConnected() {
		return active;
	}

	void process(float deltaTime);

	/** Use getNormalVoltage() instead. */
	DEPRECATED float normalize(float normalVoltage) {
		return getNormalVoltage(normalVoltage);
	}
};


struct Output : Port {};

struct Input : Port {};


} // namespace engine
} // namespace rack
