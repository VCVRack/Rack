#include "Core.hpp"
#include "midi.hpp"


struct MIDICCToCVInterface : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(CC_OUTPUT, 16),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	midi::InputQueue midiInput;
	int8_t values[128];
	int learningId = -1;
	int ccs[16] = {};
	dsp::ExponentialFilter valueFilters[16];
	int8_t lastValues[16] = {};

	MIDICCToCVInterface() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		onReset();
	}

	void onReset() override {
		for (int i = 0; i < 128; i++) {
			values[i] = 0;
		}
		for (int i = 0; i < 16; i++) {
			ccs[i] = i;
		}
		learningId = -1;
		midiInput.reset();
	}

	void step() override {
		midi::Message msg;
		while (midiInput.shift(&msg)) {
			processMessage(msg);
		}

		float lambda = app()->engine->getSampleTime() * 100.f;
		for (int i = 0; i < 16; i++) {
			if (!outputs[CC_OUTPUT + i].active)
				continue;

			int cc = ccs[i];

			float value = rescale(values[cc], 0, 127, 0.f, 10.f);
			valueFilters[i].lambda = lambda;

			// Detect behavior from MIDI buttons.
			if ((lastValues[i] == 0 && values[cc] == 127) || (lastValues[i] == 127 && values[cc] == 0)) {
				// Jump value
				valueFilters[i].out = value;
			}
			else {
				// Smooth value with filter
				valueFilters[i].process(value);
			}
			lastValues[i] = values[cc];
			outputs[CC_OUTPUT + i].setVoltage(valueFilters[i].out);
		}
	}

	void processMessage(midi::Message msg) {
		switch (msg.getStatus()) {
			// cc
			case 0xb: {
				uint8_t cc = msg.getNote();
				// Learn
				if (learningId >= 0 && values[cc] != msg.data2) {
					ccs[learningId] = cc;
					learningId = -1;
				}
				// Allow CC to be negative if the 8th bit is set.
				// The gamepad driver abuses this, for example.
				values[cc] = msg.data2;
			} break;
			default: break;
		}
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		json_t *ccsJ = json_array();
		for (int i = 0; i < 16; i++) {
			json_array_append_new(ccsJ, json_integer(ccs[i]));
		}
		json_object_set_new(rootJ, "ccs", ccsJ);

		// Remember values so users don't have to touch MIDI controller knobs when restarting Rack
		json_t *valuesJ = json_array();
		for (int i = 0; i < 128; i++) {
			json_array_append_new(valuesJ, json_integer(values[i]));
		}
		json_object_set_new(rootJ, "values", valuesJ);

		json_object_set_new(rootJ, "midi", midiInput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *ccsJ = json_object_get(rootJ, "ccs");
		if (ccsJ) {
			for (int i = 0; i < 16; i++) {
				json_t *ccJ = json_array_get(ccsJ, i);
				if (ccJ)
					ccs[i] = json_integer_value(ccJ);
			}
		}

		json_t *valuesJ = json_object_get(rootJ, "values");
		if (valuesJ) {
			for (int i = 0; i < 128; i++) {
				json_t *valueJ = json_array_get(valuesJ, i);
				if (valueJ) {
					values[i] = json_integer_value(valueJ);
				}
			}
		}

		json_t *midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiInput.fromJson(midiJ);
	}
};


struct MidiCcChoice : GridChoice {
	MIDICCToCVInterface *module;
	int id;
	int focusCc;

	MidiCcChoice() {
		box.size.y = mm2px(6.666);
		textOffset.y -= 4;
	}

	void setId(int id) override {
		this->id = id;
	}

	void step() override {
		if (!module) {
			text = "";
			return;
		}
		if (module->learningId == id) {
			if (0 <= focusCc)
				text = string::f("%d", focusCc);
			else
				text = "LRN";
			color.a = 0.5;
		}
		else {
			text = string::f("%d", module->ccs[id]);
			color.a = 1.0;
			if (app()->event->selectedWidget == this)
				app()->event->selectedWidget = NULL;
		}
	}

	void onSelect(const event::Select &e) override {
		e.consume(this);
		if (!module)
			return;
		module->learningId = id;
		focusCc = -1;
	}

	void onDeselect(const event::Deselect &e) override {
		if (!module)
			return;
		if (0 <= focusCc && focusCc < 128) {
			module->ccs[id] = focusCc;
		}
		module->learningId = -1;
	}

	void onSelectText(const event::SelectText &e) override {
		char c = e.codepoint;
		if ('0' <= c && c <= '9') {
			if (focusCc < 0)
				focusCc = 0;
			focusCc = focusCc * 10 + (c - '0');
		}
		e.consume(this);
	}

	void onSelectKey(const event::SelectKey &e) override {
		if (app()->event->selectedWidget == this) {
			if (e.action == GLFW_PRESS && (e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER)) {
				event::Deselect eDeselect;
				onDeselect(eDeselect);
				app()->event->selectedWidget = NULL;
				e.consume(this);
			}
		}
	}
};


struct MidiCcWidget : Grid16MidiWidget {
	MIDICCToCVInterface *module;
	GridChoice *createGridChoice() override {
		MidiCcChoice *gridChoice = new MidiCcChoice;
		gridChoice->module = module;
		return gridChoice;
	}
};


struct MIDICCToCVInterfaceWidget : ModuleWidget {
	MIDICCToCVInterfaceWidget(MIDICCToCVInterface *module) {
		setModule(module);
		setPanel(SVG::load(asset::system("res/Core/MIDICCToCVInterface.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.894335, 73.344704)), module, MIDICCToCVInterface::CC_OUTPUT + 0));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.494659, 73.344704)), module, MIDICCToCVInterface::CC_OUTPUT + 1));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.094982, 73.344704)), module, MIDICCToCVInterface::CC_OUTPUT + 2));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.693932, 73.344704)), module, MIDICCToCVInterface::CC_OUTPUT + 3));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.8943355, 84.945023)), module, MIDICCToCVInterface::CC_OUTPUT + 4));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.49466, 84.945023)), module, MIDICCToCVInterface::CC_OUTPUT + 5));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.094982, 84.945023)), module, MIDICCToCVInterface::CC_OUTPUT + 6));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.693932, 84.945023)), module, MIDICCToCVInterface::CC_OUTPUT + 7));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.8943343, 96.543976)), module, MIDICCToCVInterface::CC_OUTPUT + 8));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.494659, 96.543976)), module, MIDICCToCVInterface::CC_OUTPUT + 9));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.09498, 96.543976)), module, MIDICCToCVInterface::CC_OUTPUT + 10));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.693932, 96.543976)), module, MIDICCToCVInterface::CC_OUTPUT + 11));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.894335, 108.14429)), module, MIDICCToCVInterface::CC_OUTPUT + 12));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.49466, 108.14429)), module, MIDICCToCVInterface::CC_OUTPUT + 13));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.09498, 108.14429)), module, MIDICCToCVInterface::CC_OUTPUT + 14));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.693932, 108.14429)), module, MIDICCToCVInterface::CC_OUTPUT + 15));

		MidiCcWidget *midiWidget = createWidget<MidiCcWidget>(mm2px(Vec(3.399621, 14.837339)));
		midiWidget->module = module;
		midiWidget->box.size = mm2px(Vec(44, 54.667));
		if (module)
			midiWidget->midiIO = &module->midiInput;
		midiWidget->createGridChoices();
		addChild(midiWidget);
	}
};


Model *modelMIDICCToCVInterface = createModel<MIDICCToCVInterface, MIDICCToCVInterfaceWidget>("MIDICCToCVInterface");
