#include <assert.h>
#include <list>
#include <algorithm>
#include <portmidi.h>
#include "core.hpp"


using namespace rack;


void midiInit() {
	PmError err = Pm_Initialize();
	if (err) {
		printf("Failed to initialize PortMidi: %s\n", Pm_GetErrorText(err));
		return;
	}
}


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

	PortMidiStream *stream = NULL;
	std::list<int> notes;
	bool pedal = false;
	int note = 60; // C4, most modules should use 261.626 Hz
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

	if (outputs[GATE_OUTPUT]) {
		bool gate = pedal || !notes.empty();
		if (retrigger && retriggered) {
			gate = false;
			retriggered = false;
		}
		*outputs[GATE_OUTPUT] = gate ? 5.0 : 0.0;
	}
	if (outputs[PITCH_OUTPUT]) {
		*outputs[PITCH_OUTPUT] = ((note - 64) + 2.0*(pitchWheel - 64) / 64.0) / 12.0;
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

	// Open new port
	if (portId >= 0) {
		err = Pm_OpenInput(&stream, portId, NULL, 128, NULL, NULL);
		if (err) {
			printf("Failed to open MIDI port: %s\n", Pm_GetErrorText(err));
			return;
		}
	}
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

	if (channel != 0)
		return;
	printf("channel %d status %d data1 %d data2 %d\n", channel, status, data1, data2);

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
		midiInterface->openPort(portId);
	}
};

struct MidiChoice : ChoiceButton {
	MidiInterface *midiInterface;
	void onAction() {
		MenuOverlay *overlay = new MenuOverlay();
		Menu *menu = new Menu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));

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
		overlay->addChild(menu);
		gScene->setOverlay(overlay);
	}
};


MidiInterfaceWidget::MidiInterfaceWidget() : ModuleWidget(new MidiInterface()) {
	box.size = Vec(15*8, 380);

	float margin = 5;
	float yPos = margin;

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "MIDI Interface";
		addChild(label);
		yPos += label->box.size.y + margin;
	}

	{
		MidiChoice *midiChoice = new MidiChoice();
		midiChoice->midiInterface = dynamic_cast<MidiInterface*>(module);
		midiChoice->text = "MIDI device";
		midiChoice->box.pos = Vec(margin, yPos);
		midiChoice->box.size.x = box.size.x - 10;
		addChild(midiChoice);
		yPos += midiChoice->box.size.y + margin;
	}

	yPos += 5;
	addOutput(createOutput(Vec(25, yPos), module, MidiInterface::PITCH_OUTPUT));
	addOutput(createOutput(Vec(75, yPos), module, MidiInterface::GATE_OUTPUT));
	yPos += 25 + margin;

	{
		Label *pitchLabel = new Label();
		pitchLabel->box.pos = Vec(25-12, yPos);
		pitchLabel->text = "Pitch";
		addChild(pitchLabel);

		Label *gateLabel = new Label();
		gateLabel->box.pos = Vec(75-12, yPos);
		gateLabel->text = "Gate";
		addChild(gateLabel);

		yPos += pitchLabel->box.size.y + margin;
	}
}

void MidiInterfaceWidget::draw(NVGcontext *vg) {
	bndBackground(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);
	ModuleWidget::draw(vg);
}
