#include <list>
#include <algorithm>
#include "rtmidi/RtMidi.h"
#include "core.hpp"
#include "MidiInterface.hpp"

using namespace rack;

// note this is currently not thread safe but can be easily archieved by adding a mutex
RtMidiInSplitter::RtMidiInSplitter() {
	midiInMap = {};
	deviceIdMessagesMap = {};
}

int RtMidiInSplitter::openDevice(std::string deviceName) {
	int id;

	if (!midiInMap[deviceName]) {
		try {
			RtMidiIn *t = new RtMidiIn(RtMidi::UNSPECIFIED, "Rack");
			t->ignoreTypes(true, false); // TODO: make this optional!
			midiInMap[deviceName] = t;
			for (int i = 0; i < t->getPortCount(); i++) {
				if (deviceName == t->getPortName(i)) {
					t->openPort(i);
					break;
				}
			}
		}
		catch (RtMidiError &error) {
			fprintf(stderr, "Failed to create RtMidiIn: %s\n", error.getMessage().c_str());
		}
		id = 0;
		deviceIdMessagesMap[deviceName] = {};
	} else {
		id = deviceIdMessagesMap[deviceName].size();
	}

	deviceIdMessagesMap[deviceName][id] = {};
	return id;
}

std::vector<unsigned char> RtMidiInSplitter::getMessage(std::string deviceName, int id) {
	std::vector<unsigned char> next_msg, ret;
	midiInMap[deviceName]->getMessage(&next_msg);

	if (next_msg.size() > 0) {
		for (int i = 0; i < deviceIdMessagesMap[deviceName].size(); i++) {
			deviceIdMessagesMap[deviceName][i].push_back(next_msg);
		}
	}

	if (deviceIdMessagesMap[deviceName][id].size() == 0){
		return next_msg;
	}

	ret = deviceIdMessagesMap[deviceName][id].front();
	deviceIdMessagesMap[deviceName][id].pop_front();
	return ret;
}

std::vector<std::string> RtMidiInSplitter::getDevices() {
	/*This is a bit unneccessary */
	RtMidiIn *t = new RtMidiIn(RtMidi::UNSPECIFIED, "Rack");

	std::vector<std::string> names = {};

	for (int i = 0; i < t->getPortCount(); i++) {
		names.push_back(t->getPortName(i));
	}

	return names;
}

/**
 * MidiIO implements the shared functionality of all midi modules, namely:
 * + Channel Selection (including helper for storing json)
 * + Interface Selection (including helper for storing json)
 * + rtMidi initialisation (input or output)
 */
MidiIO::MidiIO(bool isOut) {
	channel = -1;
	this->isOut = isOut;
};

RtMidiInSplitter MidiIO::midiInSplitter = RtMidiInSplitter();

void MidiIO::setChannel(int channel) {
	this->channel = channel;
}

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
		deviceName = json_string_value(portNameJ);
		openDevice(deviceName);
	}

	json_t *channelJ = json_object_get(rootJ, "channel");
	if (channelJ) {
		setChannel(json_integer_value(channelJ));
	}
}

std::vector<std::string> MidiIO::getDevices() {
	return midiInSplitter.getDevices();
}

void MidiIO::openDevice(std::string deviceName) {
	id = midiInSplitter.openDevice(deviceName);
	deviceName = deviceName;
}

std::string MidiIO::getDeviceName() {
	return deviceName;
}

std::vector<unsigned char> MidiIO::getMessage() {
	return midiInSplitter.getMessage(deviceName, id);
}

bool MidiIO::isPortOpen() {
	return id > 0;
}

void MidiIO::setDeviceName(const std::string &deviceName) {
	MidiIO::deviceName = deviceName;
}

void MidiItem::onAction() {
	midiModule->resetMidi(); // reset Midi values
	midiModule->openDevice(text);
	midiModule->setDeviceName(text);
}

void MidiChoice::onAction() {
	Menu *menu = gScene->createMenu();
	menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));
	menu->box.size.x = box.size.x;

	{
		MidiItem *midiItem = new MidiItem();
		midiItem->midiModule = midiModule;
		midiItem->text = "";
		menu->pushChild(midiItem);
	}

	std::vector<std::string> deviceNames = midiModule->getDevices();
	for (int i = 0; i < deviceNames.size(); i++) {
		MidiItem *midiItem = new MidiItem();
		midiItem->midiModule = midiModule;
		midiItem->text = deviceNames[i];
		menu->pushChild(midiItem);
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

void ChannelItem::onAction() {
	midiModule->resetMidi(); // reset Midi values
	midiModule->setChannel(channel);
}

void ChannelChoice::onAction() {
	Menu *menu = gScene->createMenu();
	menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));
	menu->box.size.x = box.size.x;

	{
		ChannelItem *channelItem = new ChannelItem();
		channelItem->midiModule = midiModule;
		channelItem->channel = -1;
		channelItem->text = "All";
		menu->pushChild(channelItem);
	}
	for (int channel = 0; channel < 16; channel++) {
		ChannelItem *channelItem = new ChannelItem();
		channelItem->midiModule = midiModule;
		channelItem->channel = channel;
		channelItem->text = stringf("%d", channel + 1);
		menu->pushChild(channelItem);
	}
}

void ChannelChoice::step() {
	text = (midiModule->channel >= 0) ? stringf("%d", midiModule->channel + 1) : "All";
}