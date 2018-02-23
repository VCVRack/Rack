#include "util/common.hpp"
#include "audio.hpp"


#define BRIDGE_DRIVER -1
#define BRIDGE_CHANNELS 16


namespace rack {


AudioIO::AudioIO() {
	setDriver(RtAudio::UNSPECIFIED);
}

AudioIO::~AudioIO() {
	closeStream();
}

std::vector<int> AudioIO::getDrivers() {
	std::vector<RtAudio::Api> apis;
	RtAudio::getCompiledApi(apis);
	std::vector<int> drivers;
	for (RtAudio::Api api : apis)
		drivers.push_back((int) api);
	// Add Bridge fake driver
	drivers.push_back(BRIDGE_DRIVER);
	return drivers;
}

std::string AudioIO::getDriverName(int driver) {
	switch (driver) {
		case RtAudio::UNSPECIFIED: return "Unspecified";
		case RtAudio::LINUX_ALSA: return "ALSA";
		case RtAudio::LINUX_PULSE: return "PulseAudio";
		case RtAudio::LINUX_OSS: return "OSS";
		case RtAudio::UNIX_JACK: return "JACK";
		case RtAudio::MACOSX_CORE: return "Core Audio";
		case RtAudio::WINDOWS_WASAPI: return "WASAPI";
		case RtAudio::WINDOWS_ASIO: return "ASIO";
		case RtAudio::WINDOWS_DS: return "DirectSound";
		case RtAudio::RTAUDIO_DUMMY: return "Dummy Audio";
		case BRIDGE_DRIVER: return "Bridge";
		default: return "Unknown";
	}
}

void AudioIO::setDriver(int driver) {
	// Close driver
	closeStream();
	if (rtAudio) {
		delete rtAudio;
		rtAudio = NULL;
	}
	this->driver = 0;

	// Open driver
	if (driver >= 0) {
		rtAudio = new RtAudio((RtAudio::Api) driver);
		this->driver = (int) rtAudio->getCurrentApi();
	}
	else if (driver == BRIDGE_DRIVER) {
		// TODO Connect to Bridge
		this->driver = BRIDGE_DRIVER;
	}
}

int AudioIO::getDeviceCount() {
	if (rtAudio) {
		return rtAudio->getDeviceCount();
	}
	if (driver == BRIDGE_DRIVER) {
		return BRIDGE_CHANNELS;
	}
	return 0;
}

std::string AudioIO::getDeviceName(int device) {
	if (device < 0)
		return "";

	if (rtAudio) {
		try {
			RtAudio::DeviceInfo deviceInfo;
			if (device == this->device)
				deviceInfo = this->deviceInfo;
			else
				deviceInfo = rtAudio->getDeviceInfo(device);
			return deviceInfo.name;
		}
		catch (RtAudioError &e) {
			warn("Failed to query RtAudio device: %s", e.what());
		}
	}
	if (driver == BRIDGE_DRIVER) {
		if (device >= 0)
			return stringf("%d", device + 1);
	}
	return "";
}

std::string AudioIO::getDeviceDetail(int device) {
	if (device < 0)
		return "";

	if (rtAudio) {
		try {
			RtAudio::DeviceInfo deviceInfo;
			if (device == this->device)
				deviceInfo = this->deviceInfo;
			else
				deviceInfo = rtAudio->getDeviceInfo(device);
			return stringf("%s (%d in, %d out)", deviceInfo.name.c_str(), deviceInfo.inputChannels, deviceInfo.outputChannels);
		}
		catch (RtAudioError &e) {
			warn("Failed to query RtAudio device: %s", e.what());
		}
	}
	if (driver == BRIDGE_DRIVER) {
		if (device >= 0)
			return stringf("Channel %d", device + 1);
	}
	return "";
}

static int rtCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void *userData) {
	AudioIO *audioIO = (AudioIO*) userData;
	assert(audioIO);
	audioIO->processStream((const float *) inputBuffer, (float *) outputBuffer, nFrames);
	return 0;
}

void AudioIO::openStream() {
	// Close device but remember the current device number
	int device = this->device;
	closeStream();

	if (device < 0)
		return;

	if (rtAudio) {
		// Open new device
		try {
			deviceInfo = rtAudio->getDeviceInfo(device);
		}
		catch (RtAudioError &e) {
			warn("Failed to query RtAudio device: %s", e.what());
			return;
		}

		numOutputs = min(deviceInfo.outputChannels, maxOutputs);
		numInputs = min(deviceInfo.inputChannels, maxInputs);

		if (numOutputs == 0 && numInputs == 0) {
			warn("RtAudio device %d has 0 inputs and 0 outputs");
			return;
		}

		RtAudio::StreamParameters outParameters;
		outParameters.deviceId = device;
		outParameters.nChannels = numOutputs;

		RtAudio::StreamParameters inParameters;
		inParameters.deviceId = device;
		inParameters.nChannels = numInputs;

		RtAudio::StreamOptions options;
		// options.flags |= RTAUDIO_SCHEDULE_REALTIME;

		int closestSampleRate = deviceInfo.preferredSampleRate;
		for (int sr : deviceInfo.sampleRates) {
			if (abs(sr - sampleRate) < abs(closestSampleRate - sampleRate)) {
				closestSampleRate = sr;
			}
		}

		try {
			debug("Opening audio RtAudio device %d", device);
			rtAudio->openStream(
				numOutputs == 0 ? NULL : &outParameters,
				numInputs == 0 ? NULL : &inParameters,
				RTAUDIO_FLOAT32, closestSampleRate, (unsigned int*) &blockSize, &rtCallback, this, &options, NULL);
		}
		catch (RtAudioError &e) {
			warn("Failed to open RtAudio stream: %s", e.what());
			return;
		}

		try {
			debug("Starting RtAudio stream %d", device);
			rtAudio->startStream();
		}
		catch (RtAudioError &e) {
			warn("Failed to start RtAudio stream: %s", e.what());
			return;
		}

		// Update sample rate because this may have changed
		this->sampleRate = rtAudio->getStreamSampleRate();
		this->device = device;
		onOpenStream();
	}
	if (driver == BRIDGE_DRIVER) {
		if (device < BRIDGE_CHANNELS) {
			this->device = device;
		}
	}
}

void AudioIO::closeStream() {
	if (rtAudio) {
		if (rtAudio->isStreamRunning()) {
			debug("Stopping RtAudio stream %d", device);
			try {
				rtAudio->stopStream();
			}
			catch (RtAudioError &e) {
				warn("Failed to stop RtAudio stream %s", e.what());
			}
		}
		if (rtAudio->isStreamOpen()) {
			debug("Closing RtAudio stream %d", device);
			try {
				rtAudio->closeStream();
			}
			catch (RtAudioError &e) {
				warn("Failed to close RtAudio stream %s", e.what());
			}
		}
		deviceInfo = RtAudio::DeviceInfo();
	}

	// Reset rtAudio settings
	device = -1;
	numOutputs = 0;
	numInputs = 0;
	onCloseStream();
}

bool AudioIO::isActive() {
	if (rtAudio)
		return rtAudio->isStreamRunning();
	// TODO Bridge
	return false;
}


std::vector<int> AudioIO::getSampleRates() {
	if (rtAudio) {
		try {
			RtAudio::DeviceInfo deviceInfo = rtAudio->getDeviceInfo(device);
			std::vector<int> sampleRates(deviceInfo.sampleRates.begin(), deviceInfo.sampleRates.end());
			return sampleRates;
		}
		catch (RtAudioError &e) {
			warn("Failed to query RtAudio device: %s", e.what());
		}
	}
	if (driver == BRIDGE_DRIVER) {
		return {44100, 48000, 88200, 96000, 176400, 192000};
	}

	return {};
}

json_t *AudioIO::toJson() {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "driver", json_integer(driver));
	std::string deviceName = getDeviceName(device);
	json_object_set_new(rootJ, "deviceName", json_string(deviceName.c_str()));
	json_object_set_new(rootJ, "sampleRate", json_integer(sampleRate));
	json_object_set_new(rootJ, "blockSize", json_integer(blockSize));
	return rootJ;
}

void AudioIO::fromJson(json_t *rootJ) {
	json_t *driverJ = json_object_get(rootJ, "driver");
	if (driverJ)
		setDriver(json_number_value(driverJ));

	json_t *deviceNameJ = json_object_get(rootJ, "deviceName");
	if (deviceNameJ) {
		std::string deviceName = json_string_value(deviceNameJ);
		// Search for device ID with equal name
		for (int device = 0; device < getDeviceCount(); device++) {
			if (getDeviceName(device) == deviceName) {
				this->device = device;
				break;
			}
		}
	}

	json_t *sampleRateJ = json_object_get(rootJ, "sampleRate");
	if (sampleRateJ)
		sampleRate = json_integer_value(sampleRateJ);

	json_t *blockSizeJ = json_object_get(rootJ, "blockSize");
	if (blockSizeJ)
		blockSize = json_integer_value(blockSizeJ);

	openStream();
}


} // namespace rack
