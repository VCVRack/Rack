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


struct RtAudioDevice;


struct RtAudioDriver : audio::Driver {
	RtAudio::Api api;
	std::string name;
	RtAudio* rtAudio = NULL;
	// deviceId -> Device
	std::map<int, RtAudioDevice*> devices;

	RtAudioDriver(RtAudio::Api api, std::string name) {
		this->api = api;
		this->name = name;

		INFO("Creating RtAudio %s driver", name.c_str());
		rtAudio = new RtAudio(api, [](RtAudioErrorType type, const std::string& errorText) {
			WARN("RtAudio error %d: %s", type, errorText.c_str());
		});

		rtAudio->showWarnings(false);
	}

	~RtAudioDriver() {
		assert(devices.empty());
		if (rtAudio)
			delete rtAudio;
	}

	std::string getName() override {
		return name;
	}

	std::vector<int> getDeviceIds() override {
		std::vector<int> deviceIds;
		if (rtAudio) {
			for (unsigned int id : rtAudio->getDeviceIds()) {
				deviceIds.push_back(id);
			}
		}
		return deviceIds;
	}

	std::string getDeviceName(int deviceId) override {
		if (rtAudio) {
			RtAudio::DeviceInfo deviceInfo = rtAudio->getDeviceInfo(deviceId);
			if (deviceInfo.ID > 0)
				return deviceInfo.name;
		}
		return "";
	}

	int getDeviceNumInputs(int deviceId) override {
		if (rtAudio) {
			RtAudio::DeviceInfo deviceInfo = rtAudio->getDeviceInfo(deviceId);
			if (deviceInfo.ID > 0)
				return deviceInfo.inputChannels;
		}
		return 0;
	}

	int getDeviceNumOutputs(int deviceId) override {
		if (rtAudio) {
			RtAudio::DeviceInfo deviceInfo = rtAudio->getDeviceInfo(deviceId);
			if (deviceInfo.ID > 0)
				return deviceInfo.outputChannels;
		}
		return 0;
	}

	audio::Device* subscribe(int deviceId, audio::Port* port) override;
	void unsubscribe(int deviceId, audio::Port* port) override;
};


struct RtAudioDevice : audio::Device {
	RtAudioDriver* driver;
	int deviceId;
	RtAudio* rtAudio;
	RtAudio::DeviceInfo deviceInfo;
	RtAudio::StreamParameters inputParameters;
	RtAudio::StreamParameters outputParameters;
	RtAudio::StreamOptions options;
	int blockSize = 0;
	float sampleRate = 0;

	RtAudioDevice(RtAudioDriver* driver, int deviceId) {
		this->driver = driver;
		this->deviceId = deviceId;

		// Create RtAudio object
		INFO("Creating RtAudio %s device", driver->getName().c_str());
		rtAudio = new RtAudio(driver->api, [](RtAudioErrorType type, const std::string& errorText) {
			WARN("RtAudio error %d: %s", type, errorText.c_str());
		});

		rtAudio->showWarnings(false);

		try {
			// Query device ID
			deviceInfo = rtAudio->getDeviceInfo(deviceId);
			if (deviceInfo.ID == 0)
				throw Exception("Failed to query RtAudio %s device %d", driver->getName().c_str(), deviceId);

			openStream();
		}
		catch (Exception& e) {
			delete rtAudio;
			throw;
		}
	}

	~RtAudioDevice() {
		closeStream();
		if (rtAudio)
			delete rtAudio;
	}

	void openStream() {
		// Open new device
		if (deviceInfo.outputChannels == 0 && deviceInfo.inputChannels == 0) {
			throw Exception("RtAudio %s device %d has 0 inputs and 0 outputs", driver->getName().c_str(), deviceId);
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

		int32_t closestSampleRate = deviceInfo.preferredSampleRate;
		if (sampleRate > 0) {
			// Find the closest sample rate to the requested one.
			for (int32_t sr : deviceInfo.sampleRates) {
				if (std::fabs(sr - sampleRate) < std::fabs(closestSampleRate - sampleRate)) {
					closestSampleRate = sr;
				}
			}
		}

		if (blockSize <= 0) {
			// DirectSound should use a higher default block size
			if (driver->api == RtAudio::WINDOWS_DS)
				blockSize = 1024;
			else
				blockSize = 256;
		}

		INFO("Opening RtAudio %s device %d: %s (%d in, %d out, %d sample rate, %d block size)", driver->getName().c_str(), deviceId, deviceInfo.name.c_str(), inputParameters.nChannels, outputParameters.nChannels, closestSampleRate, blockSize);
		if (rtAudio->openStream(
			outputParameters.nChannels > 0 ? &outputParameters : NULL,
			inputParameters.nChannels > 0 ? &inputParameters : NULL,
			RTAUDIO_FLOAT32, closestSampleRate, (unsigned int*) &blockSize,
			&rtAudioCallback, this, &options)) {
			throw Exception("Failed to open RtAudio %s device %d", driver->getName().c_str(), deviceId);
		}

		try {
			INFO("Starting RtAudio %s device %d", driver->getName().c_str(), deviceId);
			if (rtAudio->startStream()) {
				throw Exception("Failed to start RtAudio %s device %d", driver->getName().c_str(), deviceId);
			}

			// Update sample rate to actual value
			sampleRate = rtAudio->getStreamSampleRate();

			onStartStream();
		}
		catch (Exception& e) {
			rtAudio->closeStream();
			throw;
		}
	}

	void closeStream() {
		if (rtAudio->isStreamRunning()) {
			INFO("Stopping RtAudio %s device %d", driver->getName().c_str(), deviceId);
			rtAudio->stopStream();
		}
		if (rtAudio->isStreamOpen()) {
			INFO("Closing RtAudio %s device %d", driver->getName().c_str(), deviceId);
			rtAudio->closeStream();
		}

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
		// fprintf(stderr, ".");
		// fflush(stderr);

		RtAudioDevice* that = (RtAudioDevice*) userData;
		assert(that);

		system::setThreadName("RtAudio");

		int inputStride = that->getNumInputs();
		int outputStride = that->getNumOutputs();
		try {
			that->processBuffer((const float*) inputBuffer, inputStride, (float*) outputBuffer, outputStride, nFrames);
		}
		catch (Exception& e) {
			// Log nothing to avoid spamming the log.
		}
		return 0;
	}
};


audio::Device* RtAudioDriver::subscribe(int deviceId, audio::Port* port) {
	RtAudioDevice* device;
	auto it = devices.find(deviceId);
	if (it == devices.end()) {
		// ASIO only allows one device to be used simultaneously
		if (api == RtAudio::WINDOWS_ASIO && devices.size() >= 1)
			throw Exception("ASIO driver only allows one audio device to be used simultaneously");

		// Can throw Exception
		device = new RtAudioDevice(this, deviceId);
		devices[deviceId] = device;
	}
	else {
		device = it->second;
	}

	device->subscribe(port);
	return device;
}


void RtAudioDriver::unsubscribe(int deviceId, audio::Port* port) {
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


struct ApiInfo {
	// Should match indices in https://github.com/VCVRack/rtaudio/blob/ece277bd839603648c80c8a5f145678e13bc23f3/RtAudio.cpp#L107-L118
	int driverId;
	RtAudio::Api rtApi;
	// Used instead of RtAudio::getApiName()
	std::string name;
};
// The vector order here defines the order in the audio driver menu
static const std::vector<ApiInfo> API_INFOS = {
	{1, RtAudio::LINUX_ALSA, "ALSA"},
	{2, RtAudio::LINUX_PULSE, "PulseAudio"},
	{4, RtAudio::UNIX_JACK, "JACK"},
	{5, RtAudio::MACOSX_CORE, "Core Audio"},
	{6, RtAudio::WINDOWS_WASAPI, "WASAPI"},
	{7, RtAudio::WINDOWS_ASIO, "ASIO"},
	{8, RtAudio::WINDOWS_DS, "DirectSound"},
};


void rtaudioInit() {
	// Get RtAudio's driver list
	std::vector<RtAudio::Api> apis;
	RtAudio::getCompiledApi(apis);

	for (const ApiInfo& apiInfo : API_INFOS) {
		auto it = std::find(apis.begin(), apis.end(), apiInfo.rtApi);
		if (it == apis.end())
			continue;
		// Create and add driver
		RtAudioDriver* driver = new RtAudioDriver(apiInfo.rtApi, apiInfo.name);
		audio::addDriver(apiInfo.driverId, driver);
	}
}

} // namespace rack
