#include "Core.hpp"

#include <algorithm>


struct MIDI_CV : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		CV_OUTPUT,
		GATE_OUTPUT,
		VELOCITY_OUTPUT,
		AFTERTOUCH_OUTPUT,
		PITCH_OUTPUT,
		MOD_OUTPUT,
		RETRIGGER_OUTPUT,
		CLOCK_OUTPUT,
		CLOCK_DIV_OUTPUT,
		START_OUTPUT,
		STOP_OUTPUT,
		CONTINUE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	midi::InputQueue midiInput;

	// std::vector<uint8_t> heldNotes;

	int channels;
	enum PolyMode {
		ROTATE_MODE,
		REUSE_MODE,
		RESET_MODE,
		REASSIGN_MODE,
		NUM_POLY_MODES
	};
	PolyMode polyMode;

	uint32_t clock = 0;
	int clockDivision;

	bool pedal = false;
	uint8_t notes[16] = {60};
	bool gates[128] = {};
	uint8_t velocities[128] = {};
	uint8_t aftertouches[128] = {};

	uint16_t pitch = 8192;
	uint8_t mod = 0;

	dsp::ExponentialFilter pitchFilter;
	dsp::ExponentialFilter modFilter;
	dsp::PulseGenerator clockPulse;
	dsp::PulseGenerator clockDividerPulse;
	dsp::PulseGenerator retriggerPulses[16];
	dsp::PulseGenerator startPulse;
	dsp::PulseGenerator stopPulse;
	dsp::PulseGenerator continuePulse;


	MIDI_CV() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		// heldNotes.resize(128, 0);
		pitchFilter.lambda = 1 / 0.01f;
		modFilter.lambda = 1 / 0.01f;
		onReset();
	}

	void onReset() override {
		// heldNotes.clear();
		// lastNote = 60;
		// pedal = false;
		// gate = false;
		// clock = 0;
		channels = 1;
		polyMode = RESET_MODE;
		clockDivision = 24;
		midiInput.reset();
	}

	void step() override {
		midi::Message msg;
		while (midiInput.shift(&msg)) {
			processMessage(msg);
		}
		float deltaTime = APP->engine->getSampleTime();

		outputs[CV_OUTPUT].setChannels(channels);
		outputs[GATE_OUTPUT].setChannels(channels);
		outputs[VELOCITY_OUTPUT].setChannels(channels);
		outputs[AFTERTOUCH_OUTPUT].setChannels(channels);
		outputs[RETRIGGER_OUTPUT].setChannels(channels);
		for (int c = 0; c < channels; c++) {
			uint8_t note = notes[c];
			outputs[CV_OUTPUT].setVoltage((note - 60.f) / 12.f, c);
			outputs[GATE_OUTPUT].setVoltage(gates[note] ? 10.f : 0.f, c);
			outputs[VELOCITY_OUTPUT].setVoltage(rescale(velocities[note], 0, 127, 0.f, 10.f), c);
			outputs[AFTERTOUCH_OUTPUT].setVoltage(rescale(aftertouches[note], 0, 127, 0.f, 10.f), c);
			outputs[RETRIGGER_OUTPUT].setVoltage(retriggerPulses[c].process(deltaTime) ? 10.f : 0.f, c);
		}

		uint16_t pitchAdjusted = (pitch == 16383) ? 16384 : pitch;
		outputs[PITCH_OUTPUT].setVoltage(pitchFilter.process(deltaTime, rescale(pitchAdjusted, 0, 1<<14, -5.f, 5.f)));
		outputs[MOD_OUTPUT].setVoltage(modFilter.process(deltaTime, rescale(mod, 0, 127, 0.f, 10.f)));

		outputs[CLOCK_OUTPUT].setVoltage(clockPulse.process(deltaTime) ? 10.f : 0.f);
		outputs[CLOCK_DIV_OUTPUT].setVoltage(clockDividerPulse.process(deltaTime) ? 10.f : 0.f);
		outputs[START_OUTPUT].setVoltage(startPulse.process(deltaTime) ? 10.f : 0.f);
		outputs[STOP_OUTPUT].setVoltage(stopPulse.process(deltaTime) ? 10.f : 0.f);
		outputs[CONTINUE_OUTPUT].setVoltage(continuePulse.process(deltaTime) ? 10.f : 0.f);
	}

	void processMessage(midi::Message msg) {
		// DEBUG("MIDI: %01x %01x %02x %02x", msg.getStatus(), msg.channel(), msg.getNote(), msg.getValue());

		switch (msg.getStatus()) {
			// note off
			case 0x8: {
				releaseNote(msg.getNote());
			} break;
			// note on
			case 0x9: {
				if (msg.getValue() > 0) {
					velocities[msg.getNote()] = msg.getValue();
					pressNote(msg.getNote());
				}
				else {
					// For some reason, some keyboards send a "note on" event with a velocity of 0 to signal that the key has been released.
					releaseNote(msg.getNote());
				}
			} break;
			// channel aftertouch
			case 0xa: {
				aftertouches[msg.getNote()] = msg.getValue();
			} break;
			// cc
			case 0xb: {
				processCC(msg);
			} break;
			// pitch wheel
			case 0xe: {
				pitch = ((uint16_t) msg.getValue() << 7) | msg.getNote();
			} break;
			case 0xf: {
				processSystem(msg);
			} break;
			default: break;
		}
	}

	void processCC(midi::Message msg) {
		switch (msg.getNote()) {
			// mod
			case 0x01: {
				mod = msg.getValue();
			} break;
			// sustain
			case 0x40: {
				if (msg.getValue() >= 64)
					pressPedal();
				else
					releasePedal();
			} break;
			default: break;
		}
	}

	void processSystem(midi::Message msg) {
		switch (msg.getChannel()) {
			// Timing
			case 0x8: {
				clockPulse.trigger(1e-3);
				if (clock % clockDivision == 0) {
					clockDividerPulse.trigger(1e-3);
				}
				clock++;
			} break;
			// Start
			case 0xa: {
				startPulse.trigger(1e-3);
				clock = 0;
			} break;
			// Continue
			case 0xb: {
				continuePulse.trigger(1e-3);
			} break;
			// Stop
			case 0xc: {
				stopPulse.trigger(1e-3);
				clock = 0;
			} break;
			default: break;
		}
	}

	void pressNote(uint8_t note) {
		int c = 0;
		notes[c] = note;
		gates[note] = true;
		// // Remove existing similar note
		// auto it = std::find(heldNotes.begin(), heldNotes.end(), note);
		// if (it != heldNotes.end())
		// 	heldNotes.erase(it);
		// // Push note
		// heldNotes.push_back(note);
		// lastNote = note;
		// gate = true;
		retriggerPulses[c].trigger(1e-3);
	}

	void releaseNote(uint8_t note) {
		gates[note] = false;
		// // Remove the note
		// auto it = std::find(heldNotes.begin(), heldNotes.end(), note);
		// if (it != heldNotes.end())
		// 	heldNotes.erase(it);
		// // Hold note if pedal is pressed
		// if (pedal)
		// 	return;
		// // Set last note
		// if (!heldNotes.empty()) {
		// 	lastNote = heldNotes[heldNotes.size() - 1];
		// 	gate = true;
		// }
		// else {
		// 	gate = false;
		// }
	}

	void pressPedal() {
		pedal = true;
	}

	void releasePedal() {
		pedal = false;
		// releaseNote(255);
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "channels", json_integer(channels));
		json_object_set_new(rootJ, "polyMode", json_integer(polyMode));
		json_object_set_new(rootJ, "clockDivision", json_integer(clockDivision));
		json_object_set_new(rootJ, "midi", midiInput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *channelsJ = json_object_get(rootJ, "channels");
		if (channelsJ)
			channels = json_integer_value(channelsJ);

		json_t *polyModeJ = json_object_get(rootJ, "polyMode");
		if (polyModeJ)
			polyMode = (PolyMode) json_integer_value(polyModeJ);

		json_t *clockDivisionJ = json_object_get(rootJ, "clockDivision");
		if (clockDivisionJ)
			clockDivision = json_integer_value(clockDivisionJ);

		json_t *midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiInput.fromJson(midiJ);
	}
};


struct ClockDivisionValueItem : MenuItem {
	MIDI_CV *module;
	int clockDivision;
	void onAction(const event::Action &e) override {
		module->clockDivision = clockDivision;
	}
};


struct ClockDivisionItem : MenuItem {
	MIDI_CV *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;
		std::vector<int> divisions = {24*4, 24*2, 24, 24/2, 24/4, 24/8, 2, 1};
		std::vector<std::string> divisionNames = {"Whole", "Half", "Quarter", "8th", "16th", "32nd", "12 PPQN", "24 PPQN"};
		for (size_t i = 0; i < divisions.size(); i++) {
			ClockDivisionValueItem *item = new ClockDivisionValueItem;
			item->text = divisionNames[i];
			item->rightText = CHECKMARK(module->clockDivision == divisions[i]);
			item->module = module;
			item->clockDivision = divisions[i];
			menu->addChild(item);
		}
		return menu;
	}
};


struct ChannelValueItem : MenuItem {
	MIDI_CV *module;
	int channels;
	void onAction(const event::Action &e) override {
		module->channels = channels;
	}
};


struct ChannelItem : MenuItem {
	MIDI_CV *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;
		for (int channels = 1; channels <= 16; channels++) {
			ChannelValueItem *item = new ChannelValueItem;
			if (channels == 1)
				item->text = "Monophonic";
			else
				item->text = string::f("%d", channels);
			item->rightText = CHECKMARK(module->channels == channels);
			item->module = module;
			item->channels = channels;
			menu->addChild(item);
		}
		return menu;
	}
};


struct PolyModeValueItem : MenuItem {
	MIDI_CV *module;
	MIDI_CV::PolyMode polyMode;
	void onAction(const event::Action &e) override {
		module->polyMode = polyMode;
	}
};


struct PolyModeItem : MenuItem {
	MIDI_CV *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;
		std::vector<std::string> polyModeNames = {
			"Rotate",
			"Reuse",
			"Reset",
			"Reassign",
		};
		for (int i = 0; i < MIDI_CV::NUM_POLY_MODES; i++) {
			MIDI_CV::PolyMode polyMode = (MIDI_CV::PolyMode) i;
			PolyModeValueItem *item = new PolyModeValueItem;
			item->text = polyModeNames[i];
			item->rightText = CHECKMARK(module->polyMode == polyMode);
			item->module = module;
			item->polyMode = polyMode;
			menu->addChild(item);
		}
		return menu;
	}
};


struct MIDI_CVWidget : ModuleWidget {
	MIDI_CVWidget(MIDI_CV *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::system("res/Core/MIDI-CV.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(4.61505, 60.1445)), module, MIDI_CV::CV_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(16.214, 60.1445)), module, MIDI_CV::GATE_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.8143, 60.1445)), module, MIDI_CV::VELOCITY_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(4.61505, 76.1449)), module, MIDI_CV::AFTERTOUCH_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(16.214, 76.1449)), module, MIDI_CV::PITCH_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.8143, 76.1449)), module, MIDI_CV::MOD_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(4.61505, 92.1439)), module, MIDI_CV::CLOCK_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(16.214, 92.1439)), module, MIDI_CV::CLOCK_DIV_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.8143, 92.1439)), module, MIDI_CV::RETRIGGER_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(4.61505, 108.144)), module, MIDI_CV::START_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(16.214, 108.144)), module, MIDI_CV::STOP_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.8143, 108.144)), module, MIDI_CV::CONTINUE_OUTPUT));

		MidiWidget *midiWidget = createWidget<MidiWidget>(mm2px(Vec(3.41891, 14.8373)));
		midiWidget->box.size = mm2px(Vec(33.840, 28));
		if (module)
			midiWidget->midiIO = &module->midiInput;
		addChild(midiWidget);
	}

	void appendContextMenu(Menu *menu) override {
		MIDI_CV *module = dynamic_cast<MIDI_CV*>(this->module);

		menu->addChild(new MenuEntry);

		ClockDivisionItem *clockDivisionItem = new ClockDivisionItem;
		clockDivisionItem->text = "CLK/N divider";
		clockDivisionItem->module = module;
		menu->addChild(clockDivisionItem);

		ChannelItem *channelItem = createMenuItem<ChannelItem>("Polyphony channels");
		channelItem->module = module;
		menu->addChild(channelItem);

		PolyModeItem *polyModeItem = createMenuItem<PolyModeItem>("Polyphony mode");
		polyModeItem->module = module;
		menu->addChild(polyModeItem);
	}
};


// Use legacy slug for compatibility
Model *modelMIDI_CV = createModel<MIDI_CV, MIDI_CVWidget>("MIDIToCVInterface");
