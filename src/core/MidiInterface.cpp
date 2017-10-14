#include <list>
#include <algorithm>
#include "rtmidi/RtMidi.h"
#include "core.hpp"
#include "MidiInterface.hpp"

using namespace rack;

/**
 * MidiIO implements the shared functionality of all midi modules, namely:
 * + Channel Selection (including helper for storing json)
 * + Interface Selection (including helper for storing json)
 * + rtMidi initialisation (input or output)
 */
MidiIO::MidiIO(bool isOut) {
	portId = -1;
	rtMidi = NULL;
	channel = -1;

	/*
	 * If isOut is set to true, creates a RtMidiOut, RtMidiIn otherwise
	 */
	try {
		if (isOut) {
			rtMidi = new RtMidiOut(RtMidi::UNSPECIFIED, "Rack");
		} else {
			rtMidi = new RtMidiIn(RtMidi::UNSPECIFIED, "Rack");
		}
	}
	catch (RtMidiError &error) {
		fprintf(stderr, "Failed to create RtMidiIn: %s\n", error.getMessage().c_str());
	}
};

void MidiIO::setChannel(int channel) {
	this->channel = channel;
}

json_t *MidiIO::addBaseJson(json_t *rootJ) {
	if (portId >= 0) {
		std::string portName = getPortName(portId);
		json_object_set_new(rootJ, "portName", json_string(portName.c_str()));
		json_object_set_new(rootJ, "channel", json_integer(channel));
	}
	return rootJ;
}

void MidiIO::baseFromJson(json_t *rootJ) {
	json_t *portNameJ = json_object_get(rootJ, "portName");
	if (portNameJ) {
		std::string portName = json_string_value(portNameJ);
		for (int i = 0; i < getPortCount(); i++) {
			if (portName == getPortName(i)) {
				setPortId(i);
				break;
			}
		}
	}

	json_t *channelJ = json_object_get(rootJ, "channel");
	if (channelJ) {
		setChannel(json_integer_value(channelJ));
	}
}


int MidiIO::getPortCount() {
	return rtMidi->getPortCount();
}

std::string MidiIO::getPortName(int portId) {
	std::string portName;
	try {
		portName = rtMidi->getPortName(portId);
	}
	catch (RtMidiError &error) {
		fprintf(stderr, "Failed to get Port Name: %d, %s\n", portId, error.getMessage().c_str());
	}
	return portName;
}

void MidiIO::setPortId(int portId) {

	// Close port if it was previously opened
	if (rtMidi->isPortOpen()) {
		rtMidi->closePort();
	}
	this->portId = -1;

	// Open new port
	if (portId >= 0) {
		rtMidi->openPort(portId, "Midi Interface");
	}
	this->portId = portId;
}


void MidiItem::onAction() {
	midiModule->resetMidi(); // reset Midi values
	midiModule->setPortId(portId);
}

void MidiChoice::onAction() {
	Menu *menu = gScene->createMenu();
	menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));
	menu->box.size.x = box.size.x;

	int portCount = midiModule->getPortCount();
	{
		MidiItem *midiItem = new MidiItem();
		midiItem->midiModule = midiModule;
		midiItem->portId = -1;
		midiItem->text = "No device";
		menu->pushChild(midiItem);
	}
	for (int portId = 0; portId < portCount; portId++) {
		MidiItem *midiItem = new MidiItem();
		midiItem->midiModule = midiModule;
		midiItem->portId = portId;
		midiItem->text = midiModule->getPortName(portId);
		menu->pushChild(midiItem);
	}
}

void MidiChoice::step() {
	if (midiModule->portId < 0) {
		text = "No Device";
		return;
	}
	std::string name = midiModule->getPortName(midiModule->portId);
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