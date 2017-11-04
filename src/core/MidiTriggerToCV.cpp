#include <list>
#include <algorithm>
#include "rtmidi/RtMidi.h"
#include "core.hpp"
#include "MidiIO.hpp"
#include "dsp/digital.hpp"

using namespace rack;

/*
 * MIDIToCVInterface converts midi note on/off events, velocity , channel aftertouch, pitch wheel and mod weel to
 * CV
 */
struct MIDITriggerToCVInterface : MidiIO, Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS = 16
	};

	int trigger[NUM_OUTPUTS];
	int triggerNum[NUM_OUTPUTS];
	bool triggerNumInited[NUM_OUTPUTS];
	bool onFocus[NUM_OUTPUTS];

	MIDITriggerToCVInterface() : MidiIO(), Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			trigger[i] = 0;
			triggerNum[i] = i;
			onFocus[i] = false;
		}
	}

	~MIDITriggerToCVInterface() {
	}

	void step() override;

	void processMidi(std::vector<unsigned char> msg);

	void resetMidi() override;

	virtual json_t *toJson() override {
		json_t *rootJ = json_object();
		addBaseJson(rootJ);
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			json_object_set_new(rootJ, std::to_string(i).c_str(), json_integer(triggerNum[i]));
		}
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		baseFromJson(rootJ);
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			json_t *ccNumJ = json_object_get(rootJ, std::to_string(i).c_str());
			if (ccNumJ) {
				triggerNum[i] = json_integer_value(ccNumJ);
				triggerNumInited[i] = true;
			}

		}
	}

	void reset() override {
		resetMidi();
	}

};


void MIDITriggerToCVInterface::step() {
	if (isPortOpen()) {
		std::vector<unsigned char> message;

		// midiIn->getMessage returns empty vector if there are no messages in the queue
		getMessage(&message);
		while (message.size() > 0) {
			processMidi(message);
			getMessage(&message);
		}
	}

	for (int i = 0; i < NUM_OUTPUTS; i++) {
		// Note: Could have an option to select between gate and velocity
		// but trigger seams more useful
		// outputs[i].value = trigger[i] / 127.0 * 10;
		outputs[i].value = trigger[i] > 0 ? 10.0 : 0.0;
	}
}

void MIDITriggerToCVInterface::resetMidi() {
	for (int i = 0; i < NUM_OUTPUTS; i++) {
		trigger[i] = 0;
	}
};

void MIDITriggerToCVInterface::processMidi(std::vector<unsigned char> msg) {
	int channel = msg[0] & 0xf;
	int status = (msg[0] >> 4) & 0xf;
	int data1 = msg[1];
	int data2 = msg[2];

	//fprintf(stderr, "channel %d status %d data1 %d data2 %d\n", channel, status, data1,data2);

	// Filter channels
	if (this->channel >= 0 && this->channel != channel)
		return;

	if (status == 0x8) { // note off
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			if (data1 == triggerNum[i]) {
				trigger[i] = data2;
			}
		}
		return;
	}

	if (status == 0x9) { // note on
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			if (onFocus[i]) {
				this->triggerNum[i] = data1;
			}
		}

		for (int i = 0; i < NUM_OUTPUTS; i++) {
			if (data1 == triggerNum[i]) {
				trigger[i] = data2;
			}
		}
	}

}

struct TriggerTextField : TextField {
	void onTextChange() override;

	void draw(NVGcontext *vg) override;

	void onMouseDown(EventMouseDown &e) override;

	void onMouseUp(EventMouseUp &e) override;

	void onMouseLeave(EventMouseLeave &e) override;


	int *triggerNum;
	bool *inited;
	bool *onFocus;
};

void TriggerTextField::draw(NVGcontext *vg) {
	/* This is necessary, since the save
	 * file is loaded after constructing the widget*/
	if (*inited) {
		*inited = false;
		text = std::to_string(*triggerNum);
	}

	if (*onFocus) {
		text = std::to_string(*triggerNum);
	}

	TextField::draw(vg);
}

void TriggerTextField::onTextChange() {
	if (text.size() > 0) {
		try {
			*triggerNum = std::stoi(text);
			// Only allow valid cc numbers
			if (*triggerNum < 0 || *triggerNum > 127 || text.size() > 3) {
				text = "";
				begin = end = 0;
				*triggerNum = -1;
			}
		} catch (...) {
			text = "";
			begin = end = 0;
			*triggerNum = -1;
		}
	};
}

void TriggerTextField::onMouseDown(EventMouseDown &e) {
	if (e.button == 1) {
		*onFocus = true;
	}
}

void TriggerTextField::onMouseUp(EventMouseUp &e) {
	if (e.button == 1) {
		*onFocus = false;
	}

}

void TriggerTextField::onMouseLeave(EventMouseLeave &e) {
	*onFocus = false;
}

MIDITriggerToCVWidget::MIDITriggerToCVWidget() {
	MIDITriggerToCVInterface *module = new MIDITriggerToCVInterface();
	setModule(module);
	box.size = Vec(16 * 15, 380);

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
		label->box.pos = Vec(box.size.x - margin - 11 * 15, margin);
		label->text = "MIDI Trigger to CV";
		addChild(label);
		yPos = labelHeight * 2;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "MIDI Interface";
		addChild(label);

		MidiChoice *midiChoice = new MidiChoice();
		midiChoice->midiModule = dynamic_cast<MidiIO *>(module);
		midiChoice->box.pos = Vec((box.size.x - 10) / 2 + margin, yPos);
		midiChoice->box.size.x = (box.size.x / 2.0) - margin;
		addChild(midiChoice);
		yPos += midiChoice->box.size.y + margin;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Channel";
		addChild(label);

		ChannelChoice *channelChoice = new ChannelChoice();
		channelChoice->midiModule = dynamic_cast<MidiIO *>(module);
		channelChoice->box.pos = Vec((box.size.x - 10) / 2 + margin, yPos);
		channelChoice->box.size.x = (box.size.x / 2.0) - margin;
		addChild(channelChoice);
		yPos += channelChoice->box.size.y + margin * 3;
	}

	for (int i = 0; i < MIDITriggerToCVInterface::NUM_OUTPUTS; i++) {
		TriggerTextField *triggerNumChoice = new TriggerTextField();
		triggerNumChoice->triggerNum = &module->triggerNum[i];
		triggerNumChoice->inited = &module->triggerNumInited[i];
		triggerNumChoice->onFocus = &module->onFocus[i];
		triggerNumChoice->text = std::to_string(module->triggerNum[i]);
		triggerNumChoice->box.pos = Vec(11 + (i % 4) * (63), yPos);
		triggerNumChoice->box.size.x = 29;

		addChild(triggerNumChoice);

		yPos += labelHeight + margin;
		addOutput(createOutput<PJ3410Port>(Vec((i % 4) * (63) + 10, yPos + 5), module, i));

		if ((i + 1) % 4 == 0) {
			yPos += 47 + margin;
		} else {
			yPos -= labelHeight + margin;
		}
	}
}

void MIDITriggerToCVWidget::step() {

	ModuleWidget::step();
}
