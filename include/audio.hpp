#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#include <RtAudio.h>
#pragma GCC diagnostic pop


namespace rack {


struct AudioIO {
	int maxOutputs = 8;
	int maxInputs = 8;

	RtAudio *stream = NULL;
	// Stream properties
	int device = -1;
	int sampleRate = 44100;
	int blockSize = 256;
	int numOutputs = 0;
	int numInputs = 0;

	AudioIO();
	~AudioIO();

	std::vector<int> listDrivers();
	std::string getDriverName(int driver);
	int getDriver();
	void setDriver(int driver);

	int getDeviceCount();
	std::string getDeviceName(int device);
	std::string getDeviceDetail(int device);
	void openStream();
	void closeStream();

	std::vector<int> listSampleRates();
};


} // namespace rack
