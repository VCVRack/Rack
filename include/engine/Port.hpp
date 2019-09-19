#pragma once
#include <common.hpp>
#include <engine/Light.hpp>


namespace rack {
namespace engine {


/** This is inspired by the number of MIDI channels. */
static const int PORT_MAX_CHANNELS = 16;


struct alignas(32) Port {
	/** Voltage of the port. */
	union {
		/** Unstable API. Use getVoltage() and setVoltage() instead. */
		float voltages[PORT_MAX_CHANNELS] = {};
		/** DEPRECATED. Unstable API. Use getVoltage() and setVoltage() instead. */
		float value;
	};
	union {
		/** Number of polyphonic channels
		Unstable API. Use set/getChannels() instead.
		May be 0 to PORT_MAX_CHANNELS.
		*/
		uint8_t channels = 0;
		/** DEPRECATED. Unstable API. Use isConnected() instead. */
		uint8_t active;
	};
	/** For rendering plug lights on cables.
	Green for positive, red for negative, and blue for polyphonic.
	*/
	Light plugLights[3];

	/** Sets the voltage of the given channel. */
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
		return isMonophonic() ? getVoltage(0) : getVoltage(channel);
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
	*/
	float* getVoltages(int firstChannel = 0) {
		return &voltages[firstChannel];
	}

	/** Copies the port's voltages to an array of size at least `channels`. */
	void readVoltages(float* v) {
		for (int c = 0; c < channels; c++) {
			v[c] = voltages[c];
		}
	}

	/** Copies an array of size at least `channels` to the port's voltages.
	Remember to set the number of channels *before* calling this method.
	*/
	void writeVoltages(const float* v) {
		for (int c = 0; c < channels; c++) {
			voltages[c] = v[c];
		}
	}

	/** Sets all voltages to 0. */
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

	template <typename T>
	T getVoltageSimd(int firstChannel) {
		return T::load(&voltages[firstChannel]);
	}

	template <typename T>
	T getPolyVoltageSimd(int firstChannel) {
		return isMonophonic() ? getVoltage(0) : getVoltageSimd<T>(firstChannel);
	}

	template <typename T>
	T getNormalVoltageSimd(T normalVoltage, int firstChannel) {
		return isConnected() ? getVoltageSimd<T>(firstChannel) : normalVoltage;
	}

	template <typename T>
	T getNormalPolyVoltageSimd(T normalVoltage, int firstChannel) {
		return isConnected() ? getPolyVoltageSimd<T>(firstChannel) : normalVoltage;
	}

	template <typename T>
	void setVoltageSimd(T voltage, int firstChannel) {
		voltage.store(&voltages[firstChannel]);
	}

	/** Sets the number of polyphony channels.
	Also clears voltages of higher channels.
	If disconnected, this does nothing (`channels` remains 0).
	If 0 is given, `channels` is set to 1 but all voltages are cleared.
	*/
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

	/** Returns the number of channels.
	If the port is disconnected, it has 0 channels.
	*/
	int getChannels() {
		return channels;
	}

	/** Returns whether a cable is connected to the Port.
	You can use this for skipping code that generates output voltages.
	*/
	bool isConnected() {
		return channels > 0;
	}

	/** Returns whether the cable exists and has 1 channel. */
	bool isMonophonic() {
		return channels == 1;
	}

	/** Returns whether the cable exists and has more than 1 channel. */
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
