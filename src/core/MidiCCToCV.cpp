#include <list>
#include <algorithm>
#include "rtmidi/RtMidi.h"
#include "core.hpp"
#include "MidiInterface.hpp"

/*
 * MIDIToCVInterface converts midi note on/off events, velocity , channel aftertouch, pitch wheel and mod weel to
 * CV
 */
struct MIDICCToCVInterface : MidiIO, Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS = 16
	};

	int cc[NUM_OUTPUTS];
	int ccNum[NUM_OUTPUTS];
	bool ccNumInited[NUM_OUTPUTS];

	MIDICCToCVInterface() : MidiIO(), Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			cc[i] = 0;
			ccNum[i] = i;
		}
	}

	~MIDICCToCVInterface() {
		setPortId(-1);
	}

	void step();

	void processMidi(std::vector<unsigned char> msg);

	virtual void resetMidi();

	virtual json_t *toJson() {
		json_t *rootJ = json_object();
		addBaseJson(rootJ);
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			json_object_set_new(rootJ, std::to_string(i).c_str(), json_integer(ccNum[i]));
		}
		return rootJ;
	}

	virtual void fromJson(json_t *rootJ) {
		baseFromJson(rootJ);
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			json_t *ccNumJ = json_object_get(rootJ, std::to_string(i).c_str());
			if (ccNumJ) {
				ccNum[i] = json_integer_value(ccNumJ);
				ccNumInited[i] = true;
			}

		}
	}

	virtual void initialize() {
		setPortId(-1);
	}

};


void MIDICCToCVInterface::step() {
	if (rtMidi->isPortOpen()) {
		std::vector<unsigned char> message;

		// midiIn->getMessage returns empty vector if there are no messages in the queue

		dynamic_cast<RtMidiIn *>(rtMidi)->getMessage(&message);
		while (message.size() > 0) {
			processMidi(message);
			dynamic_cast<RtMidiIn *>(rtMidi)->getMessage(&message);
		}
	}

	for (int i = 0; i < NUM_OUTPUTS; i++) {
		outputs[i].value = cc[i] / 127.0 * 10.0;
	}
}

void MIDICCToCVInterface::resetMidi() {
	for (int i = 0; i < NUM_OUTPUTS; i++) {
		cc[i] = 0;
	}
};

void MIDICCToCVInterface::processMidi(std::vector<unsigned char> msg) {
	int channel = msg[0] & 0xf;
	int status = (msg[0] >> 4) & 0xf;
	int data1 = msg[1];
	int data2 = msg[2];

	//fprintf(stderr, "channel %d status %d data1 %d data2 %d\n", channel, status, data1,data2);

	// Filter channels
	if (this->channel >= 0 && this->channel != channel)
		return;

	if (status == 0xb) {
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			if (data1 == ccNum[i]) {
				this->cc[i] = data2;
			}

		}
	}
}

struct CCTextField : TextField {
	void onTextChange();

	void draw(NVGcontext *vg);

	int *ccNum;
	bool *inited;
};

void CCTextField::draw(NVGcontext *vg) {
	/* This is necessary, since the save
	 * file is loaded after constructing the widget*/
	if (*inited) {
		*inited = false;
		text = std::to_string(*ccNum);
	}

	TextField::draw(vg);
}

void CCTextField::onTextChange() {
	if (text.size() > 0) {
		try {
			*ccNum = std::stoi(text);
			// Only allow valid cc numbers
			if (*ccNum < 0 || *ccNum > 127 || text.size() > 3) {
				text = "";
				begin = end = 0;
				*ccNum = -1;
			}
		} catch (...) {
			text = "";
			begin = end = 0;
			*ccNum = -1;
		}
	};
}

MIDICCToCVWidget::MIDICCToCVWidget() {
	MIDICCToCVInterface *module = new MIDICCToCVInterface();
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
		label->text = "MIDI CC to CV";
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

	for (int i = 0; i < MIDICCToCVInterface::NUM_OUTPUTS; i++) {
		CCTextField *ccNumChoice = new CCTextField();
		ccNumChoice->ccNum = &module->ccNum[i];
		ccNumChoice->inited = &module->ccNumInited[i];
		ccNumChoice->text = std::to_string(module->ccNum[i]);
		ccNumChoice->box.pos = Vec(11 + (i % 4) * (63), yPos);
		ccNumChoice->box.size.x = 29;

		addChild(ccNumChoice);

		yPos += labelHeight + margin;
		addOutput(createOutput<PJ3410Port>(Vec((i % 4) * (63) + 10, yPos + 5), module, i));

		if ((i + 1) % 4 == 0) {
			yPos += 47 + margin;
		} else {
			yPos -= labelHeight + margin;
		}
	}
}

void MIDICCToCVWidget::step() {

	ModuleWidget::step();
}
