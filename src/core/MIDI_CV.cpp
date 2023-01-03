#include <algorithm>

#include "plugin.hpp"


namespace rack {
namespace core {


struct MIDI_CV : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		PITCH_OUTPUT,
		GATE_OUTPUT,
		VELOCITY_OUTPUT,
		AFTERTOUCH_OUTPUT,
		PW_OUTPUT,
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

	/** Number of semitones to bend up/down by pitch wheel */
	float pwRange;
	bool smooth;
	int clockDivision;
	int channels;
	enum PolyMode {
		ROTATE_MODE,
		REUSE_MODE,
		RESET_MODE,
		MPE_MODE,
		NUM_POLY_MODES
	};
	PolyMode polyMode;

	uint32_t clock = 0;

	bool pedal;
	// Indexed by channel
	uint8_t notes[16];
	bool gates[16];
	uint8_t velocities[16];
	uint8_t aftertouches[16];
	std::vector<uint8_t> heldNotes;

	int rotateIndex;

	/** Pitch wheel.
	When MPE is disabled, only the first channel is used.
	[channel]
	*/
	uint16_t pws[16];
	/** [channel] */
	uint8_t mods[16];
	dsp::ExponentialFilter pwFilters[16];
	dsp::ExponentialFilter modFilters[16];

	dsp::PulseGenerator clockPulse;
	dsp::PulseGenerator clockDividerPulse;
	dsp::PulseGenerator retriggerPulses[16];
	dsp::PulseGenerator startPulse;
	dsp::PulseGenerator stopPulse;
	dsp::PulseGenerator continuePulse;

	MIDI_CV() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configOutput(PITCH_OUTPUT, "1V/octave pitch");
		configOutput(GATE_OUTPUT, "Gate");
		configOutput(VELOCITY_OUTPUT, "Velocity");
		configOutput(AFTERTOUCH_OUTPUT, "Aftertouch");
		configOutput(PW_OUTPUT, "Pitch wheel");
		configOutput(MOD_OUTPUT, "Mod wheel");
		configOutput(RETRIGGER_OUTPUT, "Retrigger");
		configOutput(CLOCK_OUTPUT, "Clock");
		configOutput(CLOCK_DIV_OUTPUT, "Clock divider");
		configOutput(START_OUTPUT, "Start trigger");
		configOutput(STOP_OUTPUT, "Stop trigger");
		configOutput(CONTINUE_OUTPUT, "Continue trigger");
		heldNotes.reserve(128);
		for (int c = 0; c < 16; c++) {
			pwFilters[c].setTau(1 / 30.f);
			modFilters[c].setTau(1 / 30.f);
		}
		onReset();
	}

	void onReset() override {
		smooth = true;
		channels = 1;
		polyMode = ROTATE_MODE;
		pwRange = 2;
		clockDivision = 24;
		panic();
		midiInput.reset();
	}

	/** Resets performance state */
	void panic() {
		for (int c = 0; c < 16; c++) {
			notes[c] = 60;
			gates[c] = false;
			velocities[c] = 0;
			aftertouches[c] = 0;
			pws[c] = 8192;
			mods[c] = 0;
			pwFilters[c].reset();
			modFilters[c].reset();
		}
		pedal = false;
		rotateIndex = -1;
		heldNotes.clear();
	}

	void process(const ProcessArgs& args) override {
		midi::Message msg;
		while (midiInput.tryPop(&msg, args.frame)) {
			processMessage(msg);
		}

		// Set pitch wheel and mod wheel
		int wheelChannels = (polyMode == MPE_MODE) ? 16 : 1;
		float pwValues[16] = {};
		outputs[PW_OUTPUT].setChannels(wheelChannels);
		outputs[MOD_OUTPUT].setChannels(wheelChannels);
		for (int c = 0; c < wheelChannels; c++) {
			float pw = (int16_t(pws[c]) - 8192) / 8191.f;
			pw = clamp(pw, -1.f, 1.f);
			if (smooth)
				pw = pwFilters[c].process(args.sampleTime, pw);
			else
				pwFilters[c].out = pw;
			pwValues[c] = pw;
			outputs[PW_OUTPUT].setVoltage(pw * 5.f, c);

			float mod = mods[c] / 127.f;
			mod = clamp(mod, 0.f, 1.f);
			if (smooth)
				mod = modFilters[c].process(args.sampleTime, mod);
			else
				modFilters[c].out = mod;
			outputs[MOD_OUTPUT].setVoltage(mod * 10.f, c);
		}

		// Set note outputs
		outputs[PITCH_OUTPUT].setChannels(channels);
		outputs[GATE_OUTPUT].setChannels(channels);
		outputs[VELOCITY_OUTPUT].setChannels(channels);
		outputs[AFTERTOUCH_OUTPUT].setChannels(channels);
		outputs[RETRIGGER_OUTPUT].setChannels(channels);
		for (int c = 0; c < channels; c++) {
			float pw = pwValues[(polyMode == MPE_MODE) ? c : 0];
			float pitch = (notes[c] - 60.f + pw * pwRange) / 12.f;
			outputs[PITCH_OUTPUT].setVoltage(pitch, c);
			outputs[GATE_OUTPUT].setVoltage(gates[c] ? 10.f : 0.f, c);
			outputs[VELOCITY_OUTPUT].setVoltage(rescale(velocities[c], 0, 127, 0.f, 10.f), c);
			outputs[AFTERTOUCH_OUTPUT].setVoltage(rescale(aftertouches[c], 0, 127, 0.f, 10.f), c);
			outputs[RETRIGGER_OUTPUT].setVoltage(retriggerPulses[c].process(args.sampleTime) ? 10.f : 0.f, c);
		}

		// Set clock and transport outputs
		outputs[CLOCK_OUTPUT].setVoltage(clockPulse.process(args.sampleTime) ? 10.f : 0.f);
		outputs[CLOCK_DIV_OUTPUT].setVoltage(clockDividerPulse.process(args.sampleTime) ? 10.f : 0.f);
		outputs[START_OUTPUT].setVoltage(startPulse.process(args.sampleTime) ? 10.f : 0.f);
		outputs[STOP_OUTPUT].setVoltage(stopPulse.process(args.sampleTime) ? 10.f : 0.f);
		outputs[CONTINUE_OUTPUT].setVoltage(continuePulse.process(args.sampleTime) ? 10.f : 0.f);
	}

	void processMessage(const midi::Message& msg) {
		// DEBUG("MIDI: %ld %s", msg.getFrame(), msg.toString().c_str());

		switch (msg.getStatus()) {
			// note off
			case 0x8: {
				releaseNote(msg.getNote());
			} break;
			// note on
			case 0x9: {
				if (msg.getValue() > 0) {
					int c = msg.getChannel();
					pressNote(msg.getNote(), &c);
					velocities[c] = msg.getValue();
				}
				else {
					// For some reason, some keyboards send a "note on" event with a velocity of 0 to signal that the key has been released.
					releaseNote(msg.getNote());
				}
			} break;
			// key pressure
			case 0xa: {
				// Set the aftertouches with the same note
				// TODO Should we handle the MPE case differently?
				for (int c = 0; c < 16; c++) {
					if (notes[c] == msg.getNote())
						aftertouches[c] = msg.getValue();
				}
			} break;
			// cc
			case 0xb: {
				processCC(msg);
			} break;
			// channel pressure
			case 0xd: {
				if (polyMode == MPE_MODE) {
					// Set the channel aftertouch
					aftertouches[msg.getChannel()] = msg.getNote();
				}
				else {
					// Set all aftertouches
					for (int c = 0; c < 16; c++) {
						aftertouches[c] = msg.getNote();
					}
				}
			} break;
			// pitch wheel
			case 0xe: {
				int c = (polyMode == MPE_MODE) ? msg.getChannel() : 0;
				pws[c] = ((uint16_t) msg.getValue() << 7) | msg.getNote();
			} break;
			case 0xf: {
				processSystem(msg);
			} break;
			default: break;
		}
	}

	void processCC(const midi::Message &msg) {
		switch (msg.getNote()) {
			// mod
			case 0x01: {
				int c = (polyMode == MPE_MODE) ? msg.getChannel() : 0;
				mods[c] = msg.getValue();
			} break;
			// sustain
			case 0x40: {
				if (msg.getValue() >= 64)
					pressPedal();
				else
					releasePedal();
			} break;
			// all notes off (panic)
			case 0x7b: {
				if (msg.getValue() == 0) {
					panic();
				}
			} break;
			default: break;
		}
	}

	void processSystem(const midi::Message &msg) {
		switch (msg.getChannel()) {
			// Song Position Pointer
			case 0x2: {
				int32_t pos = int32_t(msg.getNote()) | (int32_t(msg.getValue()) << 7);
				clock = pos * 6;
			} break;
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
			} break;
			default: break;
		}
	}

	int assignChannel(uint8_t note) {
		if (channels == 1)
			return 0;

		switch (polyMode) {
			case REUSE_MODE: {
				// Find channel with the same note
				for (int c = 0; c < channels; c++) {
					if (notes[c] == note)
						return c;
				}
			} // fallthrough

			case ROTATE_MODE: {
				// Find next available channel
				for (int i = 0; i < channels; i++) {
					rotateIndex++;
					if (rotateIndex >= channels)
						rotateIndex = 0;
					if (!gates[rotateIndex])
						return rotateIndex;
				}
				// No notes are available. Advance rotateIndex once more.
				rotateIndex++;
				if (rotateIndex >= channels)
					rotateIndex = 0;
				return rotateIndex;
			} break;

			case RESET_MODE: {
				for (int c = 0; c < channels; c++) {
					if (!gates[c])
						return c;
				}
				return channels - 1;
			} break;

			case MPE_MODE: {
				// This case is handled by querying the MIDI message channel.
				return 0;
			} break;

			default: return 0;
		}
	}

	void pressNote(uint8_t note, int* channel) {
		// Remove existing similar note
		auto it = std::find(heldNotes.begin(), heldNotes.end(), note);
		if (it != heldNotes.end())
			heldNotes.erase(it);
		// Push note
		heldNotes.push_back(note);
		// Determine actual channel
		if (polyMode == MPE_MODE) {
			// Channel is already decided for us
		}
		else {
			*channel = assignChannel(note);
		}
		// Set note
		notes[*channel] = note;
		gates[*channel] = true;
		retriggerPulses[*channel].trigger(1e-3);
	}

	void releaseNote(uint8_t note) {
		// Remove the note
		auto it = std::find(heldNotes.begin(), heldNotes.end(), note);
		if (it != heldNotes.end())
			heldNotes.erase(it);
		// Hold note if pedal is pressed
		if (pedal)
			return;
		// Turn off gate of all channels with note
		for (int c = 0; c < channels; c++) {
			if (notes[c] == note) {
				gates[c] = false;
			}
		}
		// Set last note if monophonic
		if (channels == 1) {
			if (note == notes[0] && !heldNotes.empty()) {
				uint8_t lastNote = heldNotes.back();
				notes[0] = lastNote;
				gates[0] = true;
				return;
			}
		}
	}

	void pressPedal() {
		if (pedal)
			return;
		pedal = true;
	}

	void releasePedal() {
		if (!pedal)
			return;
		pedal = false;
		// Set last note if monophonic
		if (channels == 1) {
			if (!heldNotes.empty()) {
				uint8_t lastNote = heldNotes.back();
				notes[0] = lastNote;
			}
		}
		// Clear notes that are not held if polyphonic
		else {
			for (int c = 0; c < channels; c++) {
				if (!gates[c])
					continue;
				gates[c] = false;
				for (uint8_t note : heldNotes) {
					if (notes[c] == note) {
						gates[c] = true;
						break;
					}
				}
			}
		}
	}

	void setChannels(int channels) {
		if (channels == this->channels)
			return;
		this->channels = channels;
		panic();
	}

	void setPolyMode(PolyMode polyMode) {
		if (polyMode == this->polyMode)
			return;
		this->polyMode = polyMode;
		panic();
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "pwRange", json_real(pwRange));
		json_object_set_new(rootJ, "smooth", json_boolean(smooth));
		json_object_set_new(rootJ, "channels", json_integer(channels));
		json_object_set_new(rootJ, "polyMode", json_integer(polyMode));
		json_object_set_new(rootJ, "clockDivision", json_integer(clockDivision));
		// Saving/restoring pitch and mod doesn't make much sense for MPE.
		if (polyMode != MPE_MODE) {
			json_object_set_new(rootJ, "lastPitch", json_integer(pws[0]));
			json_object_set_new(rootJ, "lastMod", json_integer(mods[0]));
		}
		json_object_set_new(rootJ, "midi", midiInput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* pwRangeJ = json_object_get(rootJ, "pwRange");
		if (pwRangeJ)
			pwRange = json_number_value(pwRangeJ);
		// For backwards compatibility, set to 0 if undefined in JSON.
		else
			pwRange = 0;

		json_t* smoothJ = json_object_get(rootJ, "smooth");
		if (smoothJ)
			smooth = json_boolean_value(smoothJ);

		json_t* channelsJ = json_object_get(rootJ, "channels");
		if (channelsJ)
			setChannels(json_integer_value(channelsJ));

		json_t* polyModeJ = json_object_get(rootJ, "polyMode");
		if (polyModeJ)
			polyMode = (PolyMode) json_integer_value(polyModeJ);

		json_t* clockDivisionJ = json_object_get(rootJ, "clockDivision");
		if (clockDivisionJ)
			clockDivision = json_integer_value(clockDivisionJ);

		json_t* lastPitchJ = json_object_get(rootJ, "lastPitch");
		if (lastPitchJ)
			pws[0] = json_integer_value(lastPitchJ);

		json_t* lastModJ = json_object_get(rootJ, "lastMod");
		if (lastModJ)
			mods[0] = json_integer_value(lastModJ);

		json_t* midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiInput.fromJson(midiJ);
	}
};


struct MIDI_CVWidget : ModuleWidget {
	MIDI_CVWidget(MIDI_CV* module) {
		setModule(module);
		setPanel(Svg::load(asset::system("res/Core/MIDI_CV.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.905, 64.347)), module, MIDI_CV::PITCH_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.248, 64.347)), module, MIDI_CV::GATE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.591, 64.347)), module, MIDI_CV::VELOCITY_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.905, 80.603)), module, MIDI_CV::AFTERTOUCH_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.248, 80.603)), module, MIDI_CV::PW_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.591, 80.603)), module, MIDI_CV::MOD_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.905, 96.859)), module, MIDI_CV::CLOCK_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.248, 96.707)), module, MIDI_CV::CLOCK_DIV_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.591, 96.859)), module, MIDI_CV::RETRIGGER_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.905, 113.115)), module, MIDI_CV::START_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.248, 113.115)), module, MIDI_CV::STOP_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.591, 112.975)), module, MIDI_CV::CONTINUE_OUTPUT));

		MidiDisplay* display = createWidget<MidiDisplay>(mm2px(Vec(0.0, 13.048)));
		display->box.size = mm2px(Vec(40.64, 29.012));
		display->setMidiPort(module ? &module->midiInput : NULL);
		addChild(display);

		// MidiButton example
		// MidiButton* midiButton = createWidget<MidiButton_MIDI_DIN>(Vec(0, 0));
		// midiButton->setMidiPort(module ? &module->midiInput : NULL);
		// addChild(midiButton);
	}

	void appendContextMenu(Menu* menu) override {
		MIDI_CV* module = dynamic_cast<MIDI_CV*>(this->module);

		menu->addChild(new MenuSeparator);

		static const std::vector<float> pwRanges = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 24, 36, 48};
		auto getPwRangeLabel = [](float pwRange) -> std::string {
			if (pwRange == 0)
				return "Off";
			else if (std::abs(pwRange) < 12)
				return string::f("%g semitone", pwRange) + (pwRange == 1 ? "" : "s");
			else
				return string::f("%g octave", pwRange / 12) + (pwRange / 12 == 1 ? "" : "s");
		};
		menu->addChild(createSubmenuItem("Pitch bend range", getPwRangeLabel(module->pwRange), [=](Menu* menu) {
			for (size_t i = 0; i < pwRanges.size(); i++) {
				menu->addChild(createCheckMenuItem(getPwRangeLabel(pwRanges[i]), "",
					[=]() {return module->pwRange == pwRanges[i];},
					[=]() {module->pwRange = pwRanges[i];}
				));
			}
		}));

		menu->addChild(createBoolPtrMenuItem("Smooth pitch/mod wheel", "", &module->smooth));

		static const std::vector<int> clockDivisions = {24 * 4, 24 * 2, 24, 24 / 2, 24 / 4, 24 / 8, 2, 1};
		static const std::vector<std::string> clockDivisionLabels = {"Whole", "Half", "Quarter", "8th", "16th", "32nd", "12 PPQN", "24 PPQN"};
		size_t clockDivisionIndex = std::find(clockDivisions.begin(), clockDivisions.end(), module->clockDivision) - clockDivisions.begin();
		std::string clockDivisionLabel = (clockDivisionIndex < clockDivisionLabels.size()) ? clockDivisionLabels[clockDivisionIndex] : "";
		menu->addChild(createSubmenuItem("CLK/N divider", clockDivisionLabel, [=](Menu* menu) {
			for (size_t i = 0; i < clockDivisions.size(); i++) {
				menu->addChild(createCheckMenuItem(clockDivisionLabels[i], "",
					[=]() {return module->clockDivision == clockDivisions[i];},
					[=]() {module->clockDivision = clockDivisions[i];}
				));
			}
		}));

		menu->addChild(createSubmenuItem("Polyphony channels", string::f("%d", module->channels), [=](Menu* menu) {
			for (int c = 1; c <= 16; c++) {
				menu->addChild(createCheckMenuItem((c == 1) ? "Monophonic" : string::f("%d", c), "",
					[=]() {return module->channels == c;},
					[=]() {module->setChannels(c);}
				));
			}
		}));

		menu->addChild(createIndexPtrSubmenuItem("Polyphony mode", {
			"Rotate",
			"Reuse",
			"Reset",
			"MPE",
		}, &module->polyMode));

		menu->addChild(createMenuItem("Panic", "",
			[=]() {module->panic();}
		));

		// Example of using appendMidiMenu()
		// menu->addChild(new MenuSeparator);
		// appendMidiMenu(menu, &module->midiInput);
	}
};


// Use legacy slug for compatibility
Model* modelMIDI_CV = createModel<MIDI_CV, MIDI_CVWidget>("MIDIToCVInterface");


} // namespace core
} // namespace rack
