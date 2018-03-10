#include "audio.hpp"
#include "util/common.hpp"
#include "bridge.hpp"


#define BRIDGE_DRIVER -5000


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
	// Add fake Bridge driver
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
	setDevice(-1, 0);

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

bool AudioIO::getDeviceInfo(int device, RtAudio::DeviceInfo *deviceInfo) {
	if (!deviceInfo)
		return false;

	if (rtAudio) {
		if (device == this->device) {
			*deviceInfo = this->deviceInfo;
			return true;
		}
		else {
			try {
				*deviceInfo = rtAudio->getDeviceInfo(device);
				return true;
			}
			catch (RtAudioError &e) {
				warn("Failed to query RtAudio device: %s", e.what());
				return false;
			}
		}
	}
	else {
		return false;
	}
}

int AudioIO::getDeviceChannels(int device) {
	if (device < 0)
		return 0;

	if (rtAudio) {
		RtAudio::DeviceInfo deviceInfo;
		if (getDeviceInfo(device, &deviceInfo))
			return max(deviceInfo.inputChannels, deviceInfo.outputChannels);
	}
	if (driver == BRIDGE_DRIVER) {
		return 2;
	}
	return 0;
}

std::string AudioIO::getDeviceName(int device) {
	if (device < 0)
		return "";

	if (rtAudio) {
		RtAudio::DeviceInfo deviceInfo;
		if (getDeviceInfo(device, &deviceInfo))
			return deviceInfo.name;
	}
	if (driver == BRIDGE_DRIVER) {
		return stringf("%d", device + 1);
	}
	return "";
}

std::string AudioIO::getDeviceDetail(int device, int offset) {
	if (device < 0)
		return "";

	if (rtAudio) {
		RtAudio::DeviceInfo deviceInfo;
		if (getDeviceInfo(device, &deviceInfo)) {
			std::string deviceDetail = stringf("%s (", deviceInfo.name.c_str());
			if (offset < (int) deviceInfo.inputChannels)
				deviceDetail += stringf("%d-%d in", offset + 1, min(offset + maxChannels, deviceInfo.inputChannels));
			if (offset < (int) deviceInfo.inputChannels && offset < (int) deviceInfo.outputChannels)
				deviceDetail += ", ";
			if (offset < (int) deviceInfo.outputChannels)
				deviceDetail += stringf("%d-%d out", offset + 1, min(offset + maxChannels, deviceInfo.outputChannels));
			deviceDetail += ")";
			return deviceDetail;
		}
	}
	if (driver == BRIDGE_DRIVER) {
		return stringf("Channel %d", device + 1);
	}
	return "";
}

void AudioIO::setDevice(int device, int offset) {
	closeStream();
	this->device = device;
	this->offset = offset;
	openStream();
}

void AudioIO::setSampleRate(int sampleRate) {
	closeStream();
	this->sampleRate = sampleRate;
	openStream();
}

void AudioIO::setBlockSize(int blockSize) {
	closeStream();
	this->blockSize = blockSize;
	openStream();
}


static int rtCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void *userData) {
	AudioIO *audioIO = (AudioIO*) userData;
	assert(audioIO);
	audioIO->processStream((const float *) inputBuffer, (float *) outputBuffer, nFrames);
	return 0;
}

void AudioIO::openStream() {
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

		if (rtAudio->isStreamOpen())
			return;

		numOutputs = clamp((int) deviceInfo.outputChannels - offset, 0, maxChannels);
		numInputs = clamp((int) deviceInfo.inputChannels - offset, 0, maxChannels);

		if (numOutputs == 0 && numInputs == 0) {
			warn("RtAudio device %d has 0 inputs and 0 outputs");
			return;
		}

		RtAudio::StreamParameters outParameters;
		outParameters.deviceId = device;
		outParameters.nChannels = numOutputs;
		outParameters.firstChannel = offset;

		RtAudio::StreamParameters inParameters;
		inParameters.deviceId = device;
		inParameters.nChannels = numInputs;
		inParameters.firstChannel = offset;

		RtAudio::StreamOptions options;
		options.flags |= RTAUDIO_JACK_DONT_CONNECT;

		int closestSampleRate = deviceInfo.preferredSampleRate;
		for (int sr : deviceInfo.sampleRates) {
			if (abs(sr - sampleRate) < abs(closestSampleRate - sampleRate)) {
				closestSampleRate = sr;
			}
		}

		try {
			info("Opening audio RtAudio device %d with %d in %d out", device, numInputs, numOutputs);
			rtAudio->openStream(
				numOutputs == 0 ? NULL : &outParameters,
				numInputs == 0 ? NULL : &inParameters,
				RTAUDIO_FLOAT32, closestSampleRate, (unsigned int*) &blockSize,
				&rtCallback, this, &options, NULL);
		}
		catch (RtAudioError &e) {
			warn("Failed to open RtAudio stream: %s", e.what());
			return;
		}

		try {
			info("Starting RtAudio stream %d", device);
			rtAudio->startStream();
		}
		catch (RtAudioError &e) {
			warn("Failed to start RtAudio stream: %s", e.what());
			return;
		}

		// Update sample rate because this may have changed
		this->sampleRate = rtAudio->getStreamSampleRate();
		onOpenStream();
	}
	if (driver == BRIDGE_DRIVER) {
		numOutputs = 2;
		numInputs = 2;
		// TEMP
		sampleRate = 44100;
		blockSize = 256;
		bridgeAudioSubscribe(device, this);
	}
}

void AudioIO::closeStream() {
	numOutputs = 0;
	numInputs = 0;

	if (rtAudio) {
		if (rtAudio->isStreamRunning()) {
			info("Stopping RtAudio stream %d", device);
			try {
				rtAudio->stopStream();
			}
			catch (RtAudioError &e) {
				warn("Failed to stop RtAudio stream %s", e.what());
			}
		}
		if (rtAudio->isStreamOpen()) {
			info("Closing RtAudio stream %d", device);
			try {
				rtAudio->closeStream();
			}
			catch (RtAudioError &e) {
				warn("Failed to close RtAudio stream %s", e.what());
			}
		}
		deviceInfo = RtAudio::DeviceInfo();
	}
	if (driver == BRIDGE_DRIVER) {
		bridgeAudioUnsubscribe(device, this);
	}

	onCloseStream();
}

bool AudioIO::isActive() {
	if (rtAudio) {
		return rtAudio->isStreamRunning();
	}
	if (driver == BRIDGE_DRIVER) {
		bridgeAudioIsSubscribed(device, this);
	}
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
	json_object_set_new(rootJ, "offset", json_integer(offset));
	json_object_set_new(rootJ, "maxChannels", json_integer(maxChannels));
	json_object_set_new(rootJ, "sampleRate", json_integer(sampleRate));
	json_object_set_new(rootJ, "blockSize", json_integer(blockSize));
	return rootJ;
}

void AudioIO::fromJson(json_t *rootJ) {
	closeStream();

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

	json_t *offsetJ = json_object_get(rootJ, "offset");
	if (offsetJ)
		offset = json_integer_value(offsetJ);

	json_t *maxChannelsJ = json_object_get(rootJ, "maxChannels");
	if (maxChannelsJ)
		maxChannels = json_integer_value(maxChannelsJ);

	json_t *sampleRateJ = json_object_get(rootJ, "sampleRate");
	if (sampleRateJ)
		sampleRate = json_integer_value(sampleRateJ);

	json_t *blockSizeJ = json_object_get(rootJ, "blockSize");
	if (blockSizeJ)
		blockSize = json_integer_value(blockSizeJ);

	openStream();
}


} // namespace rack
