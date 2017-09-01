#include <assert.h>
#include <list>
#include <algorithm>
#include <portmidi.h>
#include "core.hpp"


using namespace rack;

static bool initialized = false;

void midiInit() {
	if (initialized)
		return;

	PmError err = Pm_Initialize();
	if (err) {
		printf("Failed to initialize PortMidi: %s\n", Pm_GetErrorText(err));
		return;
	}
	initialized = true;
}


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
	PortMidiStream *stream = NULL;
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

	MidiInterface();
	~MidiInterface();
	void step();

	int getPortCount();
	std::string getPortName(int portId);
	// -1 will close the port
	void openPort(int portId);
	void pressNote(int note);
	void releaseNote(int note);
	void processMidi(long msg);
};


MidiInterface::MidiInterface() {
	params.resize(NUM_PARAMS);
	inputs.resize(NUM_INPUTS);
	outputs.resize(NUM_OUTPUTS);
	midiInit();
}

MidiInterface::~MidiInterface() {
	openPort(-1);
}

void MidiInterface::step() {
	if (stream) {
		// Read MIDI events
		PmEvent event;
		while (Pm_Read(stream, &event, 1) > 0) {
			processMidi(event.message);
		}
	}

	if (outputs[PITCH_OUTPUT]) {
		*outputs[PITCH_OUTPUT] = ((note - 64)) / 12.0;
	}
	if (outputs[GATE_OUTPUT]) {
		bool gate = pedal || !notes.empty();
		if (retrigger && retriggered) {
			gate = false;
			retriggered = false;
		}
		*outputs[GATE_OUTPUT] = gate ? 10.0 : 0.0;
	}
	if (outputs[MOD_OUTPUT]) {
		*outputs[MOD_OUTPUT] = mod / 127.0 * 10.0;
	}
	if (outputs[PITCHWHEEL_OUTPUT]) {
		*outputs[PITCHWHEEL_OUTPUT] = (pitchWheel - 64) / 64.0 * 10.0;
	}
}

int MidiInterface::getPortCount() {
	return Pm_CountDevices();
}

std::string MidiInterface::getPortName(int portId) {
	const PmDeviceInfo *info = Pm_GetDeviceInfo(portId);
	if (!info)
		return "";
	return stringf("%s: %s (%s)", info->interf, info->name, info->input ? "input" : "output");
}

void MidiInterface::openPort(int portId) {
	PmError err;

	// Close existing port
	if (stream) {
		err = Pm_Close(stream);
		if (err) {
			printf("Failed to close MIDI port: %s\n", Pm_GetErrorText(err));
		}
		stream = NULL;
	}
	this->portId = -1;

	// Open new port
	if (portId >= 0) {
		err = Pm_OpenInput(&stream, portId, NULL, 128, NULL, NULL);
		if (err) {
			printf("Failed to open MIDI port: %s\n", Pm_GetErrorText(err));
			return;
		}
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

void MidiInterface::processMidi(long msg) {
	int channel = msg & 0xf;
	int status = (msg >> 4) & 0xf;
	int data1 = (msg >> 8) & 0xff;
	int data2 = (msg >> 16) & 0xff;
	printf("channel %d status %d data1 %d data2 %d\n", channel, status, data1, data2);

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
		midiInterface->openPort(portId);
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
		midiInterface->channel = channel;
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
	box.size = Vec(15*6, 380);

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
