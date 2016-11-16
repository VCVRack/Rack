#include "core.hpp"
#include <assert.h>
#include <rtmidi/RtMidi.h>
#include <list>
#include <algorithm>


struct MidiInterface : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		GATE_OUTPUT,
		PITCH_OUTPUT,
		NUM_OUTPUTS
	};

	RtMidiIn midi;
	std::list<int> notes;
	bool pedal = false;
	bool gate = false;
	int note = 64; // C4
	int pitchWheel = 64;

	MidiInterface();
	~MidiInterface();
	void step();

	void openPort(int portId);
	void closePort();
	void pressNote(int note);
	void releaseNote(int note);
	void processMidi(long msg);
};


void midiCallback(double timeStamp, std::vector<unsigned char> *message, void *userData) {
	MidiInterface *that = (MidiInterface*) userData;
	if (message->size() < 3)
		return;

	long msg = (message->at(0)) | (message->at(1) << 8) | (message->at(2) << 16);
	that->processMidi(msg);
}

MidiInterface::MidiInterface() {
	params.resize(NUM_PARAMS);
	inputs.resize(NUM_INPUTS);
	outputs.resize(NUM_OUTPUTS);
	midi.setCallback(midiCallback, this);
}

MidiInterface::~MidiInterface() {
	closePort();
}

void MidiInterface::step() {
	if (outputs[GATE_OUTPUT]) {
		*outputs[GATE_OUTPUT] = gate ? 5.0 : 0.0;
	}
	if (outputs[PITCH_OUTPUT]) {
		*outputs[PITCH_OUTPUT] = ((note - 64) + 2.0*(pitchWheel - 64) / 64.0) / 12.0;
	}
}

void MidiInterface::openPort(int portId) {
	closePort();
	try {
		midi.openPort(portId);
	}
	catch (RtMidiError &e) {
		printf("Could not open midi port: %s\n", e.what());
	}
}

void MidiInterface::closePort() {
	if (!midi.isPortOpen())
		return;
	midi.closePort();
}

void MidiInterface::pressNote(int note) {
	auto it = std::find(notes.begin(), notes.end(), note);
	if (it != notes.end())
		notes.erase(it);
	notes.push_back(note);
	this->gate = true;
	this->note = note;
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
	}
	else {
		// No notes are held, turn the gate off
		this->gate = false;
	}
}

void MidiInterface::processMidi(long msg) {
	int channel = msg & 0xf;
	int status = (msg >> 4) & 0xf;
	int data1 = (msg >> 8) & 0xff;
	int data2 = (msg >> 16) & 0xff;

	if (channel != 0)
		return;
	printf("channel %d status %d data1 %d data2 %d\n", channel, status, data1, data2);

	switch (status) {
		// note off
		case 0x8: {
			releaseNote(data1);
		} break;
		case 0x9: // note on
			if (data2) {
				pressNote(data1);
			}
			else {
				// For some reason, some keyboards send a "note on" event with a velocity of 0 to signal that the key has been released.
				releaseNote(data1);
			}
			break;
		case 0xb: // cc
			switch (data1) {
				case 0x40:
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
		midiInterface->closePort();
		midiInterface->openPort(portId);
	}
};

struct MidiChoice : ChoiceButton {
	MidiInterface *midiInterface;
	void onAction() {
		MenuOverlay *overlay = new MenuOverlay();
		Menu *menu = new Menu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));

		int portCount = midiInterface->midi.getPortCount();
		if (portCount == 0) {
			MenuLabel *label = new MenuLabel();
			label->text = "No MIDI devices";
			menu->pushChild(label);
		}
		for (int portId = 0; portId < portCount; portId++) {
			MidiItem *midiItem = new MidiItem();
			midiItem->midiInterface = midiInterface;
			midiItem->portId = portId;
			midiItem->text = midiInterface->midi.getPortName();
			menu->pushChild(midiItem);
		}
		overlay->addChild(menu);
		gScene->addChild(overlay);
	}
};


MidiInterfaceWidget::MidiInterfaceWidget() : ModuleWidget(new MidiInterface()) {
	box.size = Vec(15*8, 380);
	outputs.resize(MidiInterface::NUM_OUTPUTS);

	createOutputPort(this, MidiInterface::GATE_OUTPUT, Vec(15, 100));
	createOutputPort(this, MidiInterface::PITCH_OUTPUT, Vec(70, 100));

	MidiChoice *midiChoice = new MidiChoice();
	midiChoice->midiInterface = dynamic_cast<MidiInterface*>(module);
	midiChoice->text = "MIDI Interface";
	midiChoice->box.pos = Vec(0, 0);
	midiChoice->box.size.x = box.size.x;
	addChild(midiChoice);
}

void MidiInterfaceWidget::draw(NVGcontext *vg) {
	bndBackground(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);
	ModuleWidget::draw(vg);
}
