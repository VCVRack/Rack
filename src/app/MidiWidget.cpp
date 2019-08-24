#include <app/MidiWidget.hpp>
#include <helpers.hpp>


namespace rack {
namespace app {


struct MidiDriverItem : ui::MenuItem {
	midi::Port* port;
	int driverId;
	void onAction(const event::Action& e) override {
		port->setDriverId(driverId);
	}
};

struct MidiDriverChoice : LedDisplayChoice {
	midi::Port* port;
	void onAction(const event::Action& e) override {
		if (!port)
			return;

		ui::Menu* menu = createMenu();
		menu->addChild(createMenuLabel("MIDI driver"));
		for (int driverId : port->getDriverIds()) {
			MidiDriverItem* item = new MidiDriverItem;
			item->port = port;
			item->driverId = driverId;
			item->text = port->getDriverName(driverId);
			item->rightText = CHECKMARK(item->driverId == port->driverId);
			menu->addChild(item);
		}
	}
	void step() override {
		text = port ? port->getDriverName(port->driverId) : "";
		if (text.empty()) {
			text = "(No driver)";
			color.a = 0.5f;
		}
		else {
			color.a = 1.f;
		}
	}
};

struct MidiDeviceItem : ui::MenuItem {
	midi::Port* port;
	int deviceId;
	void onAction(const event::Action& e) override {
		port->setDeviceId(deviceId);
	}
};

struct MidiDeviceChoice : LedDisplayChoice {
	midi::Port* port;
	void onAction(const event::Action& e) override {
		if (!port)
			return;

		ui::Menu* menu = createMenu();
		menu->addChild(createMenuLabel("MIDI device"));
		{
			MidiDeviceItem* item = new MidiDeviceItem;
			item->port = port;
			item->deviceId = -1;
			item->text = "(No device)";
			item->rightText = CHECKMARK(item->deviceId == port->deviceId);
			menu->addChild(item);
		}
		for (int deviceId : port->getDeviceIds()) {
			MidiDeviceItem* item = new MidiDeviceItem;
			item->port = port;
			item->deviceId = deviceId;
			item->text = port->getDeviceName(deviceId);
			item->rightText = CHECKMARK(item->deviceId == port->deviceId);
			menu->addChild(item);
		}
	}
	void step() override {
		text = port ? port->getDeviceName(port->deviceId) : "";
		if (text.empty()) {
			text = "(No device)";
			color.a = 0.5f;
		}
		else {
			color.a = 1.f;
		}
	}
};

struct MidiChannelItem : ui::MenuItem {
	midi::Port* port;
	int channel;
	void onAction(const event::Action& e) override {
		port->channel = channel;
	}
};

struct MidiChannelChoice : LedDisplayChoice {
	midi::Port* port;
	void onAction(const event::Action& e) override {
		if (!port)
			return;

		ui::Menu* menu = createMenu();
		menu->addChild(createMenuLabel("MIDI channel"));
		for (int channel : port->getChannels()) {
			MidiChannelItem* item = new MidiChannelItem;
			item->port = port;
			item->channel = channel;
			item->text = port->getChannelName(channel);
			item->rightText = CHECKMARK(item->channel == port->channel);
			menu->addChild(item);
		}
	}
	void step() override {
		text = port ? port->getChannelName(port->channel) : "Channel 1";
	}
};


void MidiWidget::setMidiPort(midi::Port* port) {
	clearChildren();

	math::Vec pos;

	MidiDriverChoice* driverChoice = createWidget<MidiDriverChoice>(pos);
	driverChoice->box.size.x = box.size.x;
	driverChoice->port = port;
	addChild(driverChoice);
	pos = driverChoice->box.getBottomLeft();
	this->driverChoice = driverChoice;

	this->driverSeparator = createWidget<LedDisplaySeparator>(pos);
	this->driverSeparator->box.size.x = box.size.x;
	addChild(this->driverSeparator);

	MidiDeviceChoice* deviceChoice = createWidget<MidiDeviceChoice>(pos);
	deviceChoice->box.size.x = box.size.x;
	deviceChoice->port = port;
	addChild(deviceChoice);
	pos = deviceChoice->box.getBottomLeft();
	this->deviceChoice = deviceChoice;

	this->deviceSeparator = createWidget<LedDisplaySeparator>(pos);
	this->deviceSeparator->box.size.x = box.size.x;
	addChild(this->deviceSeparator);

	MidiChannelChoice* channelChoice = createWidget<MidiChannelChoice>(pos);
	channelChoice->box.size.x = box.size.x;
	channelChoice->port = port;
	addChild(channelChoice);
	this->channelChoice = channelChoice;
}


} // namespace app
} // namespace rack
