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

	bool smooth;
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
	int clockDivision;

	bool pedal;
	// Indexed by channel
	uint8_t notes[16];
	bool gates[16];
	uint8_t velocities[16];
	uint8_t aftertouches[16];
	std::vector<uint8_t> heldNotes;

	int rotateIndex;

	// 16 channels for MPE. When MPE is disabled, only the first channel is used.
	uint16_t pitches[16];
	uint8_t mods[16];
	dsp::ExponentialFilter pitchFilters[16];
	dsp::ExponentialFilter modFilters[16];

	dsp::PulseGenerator clockPulse;
	dsp::PulseGenerator clockDividerPulse;
	dsp::PulseGenerator retriggerPulses[16];
	dsp::PulseGenerator startPulse;
	dsp::PulseGenerator stopPulse;
	dsp::PulseGenerator continuePulse;

	MIDI_CV() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configOutput(CV_OUTPUT, "V/oct");
		configOutput(GATE_OUTPUT, "Gate");
		configOutput(VELOCITY_OUTPUT, "Velocity");
		configOutput(AFTERTOUCH_OUTPUT, "Aftertouch");
		configOutput(PITCH_OUTPUT, "Pitch wheel");
		configOutput(MOD_OUTPUT, "Mod wheel");
		configOutput(RETRIGGER_OUTPUT, "Retrigger");
		configOutput(CLOCK_OUTPUT, "Clock");
		configOutput(CLOCK_DIV_OUTPUT, "Clock divider");
		configOutput(START_OUTPUT, "Start");
		configOutput(STOP_OUTPUT, "Stop");
		configOutput(CONTINUE_OUTPUT, "Continue");
		heldNotes.reserve(128);
		for (int c = 0; c < 16; c++) {
			pitchFilters[c].setTau(1 / 30.f);
			modFilters[c].setTau(1 / 30.f);
		}
		onReset();
	}

	void onReset() override {
		smooth = true;
		channels = 1;
		polyMode = ROTATE_MODE;
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
			pitches[c] = 8192;
			mods[c] = 0;
			pitchFilters[c].reset();
			modFilters[c].reset();
		}
		pedal = false;
		rotateIndex = -1;
		heldNotes.clear();
	}

	void process(const ProcessArgs& args) override {
		while (!midiInput.queue.empty()) {
			const midi::Message& msg = midiInput.queue.front();
			// Don't process MIDI message until we've reached its frame.
			if (msg.frame > args.frame)
				break;
			processMessage(msg);
			midiInput.queue.pop();
		}

		outputs[CV_OUTPUT].setChannels(channels);
		outputs[GATE_OUTPUT].setChannels(channels);
		outputs[VELOCITY_OUTPUT].setChannels(channels);
		outputs[AFTERTOUCH_OUTPUT].setChannels(channels);
		outputs[RETRIGGER_OUTPUT].setChannels(channels);
		for (int c = 0; c < channels; c++) {
			outputs[CV_OUTPUT].setVoltage((notes[c] - 60.f) / 12.f, c);
			outputs[GATE_OUTPUT].setVoltage(gates[c] ? 10.f : 0.f, c);
			outputs[VELOCITY_OUTPUT].setVoltage(rescale(velocities[c], 0, 127, 0.f, 10.f), c);
			outputs[AFTERTOUCH_OUTPUT].setVoltage(rescale(aftertouches[c], 0, 127, 0.f, 10.f), c);
			outputs[RETRIGGER_OUTPUT].setVoltage(retriggerPulses[c].process(args.sampleTime) ? 10.f : 0.f, c);
		}

		// Set pitch and mod wheel
		auto updatePitch = [&](int c) {
			float pitch = ((int) pitches[c] - 8192) / 8191.f;
			pitch = clamp(pitch, -1.f, 1.f);
			if (smooth)
				pitch = pitchFilters[c].process(args.sampleTime, pitch);
			else
				pitchFilters[c].out = pitch;
			outputs[PITCH_OUTPUT].setVoltage(pitchFilters[c].out * 5.f);
		};
		auto updateMod = [&](int c) {
			float mod = mods[c] / 127.f;
			mod = clamp(mod, 0.f, 1.f);
			if (smooth)
				modFilters[c].process(args.sampleTime, mod);
			else
				modFilters[c].out = mod;
			outputs[MOD_OUTPUT].setVoltage(modFilters[c].out * 10.f);
		};
		if (polyMode == MPE_MODE) {
			for (int c = 0; c < channels; c++) {
				updatePitch(c);
				outputs[PITCH_OUTPUT].setChannels(1);
				updateMod(c);
				outputs[MOD_OUTPUT].setChannels(1);
			}
		}
		else {
			updatePitch(0);
			outputs[PITCH_OUTPUT].setChannels(1);
			updateMod(0);
			outputs[MOD_OUTPUT].setChannels(1);
		}

		outputs[CLOCK_OUTPUT].setVoltage(clockPulse.process(args.sampleTime) ? 10.f : 0.f);
		outputs[CLOCK_DIV_OUTPUT].setVoltage(clockDividerPulse.process(args.sampleTime) ? 10.f : 0.f);
		outputs[START_OUTPUT].setVoltage(startPulse.process(args.sampleTime) ? 10.f : 0.f);
		outputs[STOP_OUTPUT].setVoltage(stopPulse.process(args.sampleTime) ? 10.f : 0.f);
		outputs[CONTINUE_OUTPUT].setVoltage(continuePulse.process(args.sampleTime) ? 10.f : 0.f);
	}

	void processMessage(const midi::Message& msg) {
		// DEBUG("MIDI: %ld %s", msg.frame, msg.toString().c_str());

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
				pitches[c] = ((uint16_t) msg.getValue() << 7) | msg.getNote();
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
		json_object_set_new(rootJ, "smooth", json_boolean(smooth));
		json_object_set_new(rootJ, "channels", json_integer(channels));
		json_object_set_new(rootJ, "polyMode", json_integer(polyMode));
		json_object_set_new(rootJ, "clockDivision", json_integer(clockDivision));
		// Saving/restoring pitch and mod doesn't make much sense for MPE.
		if (polyMode != MPE_MODE) {
			json_object_set_new(rootJ, "lastPitch", json_integer(pitches[0]));
			json_object_set_new(rootJ, "lastMod", json_integer(mods[0]));
		}
		json_object_set_new(rootJ, "midi", midiInput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
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
			pitches[0] = json_integer_value(lastPitchJ);

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

		MidiWidget* midiWidget = createWidget<MidiWidget>(mm2px(Vec(3.41891, 14.8373)));
		midiWidget->box.size = mm2px(Vec(33.840, 28));
		midiWidget->setMidiPort(module ? &module->midiInput : NULL);
		addChild(midiWidget);
	}

	void appendContextMenu(Menu* menu) override {
		MIDI_CV* module = dynamic_cast<MIDI_CV*>(this->module);

		menu->addChild(new MenuSeparator);

		struct SmoothItem : MenuItem {
			MIDI_CV* module;
			void onAction(const ActionEvent& e) override {
				module->smooth ^= true;
			}
		};

		SmoothItem* smoothItem = new SmoothItem;
		smoothItem->text = "Smooth pitch/mod wheel";
		smoothItem->rightText = CHECKMARK(module->smooth);
		smoothItem->module = module;
		menu->addChild(smoothItem);

		struct ClockDivisionValueItem : MenuItem {
			MIDI_CV* module;
			int clockDivision;
			void onAction(const ActionEvent& e) override {
				module->clockDivision = clockDivision;
			}
		};

		struct ClockDivisionItem : MenuItem {
			MIDI_CV* module;
			Menu* createChildMenu() override {
				Menu* menu = new Menu;
				std::vector<int> divisions = {24 * 4, 24 * 2, 24, 24 / 2, 24 / 4, 24 / 8, 2, 1};
				std::vector<std::string> divisionNames = {"Whole", "Half", "Quarter", "8th", "16th", "32nd", "12 PPQN", "24 PPQN"};
				for (size_t i = 0; i < divisions.size(); i++) {
					ClockDivisionValueItem* item = new ClockDivisionValueItem;
					item->text = divisionNames[i];
					item->rightText = CHECKMARK(module->clockDivision == divisions[i]);
					item->module = module;
					item->clockDivision = divisions[i];
					menu->addChild(item);
				}
				return menu;
			}
		};

		ClockDivisionItem* clockDivisionItem = new ClockDivisionItem;
		clockDivisionItem->text = "CLK/N divider";
		clockDivisionItem->rightText = RIGHT_ARROW;
		clockDivisionItem->module = module;
		menu->addChild(clockDivisionItem);

		struct ChannelValueItem : MenuItem {
			MIDI_CV* module;
			int channels;
			void onAction(const ActionEvent& e) override {
				module->setChannels(channels);
			}
		};

		struct ChannelItem : MenuItem {
			MIDI_CV* module;
			Menu* createChildMenu() override {
				Menu* menu = new Menu;
				for (int channels = 1; channels <= 16; channels++) {
					ChannelValueItem* item = new ChannelValueItem;
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

		ChannelItem* channelItem = new ChannelItem;
		channelItem->text = "Polyphony channels";
		channelItem->rightText = string::f("%d", module->channels) + " " + RIGHT_ARROW;
		channelItem->module = module;
		menu->addChild(channelItem);

		struct PolyModeValueItem : MenuItem {
			MIDI_CV* module;
			MIDI_CV::PolyMode polyMode;
			void onAction(const ActionEvent& e) override {
				module->setPolyMode(polyMode);
			}
		};

		struct PolyModeItem : MenuItem {
			MIDI_CV* module;
			Menu* createChildMenu() override {
				Menu* menu = new Menu;
				std::vector<std::string> polyModeNames = {
					"Rotate",
					"Reuse",
					"Reset",
					"MPE",
				};
				for (int i = 0; i < MIDI_CV::NUM_POLY_MODES; i++) {
					MIDI_CV::PolyMode polyMode = (MIDI_CV::PolyMode) i;
					PolyModeValueItem* item = new PolyModeValueItem;
					item->text = polyModeNames[i];
					item->rightText = CHECKMARK(module->polyMode == polyMode);
					item->module = module;
					item->polyMode = polyMode;
					menu->addChild(item);
				}
				return menu;
			}
		};

		PolyModeItem* polyModeItem = new PolyModeItem;
		polyModeItem->text = "Polyphony mode";
		polyModeItem->rightText = RIGHT_ARROW;
		polyModeItem->module = module;
		menu->addChild(polyModeItem);

		struct PanicItem : MenuItem {
			MIDI_CV* module;
			void onAction(const ActionEvent& e) override {
				module->panic();
			}
		};

		PanicItem* panicItem = new PanicItem;
		panicItem->text = "Panic";
		panicItem->module = module;
		menu->addChild(panicItem);

		// Example of using appendMidiMenu()
		// menu->addChild(new MenuSeparator);
		// appendMidiMenu(menu, &module->midiInput);
	}
};


// Use legacy slug for compatibility
Model* modelMIDI_CV = createModel<MIDI_CV, MIDI_CVWidget>("MIDIToCVInterface");


} // namespace core
} // namespace rack
