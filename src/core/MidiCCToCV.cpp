#include <list>
#include <algorithm>
#include "rtmidi/RtMidi.h"
#include "core.hpp"
#include "MidiIO.hpp"

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
	enum LightIds {
		NUM_LIGHTS = 16
	};

	int cc[NUM_OUTPUTS];
	int ccNum[NUM_OUTPUTS];
	int ccSync[NUM_OUTPUTS];
	bool ccSyncFirst[NUM_OUTPUTS];
	bool ccNumInited[NUM_OUTPUTS];
	bool onFocus[NUM_OUTPUTS];


	MIDICCToCVInterface() : MidiIO(), Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			cc[i] = 0;
			ccNum[i] = i;
			ccSync[i] = 0;
			ccSyncFirst[i] = true;
			onFocus[i] = false;
		}
	}

	~MIDICCToCVInterface() {

	}

	void step() override;

	void processMidi(std::vector<unsigned char> msg);

	void resetMidi() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();
		addBaseJson(rootJ);
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			json_object_set_new(rootJ, ("ccNum" + std::to_string(i)).c_str(), json_integer(ccNum[i]));
			if (outputs[i].active) {
				json_object_set_new(rootJ, ("ccVal" + std::to_string(i)).c_str(), json_integer(cc[i]));
			}
		}
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		baseFromJson(rootJ);
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			json_t *ccNumJ = json_object_get(rootJ, ("ccNum" + std::to_string(i)).c_str());
			if (ccNumJ) {
				ccNum[i] = json_integer_value(ccNumJ);
				ccNumInited[i] = true;
			}

			json_t *ccValJ = json_object_get(rootJ, ("ccVal" + std::to_string(i)).c_str());
			if (ccValJ) {
				cc[i] = json_integer_value(ccValJ);
			}

		}
	}

	void reset() override {
		resetMidi();
	}

};

void MIDICCToCVInterface::step() {
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

		lights[i].setBrightness(ccSync[i] / 127.0);

		outputs[i].value = cc[i] / 127.0 * 10.0;
	}
}

void MIDICCToCVInterface::resetMidi() {
	for (int i = 0; i < NUM_OUTPUTS; i++) {
		cc[i] = 0;
		ccSync[i] = 0;
		ccSyncFirst[i] = true;
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
			if (onFocus[i]) {
				ccSync[i] = true;
				ccSyncFirst[i] = true;
				ccNum[i] = data1;
			}

			if (data1 == ccNum[i]) {
				if (ccSyncFirst[i]) {
					ccSyncFirst[i] = false;

					if (data2 < cc[i] + 2 && data2 > cc[i] - 2) {
						ccSync[i] = 0;
					} else {
						ccSync[i] = absi(data2 - cc[i]);
					}
				}

				if (ccSync[i] == 0) {
					cc[i] = data2;
				} else {
					ccSync[i] = absi(data2 - cc[i]);
				}
			}
		}
	}
}

struct CCTextField : TextField {
	void onTextChange() override;

	void draw(NVGcontext *vg) override;

	void onMouseDown(EventMouseDown &e) override;

	void onMouseUp(EventMouseUp &e) override;

	void onMouseLeave(EventMouseLeave &e) override;

	int num;

	MIDICCToCVInterface *module;
};

void CCTextField::draw(NVGcontext *vg) {
	/* This is necessary, since the save
	 * file is loaded after constructing the widget*/
	if (module->ccNumInited[num]) {
		module->ccNumInited[num] = false;
		text = std::to_string(module->ccNum[num]);
	}

	if (module->onFocus[num]) {
		text = std::to_string(module->ccNum[num]);
	}

	TextField::draw(vg);
}

void CCTextField::onMouseDown(EventMouseDown &e) {
	if (e.button == 1) {
		module->onFocus[num] = true;
	}
}

void CCTextField::onMouseUp(EventMouseUp &e) {
	if (e.button == 1) {
		module->onFocus[num] = false;
	}

}

void CCTextField::onMouseLeave(EventMouseLeave &e) {
	module->onFocus[num] = false;
}


void CCTextField::onTextChange() {
	int *ccNum = &module->ccNum[num];
	if (text.size() > 0) {
		try {
			*ccNum = std::stoi(text);
			// Only allow valid cc numbers
			if (*ccNum < 0 || *ccNum > 127 || text.size() > 3) {
				text = "";
				begin = end = 0;
				*ccNum = -1;
				return;
			}

			if (!module->ccNumInited[num] && *ccNum != std::stoi(text)) {
				module->ccSync[num] = 0;
				module->ccSyncFirst[num] = true;
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
		ccNumChoice->module = module;
		ccNumChoice->num = i;
		ccNumChoice->text = std::to_string(module->ccNum[i]);
		ccNumChoice->box.pos = Vec(11 + (i % 4) * (63), yPos);
		ccNumChoice->box.size.x = 29;

		addChild(ccNumChoice);

		yPos += labelHeight + margin;
		addOutput(createOutput<PJ3410Port>(Vec((i % 4) * (63) + 10, yPos + 5), module, i));
		addChild(createLight<SmallLight<RedLight>>(Vec((i % 4) * (63) + 32, yPos + 5), module, i));

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
