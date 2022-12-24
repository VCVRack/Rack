#include <audio.hpp>
#include <string.hpp>
#include <math.hpp>


namespace rack {
namespace audio {


static std::vector<std::pair<int, Driver*>> drivers;

////////////////////
// Driver
////////////////////


////////////////////
// Device
////////////////////

void Device::subscribe(Port* port) {
	std::lock_guard<std::mutex> lock(processMutex);
	subscribed.insert(port);
}

void Device::unsubscribe(Port* port) {
	std::lock_guard<std::mutex> lock(processMutex);
	auto it = subscribed.find(port);
	if (it != subscribed.end())
		subscribed.erase(it);
}

void Device::processBuffer(const float* input, int inputStride, float* output, int outputStride, int frames) {
	// Zero output since Ports might not write to all elements, or no Ports exist
	std::fill_n(output, frames * outputStride, 0.f);

	std::lock_guard<std::mutex> lock(processMutex);
	for (Port* port : subscribed) {
		// Setting the thread context should probably be the responsibility of Port, but because processInput() etc are overridden, this is the only good place for it.
		contextSet(port->context);
		port->processInput(input + port->inputOffset, inputStride, frames);
	}
	for (Port* port : subscribed) {
		contextSet(port->context);
		port->processBuffer(input + port->inputOffset, inputStride, output + port->outputOffset, outputStride, frames);
	}
	for (Port* port : subscribed) {
		contextSet(port->context);
		port->processOutput(output + port->outputOffset, outputStride, frames);
	}
}

void Device::onStartStream() {
	for (Port* port : subscribed) {
		contextSet(port->context);
		port->onStartStream();
	}
}

void Device::onStopStream() {
	for (Port* port : subscribed) {
		contextSet(port->context);
		port->onStopStream();
	}
}

////////////////////
// Port
////////////////////

Port::Port() {
	context = contextGet();
	reset();
}

Port::~Port() {
	setDeviceId(-1);
}

void Port::reset() {
	// Set default driver
	setDriverId(-1);
}

Driver* Port::getDriver() {
	return driver;
}

int Port::getDriverId() {
	return driverId;
}

void Port::setDriverId(int driverId) {
	// Unset device and driver
	setDeviceId(-1);
	driver = NULL;
	this->driverId = -1;

	// Find driver by ID
	driver = audio::getDriver(driverId);
	if (driver) {
		this->driverId = driverId;
	}
	else if (!drivers.empty()) {
		// Set first driver as default
		driver = drivers[0].second;
		this->driverId = drivers[0].first;
	}
	else {
		// No fallback drivers
		return;
	}

	// Set default device if exists
	try {
		int defaultDeviceId = driver->getDefaultDeviceId();
		if (defaultDeviceId >= 0)
			setDeviceId(defaultDeviceId);
	}
	catch (Exception& e) {
		WARN("Audio port could not get default device ID: %s", e.what());
	}
}

std::string Port::getDriverName() {
	if (!driver)
		return "";
	try {
		return driver->getName();
	}
	catch (Exception& e) {
		WARN("Audio port could not get driver name: %s", e.what());
		return "";
	}
}

Device* Port::getDevice() {
	return device;
}

std::vector<int> Port::getDeviceIds() {
	if (!driver)
		return {};
	try {
		return driver->getDeviceIds();
	}
	catch (Exception& e) {
		WARN("Audio port could not get device IDs: %s", e.what());
		return {};
	}
}

int Port::getDeviceId() {
	return deviceId;
}

void Port::setDeviceId(int deviceId) {
	if (!driver)
		return;
	if (deviceId == this->deviceId)
		return;
	// Destroy device
	if (this->deviceId >= 0) {
		try {
			driver->unsubscribe(this->deviceId, this);
			onStopStream();
		}
		catch (Exception& e) {
			WARN("Audio port could not unsubscribe from device: %s", e.what());
		}
	}
	device = NULL;
	this->deviceId = -1;

	// Create device
	if (deviceId >= 0) {
		try {
			device = driver->subscribe(deviceId, this);
			if (device) {
				this->deviceId = deviceId;
				onStartStream();
			}
		}
		catch (Exception& e) {
			WARN("Audio port could not subscribe to device: %s", e.what());
		}
	}
}

int Port::getDeviceNumInputs(int deviceId) {
	if (!driver)
		return 0;
	try {
		return driver->getDeviceNumInputs(deviceId);
	}
	catch (Exception& e) {
		WARN("Audio port could not get device number of inputs: %s", e.what());
		return 0;
	}
}

int Port::getDeviceNumOutputs(int deviceId) {
	if (!driver)
		return 0;
	try {
		return driver->getDeviceNumOutputs(deviceId);
	}
	catch (Exception& e) {
		WARN("Audio port could not get device number of outputs: %s", e.what());
		return 0;
	}
}

std::string Port::getDeviceName(int deviceId) {
	if (!driver)
		return "";
	try {
		return driver->getDeviceName(deviceId);
	}
	catch (Exception& e) {
		WARN("Audio port could not get device name: %s", e.what());
		return 0;
	}
}

std::set<float> Port::getSampleRates() {
	if (!device)
		return {};
	try {
		return device->getSampleRates();
	}
	catch (Exception& e) {
		WARN("Audio port could not get device sample rates: %s", e.what());
		return {};
	}
}

float Port::getSampleRate() {
	if (!device)
		return 0;
	try {
		return device->getSampleRate();
	}
	catch (Exception& e) {
		WARN("Audio port could not get device sample rate: %s", e.what());
		return 0;
	}
}

void Port::setSampleRate(float sampleRate) {
	if (!device)
		return;
	try {
		device->setSampleRate(sampleRate);
	}
	catch (Exception& e) {
		WARN("Audio port could not set device sample rate: %s", e.what());
	}
}

std::set<int> Port::getBlockSizes() {
	if (!device)
		return {};
	try {
		return device->getBlockSizes();
	}
	catch (Exception& e) {
		WARN("Audio port could not get device block sizes: %s", e.what());
		return {};
	}
}

int Port::getBlockSize() {
	if (!device)
		return 0;
	try {
		return device->getBlockSize();
	}
	catch (Exception& e) {
		WARN("Audio port could not get device block size: %s", e.what());
		return 0;
	}
}

void Port::setBlockSize(int blockSize) {
	if (!device)
		return;
	try {
		device->setBlockSize(blockSize);
	}
	catch (Exception& e) {
		WARN("Audio port could not set device block size: %s", e.what());
	}
}

int Port::getNumInputs() {
	if (!device)
		return 0;
	try {
		return math::clamp(device->getNumInputs() - inputOffset, 0, maxInputs);
	}
	catch (Exception& e) {
		WARN("Audio port could not get device number of inputs: %s", e.what());
		return 0;
	}
}

int Port::getNumOutputs() {
	if (!device)
		return 0;
	try {
		return math::clamp(device->getNumOutputs() - outputOffset, 0, maxOutputs);
	}
	catch (Exception& e) {
		WARN("Audio port could not get device number of outputs: %s", e.what());
		return 0;
	}
}

json_t* Port::toJson() {
	json_t* rootJ = json_object();
	json_object_set_new(rootJ, "driver", json_integer(getDriverId()));

	if (device) {
		try {
			std::string deviceName = device->getName();
			json_object_set_new(rootJ, "deviceName", json_string(deviceName.c_str()));
		}
		catch (Exception& e) {
			WARN("Audio port could not get device name: %s", e.what());
		}
	}

	json_object_set_new(rootJ, "sampleRate", json_real(getSampleRate()));
	json_object_set_new(rootJ, "blockSize", json_integer(getBlockSize()));
	json_object_set_new(rootJ, "inputOffset", json_integer(inputOffset));
	json_object_set_new(rootJ, "outputOffset", json_integer(outputOffset));
	return rootJ;
}

void Port::fromJson(json_t* rootJ) {
	setDriverId(-1);

	json_t* driverJ = json_object_get(rootJ, "driver");
	if (driverJ)
		setDriverId(json_number_value(driverJ));

	json_t* deviceNameJ = json_object_get(rootJ, "deviceName");
	if (deviceNameJ) {
		std::string deviceName = json_string_value(deviceNameJ);
		// Search for device ID with equal name
		for (int deviceId : getDeviceIds()) {
			std::string deviceNameCurr = getDeviceName(deviceId);
			if (deviceNameCurr == "")
				continue;
			if (deviceNameCurr == deviceName) {
				setDeviceId(deviceId);
				break;
			}
		}
	}

	json_t* sampleRateJ = json_object_get(rootJ, "sampleRate");
	if (sampleRateJ)
		setSampleRate(json_number_value(sampleRateJ));

	json_t* blockSizeJ = json_object_get(rootJ, "blockSize");
	if (blockSizeJ)
		setBlockSize(json_integer_value(blockSizeJ));

	json_t* inputOffsetJ = json_object_get(rootJ, "inputOffset");
	if (inputOffsetJ)
		inputOffset = json_integer_value(inputOffsetJ);

	json_t* outputOffsetJ = json_object_get(rootJ, "outputOffset");
	if (outputOffsetJ)
		outputOffset = json_integer_value(outputOffsetJ);
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
	assert(driverId != -1);
	drivers.push_back(std::make_pair(driverId, driver));
}

std::vector<int> getDriverIds() {
	std::vector<int> driverIds;
	for (auto& pair : drivers) {
		driverIds.push_back(pair.first);
	}
	return driverIds;
}

Driver* getDriver(int driverId) {
	if (driverId == -1)
		return NULL;
	// Search for driver by ID
	for (auto& pair : drivers) {
		if (pair.first == driverId)
			return pair.second;
	}
	return NULL;
}


} // namespace audio
} // namespace rack
