#include "global_pre.hpp"
#include "app.hpp"
#include "midi.hpp"
#include "MPEBaseWidget.hpp"
#include "global_ui.hpp"

using namespace rack ;

struct MidiDriverItem : MenuItem {
	MidiIO *midiIO;
	int driverId;
	void onAction(EventAction &e) override {
		midiIO->setDriverId(driverId);
	}
};

struct MidiDriverChoice : LedDisplayChoice {
	MPEBaseWidget *midiWidget;
	void onAction(EventAction &e) override {
#ifdef USE_VST2
		Menu *menu = rack::global_ui->ui.gScene->createMenu();
#else
		Menu *menu = gScene->createMenu();
#endif // USE_VST2
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "MIDI interface"));
		for (int driverId : midiWidget->midiIO->getDeviceIds()) {
			MidiDriverItem *item = new MidiDriverItem();
			item->midiIO = midiWidget->midiIO;
			item->driverId = driverId;
			item->text = midiWidget->midiIO->getDriverName(driverId);
			item->rightText = CHECKMARK(item->driverId == midiWidget->midiIO->driverId);
			menu->addChild(item);
		}
	}
	void step() override {
		text = midiWidget->midiIO->getDriverName(midiWidget->midiIO->driverId);
	}
};

struct MidiDeviceItem : MenuItem {
	MidiIO *midiIO;
	int device;
	void onAction(EventAction &e) override {
		midiIO->setDeviceId(device);
	}
};

struct MidiDeviceChoice : LedDisplayChoice {
	MPEBaseWidget *midiWidget;
	void onAction(EventAction &e) override {
#ifdef USE_VST2
		Menu *menu = rack::global_ui->ui.gScene->createMenu();
#else
		Menu *menu = gScene->createMenu();
#endif // USE_VST2
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "MIDI device"));
		{
			MidiDeviceItem *item = new MidiDeviceItem();
			item->midiIO = midiWidget->midiIO;
			item->device = -1;
			item->text = "(No device)";
			item->rightText = CHECKMARK(item->device == midiWidget->midiIO->deviceId);
			menu->addChild(item);
		}
		int driverIdCount = midiWidget->midiIO->getDeviceIds().size();
		for (int device = 0; device < driverIdCount; device++) {
			MidiDeviceItem *item = new MidiDeviceItem();
			item->midiIO = midiWidget->midiIO;
			item->device = device;
			item->text = midiWidget->midiIO->getDeviceName(device);
			item->rightText = CHECKMARK(item->device == midiWidget->midiIO->deviceId);
			menu->addChild(item);
		}
	}
	void step() override {
		text = midiWidget->midiIO->getDeviceName(midiWidget->midiIO->deviceId);
		if (text.empty()) {
			text = "(No device)";
			color.a = 0.5f;
		}
		else {
			color.a = 1.f;
		}
	}
};

struct MidiChannelItem : MenuItem {
	MidiIO *midiIO;
	int channel;
	void onAction(EventAction &e) override {
		midiIO->channel = channel;
	}
};

struct MidiChannelChoice : LedDisplayChoice {
	MPEBaseWidget *midiWidget;
	void onAction(EventAction &e) override {
#ifdef USE_VST2
		Menu *menu = rack::global_ui->ui.gScene->createMenu();
#else
		Menu *menu = gScene->createMenu();
#endif // USE_VST2
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "MIDI channel"));
		for (int channel = -1; channel < 16; channel++) {
			MidiChannelItem *item = new MidiChannelItem();
			item->midiIO = midiWidget->midiIO;
			item->channel = channel;
			item->text = midiWidget->midiIO->getChannelName(channel);
			item->rightText = CHECKMARK(item->channel == midiWidget->midiIO->channel);
			menu->addChild(item);
		}
	}
	void step() override {
		text = midiWidget->midiIO->getChannelName(midiWidget->midiIO->channel);
	}
};


MPEBaseWidget::MPEBaseWidget() {
	box.size = mm2px(Vec(44, 28));

	Vec pos = Vec();

	MidiDriverChoice *driverIdChoice = Widget::create<MidiDriverChoice>(pos);
	driverIdChoice->midiWidget = this;
	addChild(driverIdChoice);
	pos = driverIdChoice->box.getBottomLeft();
	this->driverChoice = driverIdChoice;

	this->driverSeparator = Widget::create<LedDisplaySeparator>(pos);
	addChild(this->driverSeparator);

	MidiDeviceChoice *deviceChoice = Widget::create<MidiDeviceChoice>(pos);
	deviceChoice->midiWidget = this;
	addChild(deviceChoice);
	pos = deviceChoice->box.getBottomLeft();
	this->deviceChoice = deviceChoice;

	// this->deviceSeparator = Widget::create<LedDisplaySeparator>(pos);
	// addChild(this->deviceSeparator);

	// MidiChannelChoice *channelChoice = Widget::create<MidiChannelChoice>(pos);
	// channelChoice->midiWidget = this;
	// addChild(channelChoice);
	// this->channelChoice = channelChoice;

}

void MPEBaseWidget::step() {
	this->driverChoice->box.size.x = box.size.x;
	this->driverSeparator->box.size.x = box.size.x;
	this->deviceChoice->box.size.x = box.size.x;
	// this->deviceSeparator->box.size.x = box.size.x;
	// this->channelChoice->box.size.x = box.size.x;
	LedDisplay::step();
}

