#include "Core.hpp"
#include "midi.hpp"

#include <algorithm>


struct MIDIToCVInterface : Module {
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
		CLOCK_1_OUTPUT,
		CLOCK_2_OUTPUT,
		START_OUTPUT,
		STOP_OUTPUT,
		CONTINUE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	midi::InputQueue midiInput;

	uint8_t mod = 0;
	dsp::ExponentialFilter modFilter;
	uint16_t pitch = 8192;
	dsp::ExponentialFilter pitchFilter;
	dsp::PulseGenerator retriggerPulse;
	dsp::PulseGenerator clockPulses[2];
	dsp::PulseGenerator startPulse;
	dsp::PulseGenerator stopPulse;
	dsp::PulseGenerator continuePulse;
	int clock = 0;
	int divisions[2];

	struct NoteData {
		uint8_t velocity = 0;
		uint8_t aftertouch = 0;
	};

	NoteData noteData[128];
	std::vector<uint8_t> heldNotes;
	uint8_t lastNote;
	bool pedal;
	bool gate;

	MIDIToCVInterface() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		heldNotes.resize(128, 0);
		onReset();
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		json_t *divisionsJ = json_array();
		for (int i = 0; i < 2; i++) {
			json_t *divisionJ = json_integer(divisions[i]);
			json_array_append_new(divisionsJ, divisionJ);
		}
		json_object_set_new(rootJ, "divisions", divisionsJ);

		json_object_set_new(rootJ, "midi", midiInput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *divisionsJ = json_object_get(rootJ, "divisions");
		if (divisionsJ) {
			for (int i = 0; i < 2; i++) {
				json_t *divisionJ = json_array_get(divisionsJ, i);
				if (divisionJ)
					divisions[i] = json_integer_value(divisionJ);
			}
		}

		json_t *midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiInput.fromJson(midiJ);
	}

	void onReset() override {
		heldNotes.clear();
		lastNote = 60;
		pedal = false;
		gate = false;
		clock = 0;
		divisions[0] = 24;
		divisions[1] = 6;
	}

	void pressNote(uint8_t note) {
		// Remove existing similar note
		auto it = std::find(heldNotes.begin(), heldNotes.end(), note);
		if (it != heldNotes.end())
			heldNotes.erase(it);
		// Push note
		heldNotes.push_back(note);
		lastNote = note;
		gate = true;
		retriggerPulse.trigger(1e-3);
	}

	void releaseNote(uint8_t note) {
		// Remove the note
		auto it = std::find(heldNotes.begin(), heldNotes.end(), note);
		if (it != heldNotes.end())
			heldNotes.erase(it);
		// Hold note if pedal is pressed
		if (pedal)
			return;
		// Set last note
		if (!heldNotes.empty()) {
			lastNote = heldNotes[heldNotes.size() - 1];
			gate = true;
		}
		else {
			gate = false;
		}
	}

	void pressPedal() {
		pedal = true;
	}

	void releasePedal() {
		pedal = false;
		releaseNote(255);
	}

	void step() override {
		midi::Message msg;
		while (midiInput.shift(&msg)) {
			processMessage(msg);
		}
		float deltaTime = app()->engine->getSampleTime();

		outputs[CV_OUTPUT].setVoltage((lastNote - 60) / 12.f);
		outputs[GATE_OUTPUT].setVoltage(gate ? 10.f : 0.f);
		outputs[VELOCITY_OUTPUT].setVoltage(rescale(noteData[lastNote].velocity, 0, 127, 0.f, 10.f));

		outputs[AFTERTOUCH_OUTPUT].setVoltage(rescale(noteData[lastNote].aftertouch, 0, 127, 0.f, 10.f));
		pitchFilter.lambda = 100.f * deltaTime;
		outputs[PITCH_OUTPUT].setVoltage(pitchFilter.process(rescale(pitch, 0, 16384, -5.f, 5.f)));
		modFilter.lambda = 100.f * deltaTime;
		outputs[MOD_OUTPUT].setVoltage(modFilter.process(rescale(mod, 0, 127, 0.f, 10.f)));

		outputs[RETRIGGER_OUTPUT].setVoltage(retriggerPulse.process(deltaTime) ? 10.f : 0.f);
		outputs[CLOCK_1_OUTPUT].setVoltage(clockPulses[0].process(deltaTime) ? 10.f : 0.f);
		outputs[CLOCK_2_OUTPUT].setVoltage(clockPulses[1].process(deltaTime) ? 10.f : 0.f);

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
					noteData[msg.getNote()].velocity = msg.getValue();
					pressNote(msg.getNote());
				}
				else {
					// For some reason, some keyboards send a "note on" event with a velocity of 0 to signal that the key has been released.
					releaseNote(msg.getNote());
				}
			} break;
			// channel aftertouch
			case 0xa: {
				uint8_t note = msg.getNote();
				noteData[note].aftertouch = msg.getValue();
			} break;
			// cc
			case 0xb: {
				processCC(msg);
			} break;
			// pitch wheel
			case 0xe: {
				pitch = msg.getValue() * 128 + msg.getNote();
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
				if (clock % divisions[0] == 0) {
					clockPulses[0].trigger(1e-3);
				}
				if (clock % divisions[1] == 0) {
					clockPulses[1].trigger(1e-3);
				}
				if (++clock >= (24*16*16)) {
					// Avoid overflowing the integer
					clock = 0;
				}
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
				// Reset timing
				clock = 0;
			} break;
			default: break;
		}
	}
};


struct MIDIToCVInterfaceWidget : ModuleWidget {
	MIDIToCVInterfaceWidget(MIDIToCVInterface *module) : ModuleWidget(module) {
		setPanel(SVG::load(asset::system("res/Core/MIDIToCVInterface.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(4.61505, 60.1445)), module, MIDIToCVInterface::CV_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(16.214, 60.1445)), module, MIDIToCVInterface::GATE_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.8143, 60.1445)), module, MIDIToCVInterface::VELOCITY_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(4.61505, 76.1449)), module, MIDIToCVInterface::AFTERTOUCH_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(16.214, 76.1449)), module, MIDIToCVInterface::PITCH_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.8143, 76.1449)), module, MIDIToCVInterface::MOD_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(4.61505, 92.1439)), module, MIDIToCVInterface::RETRIGGER_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(16.214, 92.1439)), module, MIDIToCVInterface::CLOCK_1_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.8143, 92.1439)), module, MIDIToCVInterface::CLOCK_2_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(4.61505, 108.144)), module, MIDIToCVInterface::START_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(16.214, 108.144)), module, MIDIToCVInterface::STOP_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.8143, 108.144)), module, MIDIToCVInterface::CONTINUE_OUTPUT));

		MidiWidget *midiWidget = createWidget<MidiWidget>(mm2px(Vec(3.41891, 14.8373)));
		midiWidget->box.size = mm2px(Vec(33.840, 28));
		if (module)
			midiWidget->midiIO = &module->midiInput;
		addChild(midiWidget);
	}

	void appendContextMenu(Menu *menu) override {
		MIDIToCVInterface *module = dynamic_cast<MIDIToCVInterface*>(this->module);

		struct ClockDivisionItem : MenuItem {
			MIDIToCVInterface *module;
			int index;
			int division;
			void onAction(const event::Action &e) override {
				module->divisions[index] = division;
			}
		};

		struct ClockItem : MenuItem {
			MIDIToCVInterface *module;
			int index;
			Menu *createChildMenu() override {
				Menu *menu = new Menu;
				std::vector<int> divisions = {24*4, 24*2, 24, 24/2, 24/4, 24/8, 2, 1};
				std::vector<std::string> divisionNames = {"Whole", "Half", "Quarter", "8th", "16th", "32nd", "12 PPQN", "24 PPQN"};
				for (size_t i = 0; i < divisions.size(); i++) {
					ClockDivisionItem *item = createMenuItem<ClockDivisionItem>(divisionNames[i], CHECKMARK(module->divisions[index] == divisions[i]));
					item->module = module;
					item->index = index;
					item->division = divisions[i];
					menu->addChild(item);
				}
				return menu;
			}
		};

		menu->addChild(construct<MenuLabel>());
		for (int i = 0; i < 2; i++) {
			ClockItem *item = createMenuItem<ClockItem>(string::f("CLK %d rate", i + 1));
			item->module = module;
			item->index = i;
			menu->addChild(item);
		}
	}
};


Model *modelMIDIToCVInterface = createModel<MIDIToCVInterface, MIDIToCVInterfaceWidget>("MIDIToCVInterface");
