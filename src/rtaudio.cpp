#include <rtaudio.hpp>
#include <audio.hpp>
#include <string.hpp>
#include <math.hpp>
#include <system.hpp>
#include <map>

#pragma GCC diagnostic push
#ifndef __clang__
	#pragma GCC diagnostic ignored "-Wsuggest-override"
#endif
#include <RtAudio.h>
#pragma GCC diagnostic pop


namespace rack {


struct RtAudioDevice : audio::Device {
	RtAudio *rtAudio;
	int deviceId;
	RtAudio::DeviceInfo deviceInfo;
	RtAudio::StreamParameters inputParameters;
	RtAudio::StreamParameters outputParameters;
	RtAudio::StreamOptions options;
	int blockSize = 256;
	int sampleRate = 44100;

	RtAudioDevice(RtAudio::Api api, int deviceId) {
		rtAudio = new RtAudio(api);
		if (!rtAudio) {
			throw Exception(string::f("Failed to create RtAudio driver %d", api));
		}
		try {
			deviceInfo = rtAudio->getDeviceInfo(deviceId);
		}
		catch (RtAudioError& e) {
			WARN("Failed to query RtAudio device: %s", e.what());
			throw Exception(string::f("Failed to query RtAudio device: %s", e.what()));
		}

		this->deviceId = deviceId;
		openStream();
	}

	~RtAudioDevice() {
		closeStream();
		delete rtAudio;
	}

	void openStream() {
		// Open new device
		if (deviceInfo.outputChannels == 0 && deviceInfo.inputChannels == 0) {
			WARN("RtAudio device %d has 0 inputs and 0 outputs", deviceId);
			return;
		}

		inputParameters = RtAudio::StreamParameters();
		inputParameters.deviceId = deviceId;
		inputParameters.nChannels = deviceInfo.inputChannels;
		inputParameters.firstChannel = 0;

		outputParameters = RtAudio::StreamParameters();
		outputParameters.deviceId = deviceId;
		outputParameters.nChannels = deviceInfo.outputChannels;
		outputParameters.firstChannel = 0;

		options = RtAudio::StreamOptions();
		options.flags |= RTAUDIO_JACK_DONT_CONNECT;
		options.streamName = "VCV Rack";

		int closestSampleRate = deviceInfo.preferredSampleRate;
		for (int sr : deviceInfo.sampleRates) {
			if (std::abs(sr - sampleRate) < std::abs(closestSampleRate - sampleRate)) {
				closestSampleRate = sr;
			}
		}

		try {
			INFO("Opening audio RtAudio device %d with %d in %d out", deviceId, inputParameters.nChannels, outputParameters.nChannels);
			rtAudio->openStream(
			  outputParameters.nChannels == 0 ? NULL : &outputParameters,
			  inputParameters.nChannels == 0 ? NULL : &inputParameters,
			  RTAUDIO_FLOAT32, closestSampleRate, (unsigned int*) &blockSize,
			  &rtAudioCallback, this, &options, NULL);
		}
		catch (RtAudioError& e) {
			WARN("Failed to open RtAudio stream: %s", e.what());
			return;
		}

		try {
			INFO("Starting RtAudio stream %d", deviceId);
			rtAudio->startStream();
		}
		catch (RtAudioError& e) {
			WARN("Failed to start RtAudio stream: %s", e.what());
			return;
		}

		// Update sample rate to actual value
		sampleRate = rtAudio->getStreamSampleRate();
		onOpenStream();
	}

	void closeStream() {
		if (rtAudio->isStreamRunning()) {
			INFO("Stopping RtAudio stream %d", deviceId);
			try {
				rtAudio->stopStream();
			}
			catch (RtAudioError& e) {
				WARN("Failed to stop RtAudio stream %s", e.what());
			}
		}
		if (rtAudio->isStreamOpen()) {
			INFO("Closing RtAudio stream %d", deviceId);
			try {
				rtAudio->closeStream();
			}
			catch (RtAudioError& e) {
				WARN("Failed to close RtAudio stream %s", e.what());
			}
		}
		onCloseStream();
	}

	std::vector<int> getSampleRates() override {
		std::vector<int> sampleRates(deviceInfo.sampleRates.begin(), deviceInfo.sampleRates.end());
		return sampleRates;
	}
	int getSampleRate() override {
		return sampleRate;
	}
	void setSampleRate(int sampleRate) override {
		closeStream();
		this->sampleRate = sampleRate;
		openStream();
	}

	std::vector<int> getBlockSizes() override {
		return {32, 48, 64, 96, 128, 192, 256, 384, 512, 768, 1024, 1536, 2048, 3072, 4096};
	}
	int getBlockSize() override {
		return blockSize;
	}
	void setBlockSize(int blockSize) override {
		closeStream();
		this->blockSize = blockSize;
		openStream();
	}

	int getNumInputs() override {
		return inputParameters.nChannels;
	}
	int getNumOutputs() override {
		return outputParameters.nChannels;
	}

	static int rtAudioCallback(void* outputBuffer, void* inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void* userData) {
		RtAudioDevice* device = (RtAudioDevice*) userData;
		assert(device);

		int inputStride = device->getNumInputs();
		int outputStride = device->getNumOutputs();
		device->processBuffer((const float*) inputBuffer, inputStride, (float*) outputBuffer, outputStride, nFrames);
		return 0;
	}
};


struct RtAudioDriver : audio::Driver {
	// Just for querying device IDs names
	RtAudio *rtAudio;
	// deviceId -> Device
	std::map<int, RtAudioDevice*> devices;

	RtAudioDriver(RtAudio::Api api) {
		rtAudio = new RtAudio(api);
	}

	~RtAudioDriver() {
		assert(devices.empty());
		delete rtAudio;
	}

	std::string getName() override {
		return RtAudio::getApiDisplayName(rtAudio->getCurrentApi());
	}

	std::vector<int> getDeviceIds() override {
		int count = rtAudio->getDeviceCount();
		std::vector<int> deviceIds;
		for (int i = 0; i < count; i++)
			deviceIds.push_back(i);
		return deviceIds;
	}

	std::string getDeviceName(int deviceId) override {
		if (deviceId >= 0) {
			RtAudio::DeviceInfo deviceInfo = rtAudio->getDeviceInfo(deviceId);
			return deviceInfo.name;
		}
		return "";
	}

	audio::Device* subscribe(int deviceId, audio::Port* port) override {
		RtAudioDevice* device = devices[deviceId];
		if (!device) {
			devices[deviceId] = device = new RtAudioDevice(rtAudio->getCurrentApi(), deviceId);
			// TODO Error check
		}

		device->subscribe(port);
		return device;
	}

	void unsubscribe(int deviceId, audio::Port* port) override {
		auto it = devices.find(deviceId);
		if (it == devices.end())
			return;
		RtAudioDevice* device = it->second;
		device->unsubscribe(port);

		if (device->subscribed.empty()) {
			devices.erase(it);
			delete device;
		}
	}
};


void rtaudioInit() {
	std::vector<RtAudio::Api> apis;
	RtAudio::getCompiledApi(apis);
	for (RtAudio::Api api : apis) {
		RtAudioDriver* driver = new RtAudioDriver(api);
		audio::addDriver((int) api, driver);
	}
}

} // namespace rack
