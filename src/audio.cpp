#include "util.hpp"
#include "math.hpp"
#include "audio.hpp"


namespace rack {


AudioIO::AudioIO() {
	setDriver(RtAudio::UNSPECIFIED);
}

AudioIO::~AudioIO() {
	closeStream();
}

std::vector<int> AudioIO::listDrivers() {
	std::vector<RtAudio::Api> apis;
	RtAudio::getCompiledApi(apis);
	std::vector<int> drivers;
	for (RtAudio::Api api : apis)
		drivers.push_back((int) api);
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
		case RtAudio::RTAUDIO_DUMMY: return "Dummy";
		default: return "Unknown";
	}
}

int AudioIO::getDriver() {
	if (!stream)
		return RtAudio::UNSPECIFIED;
	return stream->getCurrentApi();
}

void AudioIO::setDriver(int driver) {
	closeStream();
	if (stream)
		delete stream;
	stream = new RtAudio((RtAudio::Api) driver);
}

int AudioIO::getDeviceCount() {
	if (!stream)
		return 0;
	return stream->getDeviceCount();
}

std::string AudioIO::getDeviceName(int device) {
	if (!stream || device < 0)
		return "";

	try {
		RtAudio::DeviceInfo deviceInfo = stream->getDeviceInfo(device);
		return deviceInfo.name;
	}
	catch (RtAudioError &e) {
		warn("Failed to query audio device: %s", e.what());
		return "";
	}
}

std::string AudioIO::getDeviceDetail(int device) {
	if (!stream || device < 0)
		return "";

	try {
		RtAudio::DeviceInfo deviceInfo = stream->getDeviceInfo(device);
		return stringf("%s (%d in, %d out)", deviceInfo.name.c_str(), deviceInfo.inputChannels, deviceInfo.outputChannels);
	}
	catch (RtAudioError &e) {
		warn("Failed to query audio device: %s", e.what());
		return "";
	}
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
	if (!stream)
		return;

	// Open new device
	if (device >= 0) {
		RtAudio::DeviceInfo deviceInfo;
		try {
			deviceInfo = stream->getDeviceInfo(device);
		}
		catch (RtAudioError &e) {
			warn("Failed to query audio device: %s", e.what());
			return;
		}

		numOutputs = mini(deviceInfo.outputChannels, maxOutputs);
		numInputs = mini(deviceInfo.inputChannels, maxInputs);

		if (numOutputs == 0 && numInputs == 0) {
			warn("Audio device %d has 0 inputs and 0 outputs");
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
			debug("Opening audio stream %d", device);
			stream->openStream(
				numOutputs == 0 ? NULL : &outParameters,
				numInputs == 0 ? NULL : &inParameters,
				RTAUDIO_FLOAT32, closestSampleRate, (unsigned int*) &blockSize, &rtCallback, this, &options, NULL);
		}
		catch (RtAudioError &e) {
			warn("Failed to open audio stream: %s", e.what());
			return;
		}

		try {
			debug("Starting audio stream %d", device);
			stream->startStream();
		}
		catch (RtAudioError &e) {
			warn("Failed to start audio stream: %s", e.what());
			return;
		}

		// Update sample rate because this may have changed
		this->sampleRate = stream->getStreamSampleRate();
		this->device = device;
		onOpenStream();
	}
}

void AudioIO::closeStream() {
	if (stream) {
		if (stream->isStreamRunning()) {
			debug("Stopping audio stream %d", device);
			try {
				stream->stopStream();
			}
			catch (RtAudioError &e) {
				warn("Failed to stop stream %s", e.what());
			}
		}
		if (stream->isStreamOpen()) {
			debug("Closing audio stream %d", device);
			try {
				stream->closeStream();
			}
			catch (RtAudioError &e) {
				warn("Failed to close stream %s", e.what());
			}
		}
	}

	// Reset stream settings
	device = -1;
	numOutputs = 0;
	numInputs = 0;
	onCloseStream();
}

std::vector<int> AudioIO::listSampleRates() {
	if (!stream || device < 0)
		return {};

	try {
		RtAudio::DeviceInfo deviceInfo = stream->getDeviceInfo(device);
		std::vector<int> sampleRates(deviceInfo.sampleRates.begin(), deviceInfo.sampleRates.end());
		return sampleRates;
	}
	catch (RtAudioError &e) {
		warn("Failed to query audio device: %s", e.what());
		return {};
	}
}


} // namespace rack
