#if 0
#include <list>
#include <algorithm>
#include "core.hpp"
#include "MidiIO.hpp"


using namespace rack;


/**
 * MidiIO implements the shared functionality of all midi modules, namely:
 * + Channel Selection (including helper for storing json)
 * + Interface Selection (including helper for storing json)
 * + rtMidi initialisation (input or output)
 */
MidiIO::MidiIO(bool isOut) {
	channel = -1;
	this->isOut = isOut;

	// TODO
	// Support MIDI out
	assert(!isOut);
};

void MidiIO::setChannel(int channel) {
	this->channel = channel;
}

std::unordered_map<std::string, MidiInWrapper *> MidiIO::midiInMap = {};

json_t *MidiIO::addBaseJson(json_t *rootJ) {
	if (deviceName != "") {
		json_object_set_new(rootJ, "interfaceName", json_string(deviceName.c_str()));
		json_object_set_new(rootJ, "channel", json_integer(channel));
	}
	return rootJ;
}

void MidiIO::baseFromJson(json_t *rootJ) {
	json_t *portNameJ = json_object_get(rootJ, "interfaceName");
	if (portNameJ) {
		openDevice(json_string_value(portNameJ));
	}

	json_t *channelJ = json_object_get(rootJ, "channel");
	if (channelJ) {
		setChannel(json_integer_value(channelJ));
	}
}

std::vector<std::string> MidiIO::getDevices() {
	std::vector<std::string> names = {};

	if (isOut) {
		// TODO
		return names;
	}

	RtMidiIn *m;
	try {
		m = new RtMidiIn();
	}
	catch (RtMidiError &error) {
		warn("Failed to create RtMidiIn: %s", error.getMessage().c_str());
		return names;
	}

	for (unsigned int i = 0; i < m->getPortCount(); i++) {
		names.push_back(m->getPortName(i));
	}

	if (!isPortOpen())
		delete (m);

	return names;
}

void MidiIO::openDevice(std::string deviceName) {

	if (this->id > 0 || this->deviceName != "") {
		close();
	}

	MidiInWrapper *mw = midiInMap[deviceName];

	if (!mw) {
		try {
			mw = new MidiInWrapper();
			midiInMap[deviceName] = mw;


			for (unsigned int i = 0; i < mw->getPortCount(); i++) {
				if (deviceName == mw->getPortName(i)) {
					mw->openPort(i);
					break;
				}
			}

			if (!mw->isPortOpen()) {
				warn("Failed to create RtMidiIn: No such device %s", deviceName.c_str());
				this->deviceName = "";
				this->id = -1;
				return;
			}
		}
		catch (RtMidiError &error) {
			warn("Failed to create RtMidiIn: %s", error.getMessage().c_str());
			this->deviceName = "";
			this->id = -1;
			return;
		}
	}

	this->deviceName = deviceName;

	id = midiInMap[deviceName]->add();
	onDeviceChange();
}

void MidiIO::setIgnores(bool ignoreSysex, bool ignoreTime, bool ignoreSense) {
	bool sy = true, ti = true, se = true;

	midiInMap[deviceName]->ignoresMap[id].midiSysex = ignoreSysex;
	midiInMap[deviceName]->ignoresMap[id].midiTime = ignoreTime;
	midiInMap[deviceName]->ignoresMap[id].midiSense = ignoreSense;

	for (auto kv : midiInMap[deviceName]->ignoresMap) {
		sy = sy && kv.second.midiSysex;
		ti = ti && kv.second.midiTime;
		se = se && kv.second.midiSense;
	}

	midiInMap[deviceName]->ignoreTypes(se, ti, se);


}

std::string MidiIO::getDeviceName() {
	return deviceName;
}

double MidiIO::getMessage(std::vector<unsigned char> *msg) {
	MidiMessage next_msg = MidiMessage();

	MidiInWrapper *mw = midiInMap[deviceName];

	if (!mw) {
		warn("Device not opened!: %s", deviceName.c_str());
		return 0;
	}

	next_msg.timeStamp = mw->getMessage(&next_msg.bytes);
	if (next_msg.bytes.size() > 0) {
		for (auto &kv : mw->idMessagesMap) {

			kv.second.push_back(next_msg);
		}
	}


	if (mw->idMessagesMap[id].size() > 0) {
		next_msg = mw->idMessagesMap[id].front();
		mw->idMessagesMap[id].pop_front();
	}

	*msg = next_msg.bytes;

	return next_msg.timeStamp;
}

bool MidiIO::isPortOpen() {
	return id > 0;
}

void MidiIO::close() {

	MidiInWrapper *mw = midiInMap[deviceName];

	if (!mw || id < 0) {
		//warn("Trying to close already closed device!");
		return;
	}

	setIgnores(); // reset ignore types for this instance

	mw->erase(id);

	if (mw->idMessagesMap.size() == 0) {
		mw->closePort();
		midiInMap.erase(deviceName);
		delete (mw);
	}

	id = -1;
	deviceName = "";
}


void MidiItem::onAction(EventAction &e) {
	midiModule->resetMidi(); // reset Midi values
	midiModule->openDevice(text);
}

void MidiChoice::onAction(EventAction &e) {
	Menu *menu = gScene->createMenu();
	menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
	menu->box.size.x = box.size.x;

	{
		MidiItem *midiItem = new MidiItem();
		midiItem->midiModule = midiModule;
		midiItem->text = "";
		menu->addChild(midiItem);
	}

	std::vector<std::string> deviceNames = midiModule->getDevices();
	for (unsigned int i = 0; i < deviceNames.size(); i++) {
		MidiItem *midiItem = new MidiItem();
		midiItem->midiModule = midiModule;
		midiItem->text = deviceNames[i];
		menu->addChild(midiItem);
	}
}

void MidiChoice::step() {
	if (midiModule->getDeviceName() == "") {
		text = "No Device";
		return;
	}
	std::string name = midiModule->getDeviceName();
	text = ellipsize(name, 15);
}

void ChannelItem::onAction(EventAction &e) {
	midiModule->resetMidi(); // reset Midi values
	midiModule->setChannel(channel);
}

void ChannelChoice::onAction(EventAction &e) {
	Menu *menu = gScene->createMenu();
	menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
	menu->box.size.x = box.size.x;

	{
		ChannelItem *channelItem = new ChannelItem();
		channelItem->midiModule = midiModule;
		channelItem->channel = -1;
		channelItem->text = "All";
		menu->addChild(channelItem);
	}
	for (int channel = 0; channel < 16; channel++) {
		ChannelItem *channelItem = new ChannelItem();
		channelItem->midiModule = midiModule;
		channelItem->channel = channel;
		channelItem->text = stringf("%d", channel + 1);
		menu->addChild(channelItem);
	}
}

void ChannelChoice::step() {
	text = (midiModule->channel >= 0) ? stringf("%d", midiModule->channel + 1) : "All";
}
#endif