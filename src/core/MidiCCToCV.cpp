#if 0
#include <list>
#include <algorithm>
#include "core.hpp"
#include "MidiIO.hpp"


struct CCValue {
	int val = 0; // Controller value
	TransitionSmoother tSmooth;
	int num = 0; // Controller number
	bool numInited = false; // Num inited by config file
	bool numSelected = false; // Text field selected for midi learn
	bool changed = false; // Value has been changed by midi message (only if it is in sync!)
	int sync = 0; // Output value sync (implies diff)
	bool syncFirst = true; // First value after sync was reset

	void resetSync() {
		sync = 0;
		syncFirst = true;
	}
};

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

	CCValue cc[NUM_OUTPUTS];

	MIDICCToCVInterface() : MidiIO(), Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			cc[i].num = i;
		}
	}

	~MIDICCToCVInterface() {}

	void step() override;

	void processMidi(std::vector<unsigned char> msg);

	void resetMidi() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();
		addBaseJson(rootJ);
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			json_object_set_new(rootJ, ("ccNum" + std::to_string(i)).c_str(), json_integer(cc[i].num));
			if (outputs[i].active) {
				json_object_set_new(rootJ, ("ccVal" + std::to_string(i)).c_str(), json_integer(cc[i].val));
			}
		}
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		baseFromJson(rootJ);
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			json_t *ccNumJ = json_object_get(rootJ, ("ccNum" + std::to_string(i)).c_str());
			if (ccNumJ) {
				cc[i].num = json_integer_value(ccNumJ);
				cc[i].numInited = true;
			}

			json_t *ccValJ = json_object_get(rootJ, ("ccVal" + std::to_string(i)).c_str());
			if (ccValJ) {
				cc[i].val = json_integer_value(ccValJ);
				cc[i].tSmooth.set((cc[i].val / 127.0 * 10.0), (cc[i].val / 127.0 * 10.0));
				cc[i].resetSync();
			}

		}
	}

	void onReset() override {
		resetMidi();
	}

};

void MIDICCToCVInterface::step() {
	if (isPortOpen()) {
		std::vector<unsigned char> message;


		// midiIn->getMessage returns empty vector if there are no messages in the queue
		getMessage(&message);
		if (message.size() > 0) {
			processMidi(message);
		}
	}


	for (int i = 0; i < NUM_OUTPUTS; i++) {

		lights[i].setBrightness(cc[i].sync / 127.0);

		if (cc[i].changed) {
			cc[i].tSmooth.set(outputs[i].value, (cc[i].val / 127.0 * 10.0), int(engineGetSampleRate() / 32));
			cc[i].changed = false;
		}

		outputs[i].value = cc[i].tSmooth.next();
	}
}

void MIDICCToCVInterface::resetMidi() {
	for (int i = 0; i < NUM_OUTPUTS; i++) {
		cc[i].val = 0;
		cc[i].resetSync();
		cc[i].tSmooth.set(0, 0);
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
			if (cc[i].numSelected) {
				cc[i].resetSync();
				cc[i].num = data1;
			}

			if (data1 == cc[i].num) {
				/* If the first value we received after sync was reset is +/- 1 of
				 * the output value the values are in sync*/
				if (cc[i].syncFirst) {
					cc[i].syncFirst = false;
					if (data2 < cc[i].val + 2 && data2 > cc[i].val - 2) {
						cc[i].sync = 0;
					}
					else {
						cc[i].sync = absi(data2 - cc[i].val);
					}
					return;
				}

				if (cc[i].sync == 0) {
					cc[i].val = data2;
					cc[i].changed = true;
				}
				else {
					cc[i].sync = absi(data2 - cc[i].val);
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

	int outNum;

	MIDICCToCVInterface *module;
};

void CCTextField::draw(NVGcontext *vg) {
	/* This is necessary, since the save
	 * file is loaded after constructing the widget*/
	if (module->cc[outNum].numInited) {
		module->cc[outNum].numInited = false;
		text = std::to_string(module->cc[outNum].num);
	}

	/* If number is selected for midi learn*/
	if (module->cc[outNum].numSelected) {
		text = std::to_string(module->cc[outNum].num);
	}

	TextField::draw(vg);
}

void CCTextField::onMouseUp(EventMouseUp &e) {
	if (e.button == 1) {
		module->cc[outNum].numSelected = false;
		e.consumed = true;
	}
	TextField::onMouseUp(e);
}

void CCTextField::onMouseDown(EventMouseDown &e) {
	if (e.button == 1) {
		module->cc[outNum].numSelected = true;
		e.consumed = true;
	}
	TextField::onMouseDown(e);
}

void CCTextField::onMouseLeave(EventMouseLeave &e) {
	module->cc[outNum].numSelected = false;
	e.consumed = true;
}


void CCTextField::onTextChange() {
	if (text.size() > 0) {
		try {
			int num = std::stoi(text);
			// Only allow valid cc numbers
			if (num < 0 || num > 127 || text.size() > 3) {
				text = "";
				begin = end = 0;
				module->cc[outNum].num = -1;
			}
			else {
				module->cc[outNum].num = num;
				module->cc[outNum].resetSync();
			}

		}
		catch (...) {
			text = "";
			begin = end = 0;
			module->cc[outNum].num = -1;
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
		ccNumChoice->outNum = i;
		ccNumChoice->text = std::to_string(module->cc[i].num);
		ccNumChoice->box.pos = Vec(11 + (i % 4) * (63), yPos);
		ccNumChoice->box.size.x = 29;

		addChild(ccNumChoice);

		yPos += labelHeight + margin;
		addOutput(createOutput<PJ3410Port>(Vec((i % 4) * (63) + 10, yPos + 5), module, i));
		addChild(createLight<SmallLight<RedLight>>(Vec((i % 4) * (63) + 32, yPos + 5), module, i));

		if ((i + 1) % 4 == 0) {
			yPos += 47 + margin;
		}
		else {
			yPos -= labelHeight + margin;
		}
	}
}

void MIDICCToCVWidget::step() {

	ModuleWidget::step();
}
#endif