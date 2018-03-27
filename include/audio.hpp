#pragma once

#include <jansson.h>

#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wsuggest-override"
#endif
#include <RtAudio.h>
#pragma GCC diagnostic pop


namespace rack {


struct AudioIO {
	// Stream properties
	int driver = 0;
	int device = -1;
	int offset = 0;
	int maxChannels = 8;
	int sampleRate = 44100;
	int blockSize = 256;
	int numOutputs = 0;
	int numInputs = 0;
	RtAudio *rtAudio = NULL;
	/** Cached */
	RtAudio::DeviceInfo deviceInfo;

	AudioIO();
	virtual ~AudioIO();

	std::vector<int> getDrivers();
	std::string getDriverName(int driver);
	void setDriver(int driver);

	int getDeviceCount();
	bool getDeviceInfo(int device, RtAudio::DeviceInfo *deviceInfo);
	/** Returns the number of inputs or outputs, whichever is greater */
	int getDeviceChannels(int device);
	std::string getDeviceName(int device);
	std::string getDeviceDetail(int device, int offset);
	void setDevice(int device, int offset);

	std::vector<int> getSampleRates();
	void setSampleRate(int sampleRate);
	std::vector<int> getBlockSizes();
	void setBlockSize(int blockSize);

	void setChannels(int numOutputs, int numInputs);

	/** Must close the stream before opening */
	void openStream();
	void closeStream();

	virtual void processStream(const float *input, float *output, int frames) {}
	virtual void onCloseStream() {}
	virtual void onOpenStream() {}
	virtual void onChannelsChange() {}
	json_t *toJson();
	void fromJson(json_t *rootJ);
};


} // namespace rack
