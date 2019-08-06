#pragma once
#include <common.hpp>
#include <jansson.h>

#pragma GCC diagnostic push
#ifndef __clang__
	#pragma GCC diagnostic ignored "-Wsuggest-override"
#endif
#include <RtAudio.h>
#pragma GCC diagnostic pop


namespace rack {


/** Audio driver
*/
namespace audio {


struct Port {
	// Stream properties
	int driverId = 0;
	int deviceId = -1;
	int offset = 0;
	int maxChannels = 8;
	int sampleRate = 44100;
	int blockSize = 256;
	int numOutputs = 0;
	int numInputs = 0;
	RtAudio* rtAudio = NULL;
	/** Cached */
	RtAudio::DeviceInfo deviceInfo;

	Port();
	virtual ~Port();

	std::vector<int> getDriverIds();
	std::string getDriverName(int driverId);
	void setDriverId(int driverId);

	int getDeviceCount();
	bool getDeviceInfo(int deviceId, RtAudio::DeviceInfo* deviceInfo);
	/** Returns the number of inputs or outputs, whichever is greater */
	int getDeviceChannels(int deviceId);
	std::string getDeviceName(int deviceId);
	std::string getDeviceDetail(int deviceId, int offset);
	void setDeviceId(int deviceId, int offset);

	std::vector<int> getSampleRates();
	void setSampleRate(int sampleRate);
	std::vector<int> getBlockSizes();
	void setBlockSize(int blockSize);

	void setChannels(int numOutputs, int numInputs);

	/** Must close the stream before opening */
	void openStream();
	void closeStream();

	virtual void processStream(const float* input, float* output, int frames) {}
	virtual void onCloseStream() {}
	virtual void onOpenStream() {}
	virtual void onChannelsChange() {}
	json_t* toJson();
	void fromJson(json_t* rootJ);
};


} // namespace audio
} // namespace rack
