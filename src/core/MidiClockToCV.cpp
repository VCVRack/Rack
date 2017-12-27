#include <list>
#include <algorithm>
#include "core.hpp"
#include "MidiIO.hpp"
#include "dsp/digital.hpp"


using namespace rack;

struct MIDIClockToCVInterface : MidiIO, Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK1_RATIO,
		CLOCK2_RATIO,
		NUM_INPUTS
	};
	enum OutputIds {
		CLOCK1_PULSE,
		CLOCK2_PULSE,
		CONTINUE_PULSE,
		START_PULSE,
		STOP_PULSE,
		NUM_OUTPUTS
	};

	int clock1ratio = 0;
	int clock2ratio = 0;

	PulseGenerator clock1Pulse;
	PulseGenerator clock2Pulse;
	PulseGenerator continuePulse;
	PulseGenerator startPulse;
	PulseGenerator stopPulse;
	bool tick = false;
	bool running = false;
	bool start = false;
	bool stop = false;
	bool cont = false;
	int c_bar = 0;

	/* Note this is in relation to the Midi clock's Tick (6x per 16th note).
	 * Therefore, e.g. the 2:3 is calculated:
	 *
	 * 24 (Ticks per quarter note) * 2 / 3 = 16
	 *
	 * Implying that every 16 midi clock ticks we need to send a pulse
	 * */
	const int ratios[9] = {6, 8, 12, 16, 24, 32, 48, 96, 192};
	const int numratios = 9;

	/*
	 * Length of clock pulse
	 */
	const float pulseTime = 0.005;


	MIDIClockToCVInterface() : MidiIO(), Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {

	}

	~MIDIClockToCVInterface() {
	}

	void step() override;

	void processMidi(std::vector<unsigned char> msg);

	void onDeviceChange() override;

	void resetMidi() override;

	json_t *toJson()  override {
		json_t *rootJ = json_object();
		addBaseJson(rootJ);
		json_object_set_new(rootJ, "clock1ratio", json_integer(clock1ratio));
		json_object_set_new(rootJ, "clock2ratio", json_integer(clock2ratio));
		return rootJ;
	}

	void fromJson(json_t *rootJ)  override {
		baseFromJson(rootJ);
		json_t *c1rJ = json_object_get(rootJ, "clock1ratio");
		if (c1rJ) {
			clock1ratio = json_integer_value(c1rJ);
		}

		json_t *c2rJ = json_object_get(rootJ, "clock2ratio");
		if (c2rJ) {
			clock2ratio = json_integer_value(c2rJ);
		}
	}
};

void MIDIClockToCVInterface::step() {
	float sampleRate = engineGetSampleRate();

	if (isPortOpen()) {
		std::vector<unsigned char> message;

		// midiIn->getMessage returns empty vector if there are no messages in the queue
		getMessage(&message);
		if (message.size() > 0) {
			processMidi(message);
		}
	}

	if (inputs[CLOCK1_RATIO].active) {
		clock1ratio = int(clampf(inputs[CLOCK1_RATIO].value, 0.0, 10.0) * (numratios - 1) / 10);
	}

	if (inputs[CLOCK2_RATIO].active) {
		clock2ratio = int(clampf(inputs[CLOCK2_RATIO].value, 0.0, 10.0) * (numratios - 1) / 10);
	}

	if (start) {
		start = false;
		running = true;
		startPulse.trigger(pulseTime);
		c_bar = 0;
	}

	if (stop) {
		stop = false;
		running = false;
		stopPulse.trigger(pulseTime);
	}

	if (cont) {
		cont = false;
		running = true;
		continuePulse.trigger(pulseTime);
	}

	if (tick) {
		tick = false;

		/* Note: At least for my midi clock, the clock ticks are sent
		 * even if the midi clock is stopped.
		 * Therefore, we need to keep track of ticks even when the clock
		 * is stopped. Otherwise we can run into weird timing issues.
		*/
		if (running) {
			if (c_bar % ratios[clock1ratio] == 0) {
				clock1Pulse.trigger(pulseTime);
			}

			if (c_bar % ratios[clock2ratio] == 0) {
				clock2Pulse.trigger(pulseTime);
			}
		}

		c_bar++;

		// One "midi bar" = 4 whole notes = (6 ticks per 16th) 6 * 16 *4 = 384
		if (c_bar >= 384) {
			c_bar = 0;
		}
	}

	bool pulse = clock1Pulse.process(1.0 / sampleRate);
	outputs[CLOCK1_PULSE].value = pulse ? 10.0 : 0.0;

	pulse = clock2Pulse.process(1.0 / sampleRate);
	outputs[CLOCK2_PULSE].value = pulse ? 10.0 : 0.0;

	pulse = continuePulse.process(1.0 / sampleRate);
	outputs[CONTINUE_PULSE].value = pulse ? 10.0 : 0.0;

	pulse = startPulse.process(1.0 / sampleRate);
	outputs[START_PULSE].value = pulse ? 10.0 : 0.0;

	pulse = stopPulse.process(1.0 / sampleRate);
	outputs[STOP_PULSE].value = pulse ? 10.0 : 0.0;

}

void MIDIClockToCVInterface::resetMidi() {
	outputs[CLOCK1_PULSE].value = 0.0;
	outputs[CLOCK2_PULSE].value = 0.0;
}

void MIDIClockToCVInterface::processMidi(std::vector<unsigned char> msg) {

	switch (msg[0]) {
	case 0xfa:
		start = true;
		break;
	case 0xfb:
		cont = true;
		break;
	case 0xfc:
		stop = true;
		break;
	case 0xf8:
		tick = true;
		break;
	}


}

void MIDIClockToCVInterface::onDeviceChange() {
	setIgnores(true, false);
}

struct ClockRatioItem : MenuItem {
	int ratio;
	int *clockRatio;

	void onAction(EventAction &e) override {
		*clockRatio = ratio;
	}
};

struct ClockRatioChoice : ChoiceButton {
	int *clockRatio;
	const std::vector<std::string> ratioNames = {"Sixteenth note (1:4 ratio)", "Eighth note triplet (1:3 ratio)",
	                                             "Eighth note (1:2 ratio)", "Quarter note triplet (2:3 ratio)",
	                                             "Quarter note (tap speed)", "Half note triplet (4:3 ratio)",
	                                             "Half note (2:1 ratio)", "Whole note (4:1 ratio)",
	                                             "Two whole notes (8:1 ratio)"
	                                            };

	const std::vector<std::string> ratioNames_short = {"1:4 ratio", "1:3 ratio", "1:2 ratio", "2:3 ratio", "1:1 ratio",
	                                                   "4:3", "2:1 ratio", "4:1 ratio", "8:1 ratio"
	                                                  };

	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
		menu->box.size.x = box.size.x;

		for (unsigned long ratio = 0; ratio < ratioNames.size(); ratio++) {
			ClockRatioItem *clockRatioItem = new ClockRatioItem();
			clockRatioItem->ratio = ratio;
			clockRatioItem->clockRatio = clockRatio;
			clockRatioItem->text = ratioNames[ratio];
			menu->addChild(clockRatioItem);
		}
	}

	void step() override {
		text = ratioNames_short[*clockRatio];
	}
};

MIDIClockToCVWidget::MIDIClockToCVWidget() {
	MIDIClockToCVInterface *module = new MIDIClockToCVInterface();
	setModule(module);
	box.size = Vec(15 * 9, 380);

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
		label->box.pos = Vec(box.size.x - margin - 7 * 15, margin);
		label->text = "MIDI Clk-CV";
		addChild(label);
		yPos = labelHeight * 2;
	}

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
		yPos += midiChoice->box.size.y + margin * 4;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Start";
		addChild(label);
		addOutput(createOutput<PJ3410Port>(Vec(15 * 6, yPos - 5), module, MIDIClockToCVInterface::START_PULSE));
		yPos += labelHeight + margin * 4;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Stop";
		addChild(label);
		addOutput(createOutput<PJ3410Port>(Vec(15 * 6, yPos - 5), module, MIDIClockToCVInterface::STOP_PULSE));
		yPos += labelHeight + margin * 4;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "Continue";
		addChild(label);
		addOutput(createOutput<PJ3410Port>(Vec(15 * 6, yPos - 5), module, MIDIClockToCVInterface::CONTINUE_PULSE));
		yPos += labelHeight + margin * 6;
	}


	{
		addInput(createInput<PJ3410Port>(Vec(margin, yPos - 5), module, MIDIClockToCVInterface::CLOCK1_RATIO));
		ClockRatioChoice *ratioChoice = new ClockRatioChoice();
		ratioChoice->clockRatio = &module->clock1ratio;
		ratioChoice->box.pos = Vec(int(box.size.x / 3), yPos);
		ratioChoice->box.size.x = int(box.size.x / 1.5 - margin);

		addChild(ratioChoice);
		yPos += ratioChoice->box.size.y + margin * 3;

	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "C1 Pulse";
		addChild(label);

		addOutput(createOutput<PJ3410Port>(Vec(15 * 6, yPos - 5), module, MIDIClockToCVInterface::CLOCK1_PULSE));
		yPos += margin * 10;
	}


	{

		addInput(createInput<PJ3410Port>(Vec(margin, yPos - 5), module, MIDIClockToCVInterface::CLOCK2_RATIO));
		ClockRatioChoice *ratioChoice = new ClockRatioChoice();
		ratioChoice->clockRatio = &module->clock2ratio;
		ratioChoice->box.pos = Vec(int(box.size.x / 3), yPos);
		ratioChoice->box.size.x = int(box.size.x / 1.5 - margin);

		addChild(ratioChoice);
		yPos += ratioChoice->box.size.y + margin * 3;
	}

	{
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = "C2 Pulse";
		addChild(label);

		addOutput(createOutput<PJ3410Port>(Vec(15 * 6, yPos - 5), module, MIDIClockToCVInterface::CLOCK2_PULSE));
		yPos += labelHeight + margin * 3;
	}


}

void MIDIClockToCVWidget::step() {

	ModuleWidget::step();
}
