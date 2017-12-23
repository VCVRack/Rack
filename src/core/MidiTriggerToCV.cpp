#include <list>
#include <algorithm>
#include "core.hpp"
#include "MidiIO.hpp"
#include "dsp/digital.hpp"


using namespace rack;

struct TriggerValue {
	int val = 0;
	int num;
	bool numInited = false;
	bool onFocus = false;
};

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

	TriggerValue trigger[NUM_OUTPUTS];

	MIDITriggerToCVInterface() : MidiIO(), Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			trigger[i].num = i;
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
			json_object_set_new(rootJ, std::to_string(i).c_str(), json_integer(trigger[i].num));
		}
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		baseFromJson(rootJ);
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			json_t *ccNumJ = json_object_get(rootJ, std::to_string(i).c_str());
			if (ccNumJ) {
				trigger[i].num = json_integer_value(ccNumJ);
				trigger[i].numInited = true;
			}

		}
	}

	void onReset() override {
		resetMidi();
	}
};


void MIDITriggerToCVInterface::step() {
	if (isPortOpen()) {
		std::vector<unsigned char> message;

		// midiIn->getMessage returns empty vector if there are no messages in the queue
		getMessage(&message);
		if (message.size() > 0) {
			processMidi(message);
		}
	}

	for (int i = 0; i < NUM_OUTPUTS; i++) {
		// Note: Could have an option to select between gate and velocity
		// but trigger seams more useful
		// outputs[i].value = trigger[i] / 127.0 * 10;
		outputs[i].value = trigger[i].val > 0 ? 10.0 : 0.0;
	}
}

void MIDITriggerToCVInterface::resetMidi() {
	for (int i = 0; i < NUM_OUTPUTS; i++) {
		trigger[i].val = 0;
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
			if (data1 == trigger[i].num) {
				trigger[i].val = 0;
			}
		}
		return;
	}

	if (status == 0x9) { // note on
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			if (trigger[i].onFocus && data2 > 0) {
				trigger[i].num = data1;
			}

			if (data1 == trigger[i].num) {
				trigger[i].val = data2;
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

	int outNum;
	MIDITriggerToCVInterface *module;
};

void TriggerTextField::draw(NVGcontext *vg) {
	/* This is necessary, since the save
	 * file is loaded after constructing the widget*/
	if (module->trigger[outNum].numInited) {
		module->trigger[outNum].numInited = false;
		text = std::to_string(module->trigger[outNum].num);
	}

	if (module->trigger[outNum].onFocus) {
		text = std::to_string(module->trigger[outNum].num);
	}

	TextField::draw(vg);
}

void TriggerTextField::onTextChange() {
	if (text.size() > 0) {
		try {
			int num = std::stoi(text);
			// Only allow valid cc numbers
			if (num < 0 || num > 127 || text.size() > 3) {
				text = "";
				begin = end = 0;
				module->trigger[outNum].num = -1;
			}
			else {
				module->trigger[outNum].num = num;
			}
		}
		catch (...) {
			text = "";
			begin = end = 0;
			module->trigger[outNum].num = -1;
		}
	};
}

void TriggerTextField::onMouseUp(EventMouseUp &e) {
	if (e.button == 1) {
		module->trigger[outNum].onFocus = false;
		e.consumed = true;
	}
	TextField::onMouseUp(e);
}

void TriggerTextField::onMouseDown(EventMouseDown &e) {
	if (e.button == 1) {
		module->trigger[outNum].onFocus = true;
		e.consumed = true;
	}
	TextField::onMouseDown(e);
}

void TriggerTextField::onMouseLeave(EventMouseLeave &e) {
	module->trigger[outNum].onFocus = false;
	e.consumed = true;
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
		triggerNumChoice->module = module;
		triggerNumChoice->outNum = i;
		triggerNumChoice->text = std::to_string(module->trigger[i].num);
		triggerNumChoice->box.pos = Vec(11 + (i % 4) * (63), yPos);
		triggerNumChoice->box.size.x = 29;

		addChild(triggerNumChoice);

		yPos += labelHeight + margin;
		addOutput(createOutput<PJ3410Port>(Vec((i % 4) * (63) + 10, yPos + 5), module, i));

		if ((i + 1) % 4 == 0) {
			yPos += 47 + margin;
		}
		else {
			yPos -= labelHeight + margin;
		}
	}
}

void MIDITriggerToCVWidget::step() {
	ModuleWidget::step();
}
