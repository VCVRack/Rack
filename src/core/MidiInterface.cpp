#include <assert.h>
#include <list>
#include <algorithm>
#include "rtmidi/RtMidi.h"
#include "core.hpp"
#include "gui.hpp"
#include "../../include/engine.hpp"


using namespace rack;

struct MidiInterface : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		PITCH_OUTPUT,
		GATE_OUTPUT,
		MOD_OUTPUT,
		PITCHWHEEL_OUTPUT,
		NUM_OUTPUTS
	};

	int portId = -1;
	RtMidiIn *midiIn = NULL;
	std::list<int> notes;
	/** Filter MIDI channel
	-1 means all MIDI channels
	*/
	int channel = -1;
	bool pedal = false;
	int note = 60; // C4, most modules should use 261.626 Hz
	int mod = 0;
	int pitchWheel = 64;
	bool retrigger = false;
	bool retriggered = false;

	MidiInterface() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
		try {
			midiIn = new RtMidiIn(RtMidi::UNSPECIFIED, "VCVRack");
		}
		catch ( RtMidiError &error ) {
			fprintf(stderr, "Failed to create RtMidiIn: %s\n", error.getMessage().c_str());
		}
	}
	~MidiInterface() {
		setPortId(-1);
	}

	void step();

	int getPortCount();
	std::string getPortName(int portId);
	// -1 will close the port
	void setPortId(int portId);
	void setChannel(int channel) {
		this->channel = channel;
	}
	void pressNote(int note);
	void releaseNote(int note);
	void processMidi(std::vector<unsigned char> msg);

	json_t *toJson() {
		json_t *rootJ = json_object();
		if (portId >= 0) {
			std::string portName = getPortName(portId);
			json_object_set_new(rootJ, "portName", json_string(portName.c_str()));
			json_object_set_new(rootJ, "channel", json_integer(channel));
		}
		return rootJ;
	}

	void fromJson(json_t *rootJ) {
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

	void initialize() {
		setPortId(-1);
	}
};


void MidiInterface::step() {
	if (midiIn->isPortOpen()) {
		std::vector<unsigned char> message;

		// midiIn->getMessage returns empty vector if there are no messages in the queue
		double stamp = midiIn->getMessage( &message );
		while (message.size() > 0) {
			processMidi(message);
			stamp = midiIn->getMessage( &message );
		}
	}

	outputs[PITCH_OUTPUT].value = ((note - 60)) / 12.0;

	bool gate = pedal || !notes.empty();
	if (retrigger && retriggered) {
		gate = false;
		retriggered = false;
	}
	outputs[GATE_OUTPUT].value = gate ? 10.0 : 0.0;
	outputs[MOD_OUTPUT].value = mod / 127.0 * 10.0;
	outputs[PITCHWHEEL_OUTPUT].value = (pitchWheel - 64) / 64.0 * 10.0;
}

int MidiInterface::getPortCount() {
	return midiIn->getPortCount();
}

std::string MidiInterface::getPortName(int portId) {
	std::string portName;
	try {
		portName = midiIn->getPortName(portId);
	}
	catch ( RtMidiError &error ) {
		fprintf(stderr, "Failed to get Port Name: %d, %s\n", portId, error.getMessage().c_str());
	}
	return portName;
}

void MidiInterface::setPortId(int portId) {
	// Close port if it was previously opened
	if (midiIn->isPortOpen()) {
		midiIn->closePort();
	}
	this->portId = -1;

	// Open new port
	if (portId >= 0) {
		midiIn->openPort(portId, "Midi Interface");
	}
	this->portId = portId;
}

void MidiInterface::pressNote(int note) {
	// Remove existing similar note
	auto it = std::find(notes.begin(), notes.end(), note);
	if (it != notes.end())
		notes.erase(it);
	// Push note
	notes.push_back(note);
	this->note = note;
	retriggered = true;
}

void MidiInterface::releaseNote(int note) {
	// Remove the note
	auto it = std::find(notes.begin(), notes.end(), note);
	if (it != notes.end())
		notes.erase(it);

	if (pedal) {
		// Don't release if pedal is held
	}
	else if (!notes.empty()) {
		// Play previous note
		auto it2 = notes.end();
		it2--;
		this->note = *it2;
		retriggered = true;
	}
}

void MidiInterface::processMidi(std::vector<unsigned char> msg) {
	int channel = msg[0] & 0xf;
	int status = (msg[0] >> 4) & 0xf;
	int data1 = msg[1];
	int data2 = msg[2];

	//fprintf(stderr, "channel %d status %d data1 %d data2 %d\n", channel, status, data1,data2);

	// Filter channels
	if (this->channel >= 0 && this->channel != channel)
		return;

	switch (status) {
	// note off
	case 0x8: {
		releaseNote(data1);
	} break;
	case 0x9: // note on
		if (data2 > 0) {
			pressNote(data1);
		}
		else {
			// For some reason, some keyboards send a "note on" event with a velocity of 0 to signal that the key has been released.
			releaseNote(data1);
		}
		break;
	case 0xb: // cc
		switch (data1) {
		case 0x01: // mod
			this->mod = data2;
			break;
		case 0x40: // sustain
			pedal = (data2 >= 64);
			releaseNote(-1);
			break;
		}
		break;
	case 0xe: // pitch wheel
		this->pitchWheel = data2;
		break;
	}
}


struct MidiItem : MenuItem {
	MidiInterface *midiInterface;
	int portId;
	void onAction() {
		midiInterface->setPortId(portId);
	}
};

struct MidiChoice : ChoiceButton {
	MidiInterface *midiInterface;
	void onAction() {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		int portCount = midiInterface->getPortCount();
		{
			MidiItem *midiItem = new MidiItem();
			midiItem->midiInterface = midiInterface;
			midiItem->portId = -1;
			midiItem->text = "No device";
			menu->pushChild(midiItem);
		}
		for (int portId = 0; portId < portCount; portId++) {
			MidiItem *midiItem = new MidiItem();
			midiItem->midiInterface = midiInterface;
			midiItem->portId = portId;
			midiItem->text = midiInterface->getPortName(portId);
			menu->pushChild(midiItem);
		}
	}
	void step() {
		std::string name = midiInterface->getPortName(midiInterface->portId);
		text = ellipsize(name, 8);
	}
};

struct ChannelItem : MenuItem {
	MidiInterface *midiInterface;
	int channel;
	void onAction() {
		midiInterface->setChannel(channel);
	}
};

struct ChannelChoice : ChoiceButton {
	MidiInterface *midiInterface;
	void onAction() {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		{
			ChannelItem *channelItem = new ChannelItem();
			channelItem->midiInterface = midiInterface;
			channelItem->channel = -1;
			channelItem->text = "All";
			menu->pushChild(channelItem);
		}
		for (int channel = 0; channel < 16; channel++) {
			ChannelItem *channelItem = new ChannelItem();
			channelItem->midiInterface = midiInterface;
			channelItem->channel = channel;
			channelItem->text = stringf("%d", channel + 1);
			menu->pushChild(channelItem);
		}
	}
	void step() {
		text = (midiInterface->channel >= 0) ? stringf("%d", midiInterface->channel + 1) : "All";
	}
};


MidiInterfaceWidget::MidiInterfaceWidget() {
	MidiInterface *module = new MidiInterface();
	setModule(module);
	box.size = Vec(15 * 6, 380);

	{
		Panel *panel = new LightPanel();
		panel->box.size = box.size;
		addChild(panel);
	}

	float margin = 5;
	float labelHeight = 15;
	float yPos = margin;

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "MIDI device";
		addChild(label);
		yPos += labelHeight + margin;

		MidiChoice *midiChoice = new MidiChoice();
		midiChoice->midiInterface = dynamic_cast<MidiInterface*>(module);
		midiChoice->box.pos = Vec(margin, yPos);
		midiChoice->box.size.x = box.size.x - 10;
		addChild(midiChoice);
		yPos += midiChoice->box.size.y + margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Channel";
		addChild(label);
		yPos += labelHeight + margin;

		ChannelChoice *channelChoice = new ChannelChoice();
		channelChoice->midiInterface = dynamic_cast<MidiInterface*>(module);
		channelChoice->box.pos = Vec(margin, yPos);
		channelChoice->box.size.x = box.size.x - 10;
		addChild(channelChoice);
		yPos += channelChoice->box.size.y + margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "1V/oct";
		addChild(label);
		yPos += labelHeight + margin;

		addOutput(createOutput<PJ3410Port>(Vec(28, yPos), module, MidiInterface::PITCH_OUTPUT));
		yPos += 37 + margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Gate";
		addChild(label);
		yPos += labelHeight + margin;

		addOutput(createOutput<PJ3410Port>(Vec(28, yPos), module, MidiInterface::GATE_OUTPUT));
		yPos += 37 + margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Mod Wheel";
		addChild(label);
		yPos += labelHeight + margin;

		addOutput(createOutput<PJ3410Port>(Vec(28, yPos), module, MidiInterface::MOD_OUTPUT));
		yPos += 37 + margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Pitch Wheel";
		addChild(label);
		yPos += labelHeight + margin;

		addOutput(createOutput<PJ3410Port>(Vec(28, yPos), module, MidiInterface::PITCHWHEEL_OUTPUT));
		yPos += 37 + margin;
	}
}

void MidiInterfaceWidget::step() {
	// Assume QWERTY
#define MIDI_KEY(key, midi) if (glfwGetKey(gWindow, key)) printf("%d\n", midi);

	// MIDI_KEY(GLFW_KEY_Z, 48);

	ModuleWidget::step();
}
