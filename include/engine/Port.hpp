#pragma once
#include <common.hpp>
#include <engine/Light.hpp>


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

	/** Returns a pointer to the array of voltages beginning with firstChannel.
	The pointer can be used for reading and writing.
	Useful for SIMD.
	*/
	float *getVoltages(int firstChannel = 0) {
		return &voltages[firstChannel];
	}

	/** Copies the port's voltages to an array of size at least `channels`. */
	void readVoltages(float *v) {
		for (int c = 0; c < channels; c++) {
			v[c] = voltages[c];
		}
	}

	/** Copies an array of size at least `channels` to the port's voltages.
	Remember to set the number of channels *before* calling this method.
	*/
	void writeVoltages(const float *v) {
		for (int c = 0; c < channels; c++) {
			voltages[c] = v[c];
		}
	}

	void clearVoltages() {
		for (int c = 0; c < channels; c++) {
			voltages[c] = 0.f;
		}
	}

	/** Returns the sum of all voltages. */
	float getVoltageSum() {
		float sum = 0.f;
		for (int c = 0; c < channels; c++) {
			sum += voltages[c];
		}
		return sum;
	}

	/** Sets the number of polyphony channels. */
	void setChannels(int channels) {
		// If disconnected, keep the number of channels at 0.
		if (this->channels == 0) {
			return;
		}
		// Set higher channel voltages to 0
		for (int c = channels; c < this->channels; c++) {
			voltages[c] = 0.f;
		}
		// Don't allow caller to set port as disconnected
		if (channels == 0) {
			channels = 1;
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
		return channels > 0;
	}

	bool isMonophonic() {
		return channels == 1;
	}

	bool isPolyphonic() {
		return channels > 1;
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
