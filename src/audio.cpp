#include <audio.hpp>
#include <string.hpp>


namespace rack {
namespace audio {


static std::vector<std::pair<int, Driver*>> drivers;


////////////////////
// Device
////////////////////

void Device::subscribe(Port* port) {
	subscribed.insert(port);
}

void Device::unsubscribe(Port* port) {
	auto it = subscribed.find(port);
	if (it != subscribed.end())
		subscribed.erase(it);
}

void Device::processBuffer(const float* input, int inputStride, float* output, int outputStride, int frames) {
	for (Port* port : subscribed) {
		port->processInput(input + port->offset, inputStride, frames);
	}
	for (Port* port : subscribed) {
		port->processBuffer(input + port->offset, inputStride, output + port->offset, outputStride, frames);
	}
	for (Port* port : subscribed) {
		port->processOutput(output + port->offset, outputStride, frames);
	}
}

void Device::onOpenStream() {
	for (Port* port : subscribed) {
		port->onOpenStream();
	}
}

void Device::onCloseStream() {
	for (Port* port : subscribed) {
		port->onCloseStream();
	}
}

////////////////////
// Port
////////////////////

Port::Port() {
	setDriverId(-1);
}

Port::~Port() {
	setDriverId(-1);
}

std::vector<int> Port::getDriverIds() {
	std::vector<int> driverIds;
	for (auto& pair : drivers) {
		driverIds.push_back(pair.first);
	}
	return driverIds;
}

void Port::setDriverId(int driverId) {
	// Unset device and driver
	setDeviceId(-1);
	driver = NULL;
	this->driverId = -1;

	if (driverId == -1) {
		// Set first driver as default
		if (!drivers.empty()) {
			driver = drivers[0].second;
			this->driverId = drivers[0].first;
		}
	}
	else {
		// Set driver with driverId
		for (auto& pair : drivers) {
			if (pair.first == driverId) {
				driver = pair.second;
				this->driverId = driverId;
				break;
			}
		}
	}
}

std::string Port::getDriverName(int driverId) {
	for (auto& pair : drivers) {
		if (pair.first == driverId) {
			return pair.second->getName();
		}
	}
	return "";
}

void Port::setDeviceId(int deviceId) {
	// Destroy device
	if (driver && this->deviceId >= 0) {
		driver->unsubscribe(this->deviceId, this);
	}
	device = NULL;
	this->deviceId = -1;

	// Create device
	if (driver && deviceId >= 0) {
		device = driver->subscribe(deviceId, this);
		this->deviceId = deviceId;
	}
}

std::string Port::getDeviceDetail(int deviceId, int offset) {
	if (!driver || !device)
		return "";
	std::string text = getDeviceName(getDeviceId());
	text += " (";
	int numInputs = device->getNumInputs();
	int numOutputs = device->getNumOutputs();
	if (offset < numInputs) {
		text += string::f("%d-%d in", offset + 1, std::min(offset + maxChannels, numInputs));
	}
	if (offset < numInputs && offset < numOutputs) {
		text += ", ";
	}
	if (offset < numOutputs) {
		text += string::f("%d-%d out", offset + 1, std::min(offset + maxChannels, numOutputs));
	}
	text += ")";
	return text;
}

int Port::getNumInputs() {
	if (!device)
		return 0;
	return std::min(device->getNumInputs() - offset, maxChannels);
}

int Port::getNumOutputs() {
	if (!device)
		return 0;
	return std::min(device->getNumOutputs() - offset, maxChannels);
}

json_t* Port::toJson() {
	json_t* rootJ = json_object();
	json_object_set_new(rootJ, "driver", json_integer(getDriverId()));
	std::string deviceName = getDeviceName(getDeviceId());
	if (!deviceName.empty())
		json_object_set_new(rootJ, "deviceName", json_string(deviceName.c_str()));
	json_object_set_new(rootJ, "sampleRate", json_integer(getSampleRate()));
	json_object_set_new(rootJ, "blockSize", json_integer(getBlockSize()));
	json_object_set_new(rootJ, "offset", json_integer(offset));
	return rootJ;
}

void Port::fromJson(json_t* rootJ) {
	json_t* driverJ = json_object_get(rootJ, "driver");
	if (driverJ)
		setDriverId(json_number_value(driverJ));

	json_t* deviceNameJ = json_object_get(rootJ, "deviceName");
	if (deviceNameJ) {
		std::string deviceName = json_string_value(deviceNameJ);
		// Search for device ID with equal name
		for (int deviceId : getDeviceIds()) {
			if (getDeviceName(deviceId) == deviceName) {
				setDeviceId(deviceId);
				break;
			}
		}
	}

	json_t* sampleRateJ = json_object_get(rootJ, "sampleRate");
	if (sampleRateJ)
		setSampleRate(json_integer_value(sampleRateJ));

	json_t* blockSizeJ = json_object_get(rootJ, "blockSize");
	if (blockSizeJ)
		setBlockSize(json_integer_value(blockSizeJ));

	json_t* offsetJ = json_object_get(rootJ, "offset");
	if (offsetJ)
		offset = json_integer_value(offsetJ);
}


////////////////////
// audio
////////////////////

void init() {
}

void destroy() {
	for (auto& pair : drivers) {
		delete pair.second;
	}
	drivers.clear();
}

void addDriver(int driverId, Driver* driver) {
	assert(driver);
	drivers.push_back(std::make_pair(driverId, driver));
}


} // namespace audio
} // namespace rack
