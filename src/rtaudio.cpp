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


static const std::map<RtAudio::Api, std::string> RTAUDIO_API_NAMES = {
	{RtAudio::LINUX_ALSA, "ALSA"},
	{RtAudio::UNIX_JACK, "JACK"},
	{RtAudio::LINUX_PULSE, "PulseAudio"},
	{RtAudio::LINUX_OSS, "OSS"},
	{RtAudio::WINDOWS_WASAPI, "WASAPI"},
	{RtAudio::WINDOWS_ASIO, "ASIO"},
	{RtAudio::WINDOWS_DS, "DirectSound"},
	{RtAudio::MACOSX_CORE, "Core Audio"},
	{RtAudio::RTAUDIO_DUMMY, "Dummy"},
	{RtAudio::UNSPECIFIED, "Unspecified"},
};


struct RtAudioDevice : audio::Device {
	RtAudio::Api api;
	int deviceId;
	RtAudio* rtAudio;
	RtAudio::DeviceInfo deviceInfo;
	RtAudio::StreamParameters inputParameters;
	RtAudio::StreamParameters outputParameters;
	RtAudio::StreamOptions options;
	int blockSize = 0;
	float sampleRate = 0;

	RtAudioDevice(RtAudio::Api api, int deviceId) {
		this->api = api;
		this->deviceId = deviceId;

		// Create RtAudio object
		INFO("Creating RtAudio %s device", RTAUDIO_API_NAMES.at(api).c_str());
		rtAudio = new RtAudio(api, [](RtAudioErrorType type, const std::string& errorText) {
			WARN("RtAudio error %d: %s", type, errorText.c_str());
		});

		rtAudio->showWarnings(false);

		try {
			// Query device ID
			deviceInfo = rtAudio->getDeviceInfo(deviceId);
			if (!deviceInfo.probed) {
				throw Exception("Failed to query RtAudio %s device %d", RTAUDIO_API_NAMES.at(api).c_str(), deviceId);
			}

			openStream();
		}
		catch (Exception& e) {
			delete rtAudio;
			throw;
		}
	}

	~RtAudioDevice() {
		closeStream();
		delete rtAudio;
	}

	void openStream() {
		// Open new device
		if (deviceInfo.outputChannels == 0 && deviceInfo.inputChannels == 0) {
			throw Exception("RtAudio %s device %d has 0 inputs and 0 outputs", RTAUDIO_API_NAMES.at(api).c_str(), deviceId);
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
			if (api == RtAudio::WINDOWS_DS)
				blockSize = 1024;
			else
				blockSize = 256;
		}

		INFO("Opening RtAudio %s device %d: %s (%d in, %d out, %d sample rate, %d block size)", RTAUDIO_API_NAMES.at(api).c_str(), deviceId, deviceInfo.name.c_str(), inputParameters.nChannels, outputParameters.nChannels, closestSampleRate, blockSize);
		if (rtAudio->openStream(
			outputParameters.nChannels > 0 ? &outputParameters : NULL,
			inputParameters.nChannels > 0 ? &inputParameters : NULL,
			RTAUDIO_FLOAT32, closestSampleRate, (unsigned int*) &blockSize,
			&rtAudioCallback, this, &options)) {
			throw Exception("Failed to open RtAudio %s device %d", RTAUDIO_API_NAMES.at(api).c_str(), deviceId);
		}

		try {
			INFO("Starting RtAudio %s device %d", RTAUDIO_API_NAMES.at(api).c_str(), deviceId);
			if (rtAudio->startStream()) {
				throw Exception("Failed to start RtAudio %s device %d", RTAUDIO_API_NAMES.at(api).c_str(), deviceId);
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
			INFO("Stopping RtAudio %s device %d", RTAUDIO_API_NAMES.at(api).c_str(), deviceId);
			rtAudio->stopStream();
		}
		if (rtAudio->isStreamOpen()) {
			INFO("Closing RtAudio %s device %d", RTAUDIO_API_NAMES.at(api).c_str(), deviceId);
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


struct RtAudioDriver : audio::Driver {
	RtAudio::Api api;
	// deviceId -> Device
	std::map<int, RtAudioDevice*> devices;
	RtAudio* rtAudio = NULL;
	std::vector<RtAudio::DeviceInfo> deviceInfos;

	RtAudioDriver(RtAudio::Api api) {
		this->api = api;

		INFO("Creating RtAudio %s driver", RTAUDIO_API_NAMES.at(api).c_str());
		rtAudio = new RtAudio(api, [](RtAudioErrorType type, const std::string& errorText) {
			WARN("RtAudio error %d: %s", type, errorText.c_str());
		});

		rtAudio->showWarnings(false);

		// Cache DeviceInfos for performance and stability (especially for ASIO).
		if (api == RtAudio::WINDOWS_WASAPI || api == RtAudio::WINDOWS_ASIO || api == RtAudio::WINDOWS_DS) {
			int count = rtAudio->getDeviceCount();
			for (int deviceId = 0; deviceId < count; deviceId++) {
				RtAudio::DeviceInfo deviceInfo = rtAudio->getDeviceInfo(deviceId);
				INFO("Found RtAudio %s device %d: %s (%d in, %d out)", RTAUDIO_API_NAMES.at(api).c_str(), deviceId, deviceInfo.name.c_str(), deviceInfo.inputChannels, deviceInfo.outputChannels);
				deviceInfos.push_back(deviceInfo);
			}

			delete rtAudio;
			rtAudio = NULL;
		}
	}

	~RtAudioDriver() {
		assert(devices.empty());
		if (rtAudio)
			delete rtAudio;
	}

	std::string getName() override {
		return RTAUDIO_API_NAMES.at(api);
	}

	std::vector<int> getDeviceIds() override {
		int count = 0;
		if (rtAudio) {
			count = rtAudio->getDeviceCount();
		}
		else {
			count = deviceInfos.size();
		}

		std::vector<int> deviceIds;
		for (int i = 0; i < count; i++)
			deviceIds.push_back(i);
		return deviceIds;
	}

	std::string getDeviceName(int deviceId) override {
		if (rtAudio) {
			int count = rtAudio->getDeviceCount();
			if (0 <= deviceId && deviceId < count) {
				RtAudio::DeviceInfo deviceInfo = rtAudio->getDeviceInfo(deviceId);
				return deviceInfo.name;
			}
		}
		else {
			if (0 <= deviceId && deviceId < (int) deviceInfos.size())
				return deviceInfos[deviceId].name;
		}
		return "";
	}

	int getDeviceNumInputs(int deviceId) override {
		if (rtAudio) {
			int count = rtAudio->getDeviceCount();
			if (0 <= deviceId && deviceId < count) {
				RtAudio::DeviceInfo deviceInfo = rtAudio->getDeviceInfo(deviceId);
				return deviceInfo.inputChannels;
			}
		}
		else {
			if (0 <= deviceId && deviceId < (int) deviceInfos.size())
				return deviceInfos[deviceId].inputChannels;
		}
		return 0;
	}

	int getDeviceNumOutputs(int deviceId) override {
		if (rtAudio) {
			int count = rtAudio->getDeviceCount();
			if (0 <= deviceId && deviceId < count) {
				RtAudio::DeviceInfo deviceInfo = rtAudio->getDeviceInfo(deviceId);
				return deviceInfo.outputChannels;
			}
		}
		else {
			if (0 <= deviceId && deviceId < (int) deviceInfos.size())
				return deviceInfos[deviceId].outputChannels;
		}
		return 0;
	}

	audio::Device* subscribe(int deviceId, audio::Port* port) override {
		RtAudioDevice* device;
		auto it = devices.find(deviceId);
		if (it == devices.end()) {
			// ASIO only allows one device to be used simultaneously
			if (api == RtAudio::WINDOWS_ASIO && devices.size() >= 1)
				throw Exception("ASIO driver only allows one audio device to be used simultaneously");

			// Can throw Exception
			device = new RtAudioDevice(api, deviceId);
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
		RtAudio::LINUX_PULSE,
		RtAudio::UNIX_JACK,
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
