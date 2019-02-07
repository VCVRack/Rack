#include "app/MidiWidget.hpp"
#include "midi.hpp"
#include "helpers.hpp"


namespace rack {
namespace app {


struct MidiDriverItem : ui::MenuItem {
	midi::IO *midiIO;
	int driverId;
	void onAction(const event::Action &e) override {
		midiIO->setDriverId(driverId);
	}
};

struct MidiDriverChoice : LedDisplayChoice {
	midi::IO *midiIO;
	void onAction(const event::Action &e) override {
		if (!midiIO)
			return;

		ui::Menu *menu = createMenu();
		menu->addChild(createMenuLabel("MIDI driver"));
		for (int driverId : midiIO->getDriverIds()) {
			MidiDriverItem *item = new MidiDriverItem;
			item->midiIO = midiIO;
			item->driverId = driverId;
			item->text = midiIO->getDriverName(driverId);
			item->rightText = CHECKMARK(item->driverId == midiIO->driverId);
			menu->addChild(item);
		}
	}
	void step() override {
		if (!midiIO) {
			text = "";
			return;
		}
		text = midiIO->getDriverName(midiIO->driverId);
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
	midi::IO *midiIO;
	int deviceId;
	void onAction(const event::Action &e) override {
		midiIO->setDeviceId(deviceId);
	}
};

struct MidiDeviceChoice : LedDisplayChoice {
	midi::IO *midiIO;
	void onAction(const event::Action &e) override {
		if (!midiIO)
			return;

		ui::Menu *menu = createMenu();
		menu->addChild(createMenuLabel("MIDI device"));
		{
			MidiDeviceItem *item = new MidiDeviceItem;
			item->midiIO = midiIO;
			item->deviceId = -1;
			item->text = "(No device)";
			item->rightText = CHECKMARK(item->deviceId == midiIO->deviceId);
			menu->addChild(item);
		}
		for (int deviceId : midiIO->getDeviceIds()) {
			MidiDeviceItem *item = new MidiDeviceItem;
			item->midiIO = midiIO;
			item->deviceId = deviceId;
			item->text = midiIO->getDeviceName(deviceId);
			item->rightText = CHECKMARK(item->deviceId == midiIO->deviceId);
			menu->addChild(item);
		}
	}
	void step() override {
		if (!midiIO) {
			text = "";
			return;
		}
		text = midiIO->getDeviceName(midiIO->deviceId);
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
	midi::IO *midiIO;
	int channel;
	void onAction(const event::Action &e) override {
		midiIO->channel = channel;
	}
};

struct MidiChannelChoice : LedDisplayChoice {
	midi::IO *midiIO;
	void onAction(const event::Action &e) override {
		if (!midiIO)
			return;

		ui::Menu *menu = createMenu();
		menu->addChild(createMenuLabel("MIDI channel"));
		for (int channel : midiIO->getChannels()) {
			MidiChannelItem *item = new MidiChannelItem;
			item->midiIO = midiIO;
			item->channel = channel;
			item->text = midiIO->getChannelName(channel);
			item->rightText = CHECKMARK(item->channel == midiIO->channel);
			menu->addChild(item);
		}
	}
	void step() override {
		if (midiIO)
			text = midiIO->getChannelName(midiIO->channel);
		else
			text = "";
	}
};


void MidiWidget::setMidiIO(midi::IO *midiIO) {
	clearChildren();

	math::Vec pos;

	MidiDriverChoice *driverChoice = createWidget<MidiDriverChoice>(pos);
	driverChoice->box.size.x = box.size.x;
	driverChoice->midiIO = midiIO;
	addChild(driverChoice);
	pos = driverChoice->box.getBottomLeft();
	this->driverChoice = driverChoice;

	this->driverSeparator = createWidget<LedDisplaySeparator>(pos);
	this->driverSeparator->box.size.x = box.size.x;
	addChild(this->driverSeparator);

	MidiDeviceChoice *deviceChoice = createWidget<MidiDeviceChoice>(pos);
	deviceChoice->box.size.x = box.size.x;
	deviceChoice->midiIO = midiIO;
	addChild(deviceChoice);
	pos = deviceChoice->box.getBottomLeft();
	this->deviceChoice = deviceChoice;

	this->deviceSeparator = createWidget<LedDisplaySeparator>(pos);
	this->deviceSeparator->box.size.x = box.size.x;
	addChild(this->deviceSeparator);

	MidiChannelChoice *channelChoice = createWidget<MidiChannelChoice>(pos);
	channelChoice->box.size.x = box.size.x;
	channelChoice->midiIO = midiIO;
	addChild(channelChoice);
	this->channelChoice = channelChoice;
}


} // namespace app
} // namespace rack
