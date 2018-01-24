#pragma once

#include <jansson.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#include <RtAudio.h>
#pragma GCC diagnostic pop


namespace rack {


struct AudioIO {
	int maxOutputs = 8;
	int maxInputs = 8;

	// Stream properties
	int driver = 0;
	int device = -1;
	int sampleRate = 44100;
	int blockSize = 256;
	int numOutputs = 0;
	int numInputs = 0;
	RtAudio *rtAudio = NULL;

	AudioIO();
	virtual ~AudioIO();

	std::vector<int> listDrivers();
	std::string getDriverName(int driver);
	void setDriver(int driver);

	int getDeviceCount();
	std::string getDeviceName(int device);
	std::string getDeviceDetail(int device);
	void openStream();
	void closeStream();
	/** Returns whether the audio stream is open and running */
	bool isActive();

	std::vector<int> listSampleRates();

	virtual void processStream(const float *input, float *output, int length) {}
	virtual void onCloseStream() {}
	virtual void onOpenStream() {}
	json_t *toJson();
	void fromJson(json_t *rootJ);
};


} // namespace rack
