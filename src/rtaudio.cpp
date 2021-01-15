#include <map>
#include <algorithm>

#pragma GCC diagnostic push
#ifndef __clang__
	#pragma GCC diagnostic ignored "-Wsuggest-override"
#endif
#include <rtaudio/RtAudio.h>
#pragma GCC diagnostic pop

#include <rtaudio.hpp>
#include <audio.hpp>
#include <string.hpp>
#include <math.hpp>
#include <system.hpp>


namespace rack {


struct RtAudioDevice : audio::Device {
	RtAudio* rtAudio;
	int deviceId;
	RtAudio::DeviceInfo deviceInfo;
	RtAudio::StreamParameters inputParameters;
	RtAudio::StreamParameters outputParameters;
	RtAudio::StreamOptions options;
	int blockSize = 0;
	float sampleRate = 0;

	RtAudioDevice(RtAudio::Api api, int deviceId) {
		rtAudio = new RtAudio(api);
		rtAudio->showWarnings(false);
		if (!rtAudio) {
			throw Exception("Failed to create RtAudio driver %d", api);
		}
		rtAudio->showWarnings(false);

		try {
			deviceInfo = rtAudio->getDeviceInfo(deviceId);
		}
		catch (RtAudioError& e) {
			throw Exception("Failed to query RtAudio device: %s", e.what());
		}

		this->deviceId = deviceId;
		openStream();
	}

	~RtAudioDevice() {
		try {
			closeStream();
			delete rtAudio;
		}
		catch (Exception& e) {
			WARN("Failed to destroy RtAudioDevice: %s", e.what());
			// Ignore exceptions
		}
	}

	void openStream() {
		// Open new device
		if (deviceInfo.outputChannels == 0 && deviceInfo.inputChannels == 0) {
			throw Exception("RtAudio device %d has 0 inputs and 0 outputs", deviceId);
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
		// options.flags |= RTAUDIO_MINIMIZE_LATENCY;
		options.flags |= RTAUDIO_SCHEDULE_REALTIME;
		options.numberOfBuffers = 2;
		options.streamName = "VCV Rack";

		float closestSampleRate = deviceInfo.preferredSampleRate;
		if (sampleRate > 0) {
			// Find the closest sample rate to the requested one.
			for (float sr : deviceInfo.sampleRates) {
				if (std::fabs(sr - sampleRate) < std::fabs(closestSampleRate - sampleRate)) {
					closestSampleRate = sr;
				}
			}
		}

		if (blockSize <= 0) {
			blockSize = 256;
		}

		INFO("Opening audio RtAudio device %d with %d in %d out, %g sample rate %d block size", deviceId, inputParameters.nChannels, outputParameters.nChannels, closestSampleRate, blockSize);
		try {
			rtAudio->openStream(
			  outputParameters.nChannels > 0 ? &outputParameters : NULL,
			  inputParameters.nChannels > 0 ? &inputParameters : NULL,
			  RTAUDIO_FLOAT32, closestSampleRate, (unsigned int*) &blockSize,
			  &rtAudioCallback, this, &options, NULL);
		}
		catch (RtAudioError& e) {
			throw Exception("Failed to open RtAudio stream: %s", e.what());
		}

		INFO("Starting RtAudio stream %d", deviceId);
		try {
			rtAudio->startStream();
		}
		catch (RtAudioError& e) {
			throw Exception("Failed to start RtAudio stream: %s", e.what());
		}

		// Update sample rate to actual value
		sampleRate = rtAudio->getStreamSampleRate();
		INFO("Opened RtAudio stream");
		onStartStream();
	}

	void closeStream() {
		if (rtAudio->isStreamRunning()) {
			INFO("Stopping RtAudio stream %d", deviceId);
			try {
				rtAudio->stopStream();
			}
			catch (RtAudioError& e) {
				throw Exception("Failed to stop RtAudio stream %s", e.what());
			}
		}
		if (rtAudio->isStreamOpen()) {
			INFO("Closing RtAudio stream %d", deviceId);
			try {
				rtAudio->closeStream();
			}
			catch (RtAudioError& e) {
				throw Exception("Failed to close RtAudio stream %s", e.what());
			}
		}
		INFO("Closed RtAudio stream");
		onStopStream();
	}

	std::string getName() override {
		return deviceInfo.name;
	}
	int getNumInputs() override {
		return inputParameters.nChannels;
	}
	int getNumOutputs() override {
		return outputParameters.nChannels;
	}

	std::set<float> getSampleRates() override {
		std::set<float> sampleRates(deviceInfo.sampleRates.begin(), deviceInfo.sampleRates.end());
		return sampleRates;
	}
	float getSampleRate() override {
		return sampleRate;
	}
	void setSampleRate(float sampleRate) override {
		if (sampleRate == this->sampleRate)
			return;
		closeStream();
		this->sampleRate = sampleRate;
		openStream();
	}

	std::set<int> getBlockSizes() override {
		std::set<int> blockSizes;
		// 32 to 4096
		for (int i = 5; i <= 12; i++) {
			blockSizes.insert(1 << i);
		}
		return blockSizes;
	}
	int getBlockSize() override {
		return blockSize;
	}
	void setBlockSize(int blockSize) override {
		if (blockSize == this->blockSize)
			return;
		closeStream();
		this->blockSize = blockSize;
		openStream();
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
	RtAudio* rtAudio;
	// deviceId -> Device
	std::map<int, RtAudioDevice*> devices;

	RtAudioDriver(RtAudio::Api api) {
		rtAudio = new RtAudio(api);
		rtAudio->showWarnings(false);
	}

	~RtAudioDriver() {
		assert(devices.empty());
		delete rtAudio;
	}

	std::string getName() override {
		static const std::map<RtAudio::Api, std::string> apiNames = {
			{RtAudio::LINUX_ALSA, "ALSA"},
			{RtAudio::UNIX_JACK, "JACK (unsupported)"},
			{RtAudio::LINUX_PULSE, "PulseAudio"},
			{RtAudio::LINUX_OSS, "OSS"},
			{RtAudio::WINDOWS_WASAPI, "WASAPI"},
			{RtAudio::WINDOWS_ASIO, "ASIO"},
			{RtAudio::WINDOWS_DS, "DirectSound"},
			{RtAudio::MACOSX_CORE, "CoreAudio"},
			{RtAudio::RTAUDIO_DUMMY, "Dummy"},
			{RtAudio::UNSPECIFIED, "Unspecified"},
		};
		return apiNames.at(rtAudio->getCurrentApi());
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

	int getDeviceNumInputs(int deviceId) override {
		if (deviceId >= 0) {
			RtAudio::DeviceInfo deviceInfo = rtAudio->getDeviceInfo(deviceId);
			return deviceInfo.inputChannels;
		}
		return 0;
	}

	int getDeviceNumOutputs(int deviceId) override {
		if (deviceId >= 0) {
			RtAudio::DeviceInfo deviceInfo = rtAudio->getDeviceInfo(deviceId);
			return deviceInfo.outputChannels;
		}
		return 0;
	}

	audio::Device* subscribe(int deviceId, audio::Port* port) override {
		RtAudioDevice* device;
		auto it = devices.find(deviceId);
		if (it == devices.end()) {
			// Can throw Exception
			device = new RtAudioDevice(rtAudio->getCurrentApi(), deviceId);
			devices[deviceId] = device;
		}
		else {
			device = it->second;
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

	// I don't like the order returned by getCompiledApi(), so reorder it here.
	std::vector<RtAudio::Api> orderedApis = {
		RtAudio::LINUX_ALSA,
		RtAudio::UNIX_JACK,
		RtAudio::LINUX_PULSE,
		RtAudio::LINUX_OSS,
		RtAudio::WINDOWS_WASAPI,
		RtAudio::WINDOWS_ASIO,
		RtAudio::WINDOWS_DS,
		RtAudio::MACOSX_CORE,
	};
	for (RtAudio::Api api : orderedApis) {
		auto it = std::find(apis.begin(), apis.end(), api);
		if (it != apis.end()) {
			RtAudioDriver* driver = new RtAudioDriver(api);
			audio::addDriver((int) api, driver);
		}
	}
}

} // namespace rack
