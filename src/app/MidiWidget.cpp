#include "app.hpp"
#include "midi.hpp"


namespace rack {


struct MidiDriverItem : MenuItem {
	MidiIO *midiIO;
	int driver;
	void onAction(EventAction &e) override {
		// midiIO->openDriver(device);
	}
};

struct MidiDriverChoice : LedDisplayChoice {
	MidiWidget *midiWidget;
	void onAction(EventAction &e) override {
		// Menu *menu = gScene->createMenu();
		// menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Audio driver"));
		// for (int driver : audioWidget->audioIO->listDrivers()) {
		// 	AudioDriverItem *item = new AudioDriverItem();
		// 	item->audioIO = audioWidget->audioIO;
		// 	item->driver = driver;
		// 	item->text = audioWidget->audioIO->getDriverName(driver);
		// 	item->rightText = CHECKMARK(item->driver == audioWidget->audioIO->driver);
		// 	menu->addChild(item);
		// }
	}
	void step() override {
		// text = audioWidget->audioIO->getDriverName(audioWidget->audioIO->driver);
	}
};

struct MidiDeviceItem : MenuItem {
	MidiIO *midiIO;
	int device;
	void onAction(EventAction &e) override {
		// midiIO->openDevice(device);
	}
};

struct MidiDeviceChoice : LedDisplayChoice {
	MidiWidget *midiWidget;
	void onAction(EventAction &e) override {
		// Menu *menu = gScene->createMenu();
		// menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Audio driver"));
		// for (int driver : audioWidget->audioIO->listDrivers()) {
		// 	AudioDriverItem *item = new AudioDriverItem();
		// 	item->audioIO = audioWidget->audioIO;
		// 	item->driver = driver;
		// 	item->text = audioWidget->audioIO->getDriverName(driver);
		// 	item->rightText = CHECKMARK(item->driver == audioWidget->audioIO->driver);
		// 	menu->addChild(item);
		// }
	}
	void step() override {
		// text = audioWidget->audioIO->getDriverName(audioWidget->audioIO->driver);
	}
};

struct MidiChannelItem : MenuItem {
	MidiIO *midiIO;
	int channel;
	void onAction(EventAction &e) override {
		// midiIO->channel = channel;
	}
};

struct MidiChannelChoice : LedDisplayChoice {
	MidiWidget *midiWidget;
	void onAction(EventAction &e) override {
		// Menu *menu = gScene->createMenu();
		// menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Audio driver"));
		// for (int driver : audioWidget->audioIO->listDrivers()) {
		// 	AudioDriverItem *item = new AudioDriverItem();
		// 	item->audioIO = audioWidget->audioIO;
		// 	item->driver = driver;
		// 	item->text = audioWidget->audioIO->getDriverName(driver);
		// 	item->rightText = CHECKMARK(item->driver == audioWidget->audioIO->driver);
		// 	menu->addChild(item);
		// }
	}
	void step() override {
		// text = audioWidget->audioIO->getDriverName(audioWidget->audioIO->driver);
	}
};


struct MidiWidget::Internal {
	LedDisplayChoice *driverChoice;
	LedDisplaySeparator *driverSeparator;
	LedDisplayChoice *deviceChoice;
	LedDisplaySeparator *deviceSeparator;
	LedDisplayChoice *channelChoice;
};

MidiWidget::MidiWidget() {
	internal = new Internal();
	box.size = mm2px(Vec(44, 28));

	Vec pos = Vec();

	MidiDriverChoice *driverChoice = Widget::create<MidiDriverChoice>(pos);
	driverChoice->midiWidget = this;
	addChild(driverChoice);
	pos = driverChoice->box.getBottomLeft();
	internal->driverChoice = driverChoice;

	internal->driverSeparator = Widget::create<LedDisplaySeparator>(pos);
	addChild(internal->driverSeparator);

	MidiDeviceChoice *deviceChoice = Widget::create<MidiDeviceChoice>(pos);
	deviceChoice->midiWidget = this;
	addChild(deviceChoice);
	pos = deviceChoice->box.getBottomLeft();
	internal->deviceChoice = deviceChoice;

	internal->deviceSeparator = Widget::create<LedDisplaySeparator>(pos);
	addChild(internal->deviceSeparator);

	MidiChannelChoice *channelChoice = Widget::create<MidiChannelChoice>(pos);
	channelChoice->midiWidget = this;
	addChild(channelChoice);
	internal->channelChoice = channelChoice;
}

MidiWidget::~MidiWidget() {
	delete internal;
}

void MidiWidget::step() {
	internal->driverChoice->box.size.x = box.size.x;
	internal->driverSeparator->box.size.x = box.size.x;
	internal->deviceChoice->box.size.x = box.size.x;
	internal->deviceSeparator->box.size.x = box.size.x;
	internal->channelChoice->box.size.x = box.size.x;
	LedDisplay::step();
}

/*
void MidiWidget::onAction(EventAction &e) {
	if (!midiIO)
		return;

	Menu *menu = gScene->createMenu();

	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "MIDI device"));
	{
		MidiDeviceItem *item = new MidiDeviceItem();
		item->midiIO = midiIO;
		item->device = -1;
		item->text = "No device";
		item->rightText = CHECKMARK(item->device == midiIO->device);
		menu->addChild(item);
	}
	for (int device = 0; device < midiIO->getDeviceCount(); device++) {
		MidiDeviceItem *item = new MidiDeviceItem();
		item->midiIO = midiIO;
		item->device = device;
		item->text = midiIO->getDeviceName(device);
		item->rightText = CHECKMARK(item->device == midiIO->device);
		menu->addChild(item);
	}
	menu->addChild(construct<MenuEntry>());

	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "MIDI channel"));
	MidiInput *midiInput = dynamic_cast<MidiInput*>(midiIO);
	if (midiInput) {
		MidiChannelItem *item = new MidiChannelItem();
		item->midiIO = midiIO;
		item->channel = -1;
		item->text = "All";
		item->rightText = CHECKMARK(item->channel == midiIO->channel);
		menu->addChild(item);
	}
	for (int channel = 0; channel < 16; channel++) {
		MidiChannelItem *item = new MidiChannelItem();
		item->midiIO = midiIO;
		item->channel = channel;
		item->text = stringf("%d", channel + 1);
		item->rightText = CHECKMARK(item->channel == midiIO->channel);
		menu->addChild(item);
	}
}
*/


} // namespace rack
