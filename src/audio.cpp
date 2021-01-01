#include <audio.hpp>
#include <string.hpp>


namespace rack {
namespace audio {


static std::vector<std::pair<int, Driver*>> drivers;

static std::string getDetailTemplate(std::string name, int numInputs, int numOutputs, int offset, int maxChannels) {
	std::string text = name;
	text += " (";
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

////////////////////
// Driver
////////////////////

std::string Driver::getDeviceDetail(int deviceId, int offset, int maxChannels) {
	if (deviceId < 0)
		return "";
	return getDetailTemplate(getDeviceName(deviceId), getDeviceNumInputs(deviceId), getDeviceNumOutputs(deviceId), offset, maxChannels);
}

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

std::string Device::getDetail(int offset, int maxChannels) {
	return getDetailTemplate(getName(), getNumInputs(), getNumOutputs(), offset, maxChannels);
}

void Device::processBuffer(const float* input, int inputStride, float* output, int outputStride, int frames) {
	for (Port* port : subscribed) {
		// Setting the thread context should probably be the responsibility of Port, but because processInput() etc are overridden, this is the only good place for it.
		contextSet(port->context);
		port->processInput(input + port->getOffset(), inputStride, frames);
	}
	for (Port* port : subscribed) {
		contextSet(port->context);
		port->processBuffer(input + port->getOffset(), inputStride, output + port->getOffset(), outputStride, frames);
	}
	for (Port* port : subscribed) {
		contextSet(port->context);
		port->processOutput(output + port->getOffset(), outputStride, frames);
	}
}

void Device::onOpenStream() {
	for (Port* port : subscribed) {
		contextSet(port->context);
		port->onOpenStream();
	}
}

void Device::onCloseStream() {
	for (Port* port : subscribed) {
		contextSet(port->context);
		port->onCloseStream();
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
	setDriverId(-1);
}

void Port::reset() {
	// Get default driver
	int firstDriverId = -1;
	std::vector<int> driverIds = getDriverIds();
	if (!driverIds.empty())
		firstDriverId = driverIds[0];

	setDriverId(firstDriverId);
	setOffset(0);
}

Driver* Port::getDriver() {
	return driver;
}

int Port::getDriverId() {
	return driverId;
}

void Port::setDriverId(int driverId) {
	if (driverId == this->driverId)
		return;
	// Unset device and driver
	setDeviceId(-1);
	driver = NULL;
	this->driverId = -1;

	// Find driver by ID
	driver = audio::getDriver(driverId);
	if (driver) {
		this->driverId = driverId;
	}
	else {
		// Set first driver as default
		driver = drivers[0].second;
		this->driverId = drivers[0].first;
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
			this->deviceId = deviceId;
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

std::string Port::getDeviceDetail(int deviceId, int offset) {
	if (!driver)
		return "";
	try {
		// Use maxChannels from Port.
		return driver->getDeviceDetail(deviceId, offset, maxChannels);
	}
	catch (Exception& e) {
		WARN("Audio port could not get device detail: %s", e.what());
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

int Port::getOffset() {
	return offset;
}

void Port::setOffset(int offset) {
	this->offset = offset;
}


int Port::getNumInputs() {
	if (!device)
		return 0;
	try {
		return std::min(device->getNumInputs() - getOffset(), maxChannels);
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
		return std::min(device->getNumOutputs() - getOffset(), maxChannels);
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
		std::string deviceName = device->getName();
		json_object_set_new(rootJ, "deviceName", json_string(deviceName.c_str()));
	}

	json_object_set_new(rootJ, "sampleRate", json_real(getSampleRate()));
	json_object_set_new(rootJ, "blockSize", json_integer(getBlockSize()));
	json_object_set_new(rootJ, "offset", json_integer(getOffset()));
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

	json_t* offsetJ = json_object_get(rootJ, "offset");
	if (offsetJ)
		setOffset(json_integer_value(offsetJ));
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

std::vector<int> getDriverIds() {
	std::vector<int> driverIds;
	for (auto& pair : drivers) {
		driverIds.push_back(pair.first);
	}
	return driverIds;
}

Driver* getDriver(int driverId) {
	// Search for driver by ID
	for (auto& pair : drivers) {
		if (pair.first == driverId)
			return pair.second;
	}
	return NULL;
}


} // namespace audio
} // namespace rack
