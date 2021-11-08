#include <app/AudioDisplay.hpp>
#include <ui/MenuSeparator.hpp>
#include <helpers.hpp>
#include <set>


namespace rack {
namespace app {


static std::string getDetailTemplate(std::string name, int numInputs, int inputOffset, int numOutputs, int outputOffset) {
	std::string text = name;
	text += " (";
	if (numInputs > 0) {
		text += string::f("%d-%d in", inputOffset + 1, inputOffset + numInputs);
	}
	if (numInputs > 0 && numOutputs > 0) {
		text += ", ";
	}
	if (numOutputs > 0) {
		text += string::f("%d-%d out", outputOffset + 1, outputOffset + numOutputs);
	}
	text += ")";
	return text;
}


struct AudioDriverValueItem : ui::MenuItem {
	audio::Port* port;
	int driverId;
	void onAction(const ActionEvent& e) override {
		port->setDriverId(driverId);
	}
};

static void appendAudioDriverMenu(ui::Menu* menu, audio::Port* port) {
	if (!port)
		return;

	for (int driverId : audio::getDriverIds()) {
		AudioDriverValueItem* item = new AudioDriverValueItem;
		item->port = port;
		item->driverId = driverId;
		item->text = audio::getDriver(driverId)->getName();
		item->rightText = CHECKMARK(item->driverId == port->getDriverId());
		menu->addChild(item);
	}
}

void AudioDriverChoice::onAction(const ActionEvent& e) {
	ui::Menu* menu = createMenu();
	menu->addChild(createMenuLabel("Audio driver"));
	appendAudioDriverMenu(menu, port);
}

void AudioDriverChoice::step() {
	text = "";
	if (box.size.x >= 200.0)
		text += "Driver: ";
	audio::Driver* driver = port ? port->getDriver() : NULL;
	std::string driverName = driver ? driver->getName() : "";
	if (driverName != "") {
		text += driverName;
		color.a = 1.0;
	}
	else {
		text += "(No driver)";
		color.a = 0.5;
	}
}

struct AudioDriverItem : ui::MenuItem {
	audio::Port* port;
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;
		appendAudioDriverMenu(menu, port);
		return menu;
	}
};


struct AudioDeviceValueItem : ui::MenuItem {
	audio::Port* port;
	int deviceId;
	int inputOffset;
	int outputOffset;
	void onAction(const ActionEvent& e) override {
		port->setDeviceId(deviceId);
		port->inputOffset = inputOffset;
		port->outputOffset = outputOffset;
	}
};

static void appendAudioDeviceMenu(ui::Menu* menu, audio::Port* port) {
	if (!port)
		return;

	{
		AudioDeviceValueItem* item = new AudioDeviceValueItem;
		item->port = port;
		item->deviceId = -1;
		item->text = "(No device)";
		item->rightText = CHECKMARK(item->deviceId == port->getDeviceId());
		menu->addChild(item);
	}

	for (int deviceId : port->getDeviceIds()) {
		int numDeviceInputs = port->getDeviceNumInputs(deviceId);
		int numDeviceOutputs = port->getDeviceNumOutputs(deviceId);
		std::string name = port->getDeviceName(deviceId);

		// Display only 32 channel offsets per device, because some virtual devices (e.g. ALSA) can have thousands of useless channels.
		for (int i = 0; i < 32; i++) {
			int inputOffset = i * port->maxInputs;
			int outputOffset = i * port->maxOutputs;
			if (inputOffset >= numDeviceInputs && outputOffset >= numDeviceOutputs)
				break;
			int numInputs = math::clamp(numDeviceInputs - inputOffset, 0, port->maxInputs);
			int numOutputs = math::clamp(numDeviceOutputs - outputOffset, 0, port->maxOutputs);

			AudioDeviceValueItem* item = new AudioDeviceValueItem;
			item->port = port;
			item->deviceId = deviceId;
			item->inputOffset = inputOffset;
			item->outputOffset = outputOffset;
			item->text = getDetailTemplate(name, numInputs, inputOffset, numOutputs, outputOffset);
			item->rightText = CHECKMARK(deviceId == port->getDeviceId() && inputOffset == port->inputOffset && outputOffset == port->outputOffset);
			menu->addChild(item);
		}
	}
}

void AudioDeviceChoice::onAction(const ActionEvent& e) {
	ui::Menu* menu = createMenu();
	menu->addChild(createMenuLabel("Audio device"));
	appendAudioDeviceMenu(menu, port);
}

void AudioDeviceChoice::step() {
	text = "";
	if (box.size.x >= 200.0)
		text += "Device: ";
	std::string detail = "";
	if (port && port->getDevice())
		detail = getDetailTemplate(port->getDevice()->getName(), port->getNumInputs(), port->inputOffset, port->getNumOutputs(), port->outputOffset);

	if (detail != "") {
		text += detail;
		color.a = 1.0;
	}
	else {
		if (box.size.x >= 80.0)
			text += "(No device)";
		else
			text += "No device";
		color.a = 0.5;
	}
}

struct AudioDeviceItem : ui::MenuItem {
	audio::Port* port;
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;
		appendAudioDeviceMenu(menu, port);
		return menu;
	}
};


struct AudioSampleRateValueItem : ui::MenuItem {
	audio::Port* port;
	float sampleRate;
	void onAction(const ActionEvent& e) override {
		port->setSampleRate(sampleRate);
	}
};

static void appendAudioSampleRateMenu(ui::Menu* menu, audio::Port* port) {
	if (!port)
		return;

	std::set<float> sampleRates = port->getSampleRates();
	// Add current sample rate in case it's not in the list
	sampleRates.insert(port->getSampleRate());

	if (sampleRates.empty()) {
		menu->addChild(createMenuLabel("(Locked by device)"));
	}
	for (float sampleRate : sampleRates) {
		if (sampleRate <= 0)
			continue;
		AudioSampleRateValueItem* item = new AudioSampleRateValueItem;
		item->port = port;
		item->sampleRate = sampleRate;
		item->text = string::f("%g kHz", sampleRate / 1000.0);
		item->rightText = CHECKMARK(item->sampleRate == port->getSampleRate());
		menu->addChild(item);
	}
}

void AudioSampleRateChoice::onAction(const ActionEvent& e) {
	ui::Menu* menu = createMenu();
	menu->addChild(createMenuLabel("Sample rate"));
	appendAudioSampleRateMenu(menu, port);
}

void AudioSampleRateChoice::step() {
	text = "";
	if (box.size.x >= 100.0)
		text += "Rate: ";
	float sampleRate = port ? port->getSampleRate() : 0;
	if (sampleRate > 0) {
		text += string::f("%g", sampleRate / 1000.f);
		color.a = 1.0;
	}
	else {
		text += "---";
		color.a = 0.5;
	}
	text += " kHz";
}

struct AudioSampleRateItem : ui::MenuItem {
	audio::Port* port;
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;
		appendAudioSampleRateMenu(menu, port);
		return menu;
	}
};


struct AudioBlockSizeValueItem : ui::MenuItem {
	audio::Port* port;
	int blockSize;
	void onAction(const ActionEvent& e) override {
		port->setBlockSize(blockSize);
	}
};

static void appendAudioBlockSizeMenu(ui::Menu* menu, audio::Port* port) {
	if (!port)
		return;

	std::set<int> blockSizes = port->getBlockSizes();
	// Add current block size in case it's not in the list
	blockSizes.insert(port->getBlockSize());

	if (blockSizes.empty()) {
		menu->addChild(createMenuLabel("(Locked by device)"));
	}
	for (int blockSize : blockSizes) {
		if (blockSize <= 0)
			continue;
		AudioBlockSizeValueItem* item = new AudioBlockSizeValueItem;
		item->port = port;
		item->blockSize = blockSize;
		float latency = (float) blockSize / port->getSampleRate() * 1000.0;
		item->text = string::f("%d (%.1f ms)", blockSize, latency);
		item->rightText = CHECKMARK(item->blockSize == port->getBlockSize());
		menu->addChild(item);
	}
}

void AudioBlockSizeChoice::onAction(const ActionEvent& e) {
	ui::Menu* menu = createMenu();
	menu->addChild(createMenuLabel("Block size"));
	appendAudioBlockSizeMenu(menu, port);
}

void AudioBlockSizeChoice::step() {
	text = "";
	if (box.size.x >= 100.0)
		text += "Block size: ";
	int blockSize = port ? port->getBlockSize() : 0;
	if (blockSize > 0) {
		text += string::f("%d", blockSize);
		color.a = 1.0;
	}
	else {
		text += "---";
		color.a = 0.5;
	}
}

struct AudioBlockSizeItem : ui::MenuItem {
	audio::Port* port;
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;
		appendAudioBlockSizeMenu(menu, port);
		return menu;
	}
};


void AudioDisplay::setAudioPort(audio::Port* port) {
	clearChildren();

	math::Vec pos;

	AudioDriverChoice* driverChoice = createWidget<AudioDriverChoice>(pos);
	driverChoice->box.size.x = box.size.x;
	driverChoice->port = port;
	addChild(driverChoice);
	pos = driverChoice->box.getBottomLeft();
	this->driverChoice = driverChoice;

	this->driverSeparator = createWidget<LedDisplaySeparator>(pos);
	this->driverSeparator->box.size.x = box.size.x;
	addChild(this->driverSeparator);

	AudioDeviceChoice* deviceChoice = createWidget<AudioDeviceChoice>(pos);
	deviceChoice->box.size.x = box.size.x;
	deviceChoice->port = port;
	addChild(deviceChoice);
	pos = deviceChoice->box.getBottomLeft();
	this->deviceChoice = deviceChoice;

	this->deviceSeparator = createWidget<LedDisplaySeparator>(pos);
	this->deviceSeparator->box.size.x = box.size.x;
	addChild(this->deviceSeparator);

	AudioSampleRateChoice* sampleRateChoice = createWidget<AudioSampleRateChoice>(pos);
	sampleRateChoice->box.size.x = box.size.x / 2;
	sampleRateChoice->port = port;
	addChild(sampleRateChoice);
	this->sampleRateChoice = sampleRateChoice;

	this->sampleRateSeparator = createWidget<LedDisplaySeparator>(pos);
	this->sampleRateSeparator->box.pos.x = box.size.x / 2;
	this->sampleRateSeparator->box.size.y = this->sampleRateChoice->box.size.y;
	addChild(this->sampleRateSeparator);

	AudioBlockSizeChoice* bufferSizeChoice = createWidget<AudioBlockSizeChoice>(pos);
	bufferSizeChoice->box.pos.x = box.size.x / 2;
	bufferSizeChoice->box.size.x = box.size.x / 2;
	bufferSizeChoice->port = port;
	addChild(bufferSizeChoice);
	this->bufferSizeChoice = bufferSizeChoice;
}


void AudioDeviceMenuChoice::onAction(const ActionEvent& e) {
	ui::Menu* menu = createMenu();
	appendAudioMenu(menu, port);
}


void AudioButton::setAudioPort(audio::Port* port) {
	this->port = port;
}


void AudioButton::onAction(const ActionEvent& e) {
	ui::Menu* menu = createMenu();
	appendAudioMenu(menu, port);
}


void appendAudioMenu(ui::Menu* menu, audio::Port* port) {
	menu->addChild(createMenuLabel("Audio driver"));
	appendAudioDriverMenu(menu, port);

	menu->addChild(new ui::MenuSeparator);
	menu->addChild(createMenuLabel("Audio device"));
	appendAudioDeviceMenu(menu, port);

	menu->addChild(new ui::MenuSeparator);
	menu->addChild(createMenuLabel("Sample rate"));
	appendAudioSampleRateMenu(menu, port);

	menu->addChild(new ui::MenuSeparator);
	menu->addChild(createMenuLabel("Block size"));
	appendAudioBlockSizeMenu(menu, port);

	// Uncomment this to use sub-menus instead of one big menu.

	// AudioDriverItem* driverItem = createMenuItem<AudioDriverItem>("Audio driver", RIGHT_ARROW);
	// driverItem->port = port;
	// menu->addChild(driverItem);

	// AudioDeviceItem* deviceItem = createMenuItem<AudioDeviceItem>("Audio device", RIGHT_ARROW);
	// deviceItem->port = port;
	// menu->addChild(deviceItem);

	// AudioSampleRateItem* sampleRateItem = createMenuItem<AudioSampleRateItem>("Sample rate", RIGHT_ARROW);
	// sampleRateItem->port = port;
	// menu->addChild(sampleRateItem);

	// AudioBlockSizeItem* blockSizeItem = createMenuItem<AudioBlockSizeItem>("Block size", RIGHT_ARROW);
	// blockSizeItem->port = port;
	// menu->addChild(blockSizeItem);
}


} // namespace app
} // namespace rack
