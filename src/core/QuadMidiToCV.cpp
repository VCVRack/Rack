#include <list>
#include <algorithm>
#include "rtmidi/RtMidi.h"
#include "core.hpp"
#include "MidiIO.hpp"
#include "dsp/digital.hpp"

struct MidiKey {
	int pitch = 60;
	int at = 0; // aftertouch
	int vel = 0; // velocity
	int retriggerC = 0;
	bool gate = false;
};

struct QuadMIDIToCVInterface : MidiIO, Module {
	enum ParamIds {
		RESET_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		PITCH_OUTPUT = 0,
		GATE_OUTPUT = 4,
		VELOCITY_OUTPUT = 8,
		AT_OUTPUT = 12,
		NUM_OUTPUTS = 16
	};

	enum Modes {
		ROTATE,
		RESET,
		REASIGN
	};

	bool pedal = false;

	int mode = REASIGN;

	int getMode() const;

	void setMode(int mode);

	MidiKey activeKeys[4];
	std::list<int> open;

	SchmittTrigger resetTrigger;
	float resetLight = 0.0;

	QuadMIDIToCVInterface() : MidiIO(), Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {

	}

	~QuadMIDIToCVInterface() {
	};

	void step();

	void processMidi(std::vector<unsigned char> msg);

	json_t *toJson() {
		json_t *rootJ = json_object();
		addBaseJson(rootJ);
		return rootJ;
	}

	void fromJson(json_t *rootJ) {
		baseFromJson(rootJ);
	}

	void reset() {
		resetMidi();
	}

	void resetMidi();

};

void QuadMIDIToCVInterface::resetMidi() {

	for (int i = 0; i < 4; i++) {
		outputs[GATE_OUTPUT + i].value = 0.0;
		activeKeys[i].gate = false;
		activeKeys[i].vel = 0;
		activeKeys[i].at = 0;
	}

	open.clear();

	pedal = false;
	resetLight = 1.0;
}

void QuadMIDIToCVInterface::step() {
	static float sampleRate = engineGetSampleRate();
	static int msgsProcessed = 0;

	if (isPortOpen()) {
		std::vector<unsigned char> message;

		// midiIn->getMessage returns empty vector if there are no messages in the queue
		// NOTE: For the quadmidi we will process max 4 midi messages per step to avoid
		// problems with parallel input.
		getMessage(&message);
		while (msgsProcessed < 4 && message.size() > 0) {
			processMidi(message);
			getMessage(&message);
			msgsProcessed++;
		}
		msgsProcessed = 0;
	}


	for (int i = 0; i < 4; i++) {
		outputs[GATE_OUTPUT + i].value = activeKeys[i].gate ? 10.0 : 0;
		outputs[PITCH_OUTPUT + i].value = (activeKeys[i].pitch - 60) / 12.0;
		outputs[VELOCITY_OUTPUT + i].value = activeKeys[i].vel / 127.0 * 10.0;
		outputs[AT_OUTPUT + i].value = activeKeys[i].at / 127.0 * 10.0;
	}

	if (resetTrigger.process(params[RESET_PARAM].value)) {
		resetMidi();
		return;
	}

	if (resetLight > 0) {
		resetLight -= resetLight / 0.55 / sampleRate; // fade out light
	}

}


void QuadMIDIToCVInterface::processMidi(std::vector<unsigned char> msg) {
	int channel = msg[0] & 0xf;
	int status = (msg[0] >> 4) & 0xf;
	int data1 = msg[1];
	int data2 = msg[2];

	static int gate;

	// Filter channels
	if (this->channel >= 0 && this->channel != channel)
		return;

	switch (status) {
		// note off
		case 0x8: {
			gate = false;
		}
			break;
		case 0x9: // note on
			if (data2 > 0) {
				gate = true;
			} else {
				// For some reason, some keyboards send a "note on" event with a velocity of 0 to signal that the key has been released.
				gate = false;
			}
			break;
		case 0xa: // channel aftertouch
			for (int i = 0; i < 4; i++) {
				if (activeKeys[i].pitch == data1) {
					activeKeys[i].at = data2;
				}
			}
			return;
		case 0xb: // cc
			if (data1 == 0x40) { // pedal
				pedal = (data2 >= 64);
				if (!pedal) {
					for (int i = 0; i < 4; i++) {
						activeKeys[i].gate = false;
						open.push_back(i);
					}
				}
			}
			return;
		default:
			return;
	}

	if (!pedal && !gate) {
		for (int i = 0; i < 4; i++) {
			if (activeKeys[i].pitch == data1) {
				activeKeys[i].gate = false;
				activeKeys[i].vel = data2;
				open.push_front(i);
			}
		}
		return;
	}

	switch (mode) {
		case RESET:
		case REASIGN:
			for (int i = 0; i < 4; i++) {
				if (activeKeys[i].gate == false && std::find(open.begin(), open.end(), i) == open.end()) {
					open.push_back(i);
				}
			}

			if (open.size() == 4) {
				open.sort();
			}

			open.push_back(open.front());

			break;
		case ROTATE:
			if (open.empty()) {
				for (int i = 0; i < 4; i++) {
					open.push_back(i);
				}
			} else {
				open.sort();
			}
			break;
		default:
			fprintf(stderr, "No mode selected?!\n");
	}

	activeKeys[open.

			front()

	].
			gate = true;
	activeKeys[open.

			front()

	].
			pitch = data1;
	activeKeys[open.

			front()

	].
			vel = data2;
	fprintf(stderr,
			"Using No: %d\n", open.

					front()

	);
	open.

			pop_front();

	return;


}

int QuadMIDIToCVInterface::getMode() const {
	return mode;
}

void QuadMIDIToCVInterface::setMode(int mode) {
	resetMidi();
	QuadMIDIToCVInterface::mode = mode;
}

struct ModeItem : MenuItem {
	int mode;
	QuadMIDIToCVInterface *module;

	void onAction() {
		module->setMode(mode);
	}
};

struct ModeChoice : ChoiceButton {
	QuadMIDIToCVInterface *module;
	const std::vector<std::string> modeNames = {"ROTATE",  "RESET", "REASSIGN"};


	void onAction() {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsolutePos().plus(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		for (unsigned long i = 0; i < modeNames.size(); i++) {
			ModeItem *modeItem = new ModeItem();
			modeItem->mode = i;
			modeItem->module = module;
			modeItem->text = modeNames[i];
			menu->pushChild(modeItem);
		}
	}

	void step() {
		text = modeNames[module->getMode()];
	}
};


QuadMidiToCVWidget::QuadMidiToCVWidget() {
	QuadMIDIToCVInterface *module = new QuadMIDIToCVInterface();
	setModule(module);
	box.size = Vec(15 * 16, 380);

	{
		Panel *panel = new LightPanel();
		panel->box.size = box.size;
		addChild(panel);
	}

	float margin = 5;
	float labelHeight = 15;
	float yPos = margin;

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 365)));

	{
		Label *label = new Label();
		label->box.pos = Vec(box.size.x - margin - 12 * 15, margin);
		label->text = "Quad MIDI to CV";
		addChild(label);
		yPos = labelHeight * 2;
	}

	addParam(createParam<LEDButton>(Vec(12 * 15, labelHeight), module, QuadMIDIToCVInterface::RESET_PARAM, 0.0, 1.0,
									0.0));
	addChild(createValueLight<SmallLight<RedValueLight>>(Vec(12 * 15 + 5, labelHeight + 5), &module->resetLight));
	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "MIDI Interface";
		addChild(label);
		yPos += labelHeight + margin;

		MidiChoice *midiChoice = new MidiChoice();
		midiChoice->midiModule = dynamic_cast<MidiIO *>(module);
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
		channelChoice->midiModule = dynamic_cast<MidiIO *>(module);
		channelChoice->box.pos = Vec(margin, yPos);
		channelChoice->box.size.x = box.size.x - 10;
		addChild(channelChoice);
		yPos += channelChoice->box.size.y + margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Mode";
		addChild(label);
		yPos += labelHeight + margin;

		ModeChoice *modeChoice = new ModeChoice();
		modeChoice->module = module;
		modeChoice->box.pos = Vec(margin, yPos);
		modeChoice->box.size.x = box.size.x - 10;
		addChild(modeChoice);
		yPos += modeChoice->box.size.y + margin + 15;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "1V/Oct";
		addChild(label);
	}
	{
		Label *label = new Label();
		label->box.pos = Vec(67, yPos);
		label->text = "Gate";
		addChild(label);
	}
	{
		Label *label = new Label();
		label->box.pos = Vec(133, yPos);
		label->text = "Vel";
		addChild(label);
	}
	{
		Label *label = new Label();
		label->box.pos = Vec(195, yPos);
		label->text = "At";
		addChild(label);
	}

	yPos += labelHeight + margin;
	for (int i = 0; i < 4; i++) {
		addOutput(createOutput<PJ3410Port>(Vec(0 * (63) + 15, yPos + 5), module, i));
		addOutput(createOutput<PJ3410Port>(Vec(1 * (63) + 10, yPos + 5), module, i + 4));
		addOutput(createOutput<PJ3410Port>(Vec(2 * (63) + 10, yPos + 5), module, i + 8));
		addOutput(createOutput<PJ3410Port>(Vec(3 * (63) + 5, yPos + 5), module, i + 12));
		yPos += 40;
	}


}

void QuadMidiToCVWidget::step() {

	ModuleWidget::step();
}
