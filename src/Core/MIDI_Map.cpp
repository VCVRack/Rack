#include "plugin.hpp"


struct MIDI_Map : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	midi::InputQueue midiInput;
	int8_t values[128];
	int learningId;
	int lastLearnedCc;
	int learnedCcs[8];
	dsp::ExponentialFilter valueFilters[8];
	ParamMap paramMaps[8];

	MIDI_Map() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int i = 0; i < 8; i++) {
			valueFilters[i].lambda = 40.f;
		}
		onReset();
	}

	void onReset() override {
		learningId = -1;
		lastLearnedCc = -1;
		for (int i = 0; i < 8; i++) {
			learnedCcs[i] = -1;
		}
		midiInput.reset();
	}

	void step() override {
		midi::Message msg;
		while (midiInput.shift(&msg)) {
			processMessage(msg);
		}

		float deltaTime = APP->engine->getSampleTime();

		for (int i = 0; i < 8; i++) {
			// Get module
			int moduleId = paramMaps[i].moduleId;
			if (moduleId < 0)
				continue;
			Module *module = APP->engine->getModule(moduleId);
			if (!module)
				continue;
			// Get param
			int paramId = paramMaps[i].paramId;
			Param *param = &module->params[paramId];
			if (!param->isBounded())
				continue;
			// Set param
			float v = rescale(values[i], 0, 127, param->minValue, param->maxValue);
			v = valueFilters[i].process(deltaTime, v);
			module->params[paramId].setValue(v);
		}
	}

	void processMessage(midi::Message msg) {
		switch (msg.getStatus()) {
			// cc
			case 0xb: {
				processCC(msg);
			} break;
			default: break;
		}
	}

	void processCC(midi::Message msg) {
		uint8_t cc = msg.getNote();
		// Learn
		if (learningId >= 0 && values[cc] != msg.data2) {
			if (lastLearnedCc != cc) {
				learnedCcs[learningId] = cc;
				lastLearnedCc = cc;
				if (++learningId >= 8)
					learningId = -1;
			}
		}
		values[cc] = msg.getValue();
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		json_t *ccsJ = json_array();
		for (int i = 0; i < 8; i++) {
			json_array_append_new(ccsJ, json_integer(learnedCcs[i]));
		}
		json_object_set_new(rootJ, "ccs", ccsJ);

		json_t *paramMapsJ = json_array();
		for (int i = 0; i < 8; i++) {
			json_array_append_new(paramMapsJ, paramMaps[i].toJson());
		}
		json_object_set_new(rootJ, "paramMaps", paramMapsJ);

		json_object_set_new(rootJ, "midi", midiInput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *ccsJ = json_object_get(rootJ, "ccs");
		if (ccsJ) {
			for (int i = 0; i < 8; i++) {
				json_t *ccJ = json_array_get(ccsJ, i);
				if (ccJ)
					learnedCcs[i] = json_integer_value(ccJ);
			}
		}

		json_t *paramMapsJ = json_object_get(rootJ, "paramMaps");
		if (paramMapsJ) {
			for (int i = 0; i < 8; i++) {
				json_t *paramMapJ = json_array_get(paramMapsJ, i);
				if (paramMapJ)
					paramMaps[i].fromJson(paramMapJ);
			}
		}

		json_t *midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiInput.fromJson(midiJ);
	}
};


struct MIDI_MapChoice : LedDisplayChoice {
	MIDI_Map *module;
	int id;

	void setModule(MIDI_Map *module) {
		this->module = module;
	}

	void onAction(const event::Action &e) override {
		if (!module)
			return;
		module->lastLearnedCc = -1;
	}

	void onSelect(const event::Select &e) override {
		if (!module)
			return;
		module->learningId = id;
		e.consume(this);
	}

	void onDeselect(const event::Deselect &e) override {
		if (!module)
			return;
		if (module->learningId == id) {
			module->learningId = -1;
		}
	}

	void step() override {
		if (!module)
			return;
		if (module->learningId == id) {
			text = "Mapping...";
			color.a = 1.0;
			bgColor = color;
			bgColor.a = 0.15;

			// HACK
			if (APP->event->selectedWidget != this)
				APP->event->setSelected(this);
		}
		else {
			if (module->learnedCcs[id] >= 0) {
				text = string::f("CC%d", module->learnedCcs[id]);
				color.a = 1.0;
				bgColor = nvgRGBA(0, 0, 0, 0);
			}
			else {
				text = "Unmapped";
				color.a = 0.5;
				bgColor = nvgRGBA(0, 0, 0, 0);
			}

			// HACK
			if (APP->event->selectedWidget == this)
				APP->event->setSelected(NULL);
		}
	}
};


struct MIDI_MapDisplay : MidiWidget {
	void setModule(MIDI_Map *module) {
		Vec pos = channelChoice->box.getBottomLeft();
		for (int i = 0; i < 8; i++) {
			LedDisplaySeparator *separator = createWidget<LedDisplaySeparator>(pos);
			separator->box.size.x = box.size.x;
			addChild(separator);

			MIDI_MapChoice *choice = createWidget<MIDI_MapChoice>(pos);
			choice->box.size.x = box.size.x;
			choice->id = i;
			choice->setModule(module);
			addChild(choice);
			pos = choice->box.getBottomLeft();
		}
	}
};


struct MIDI_MapWidget : ModuleWidget {
	MIDI_MapWidget(MIDI_Map *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::system("res/Core/MIDI-Map.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		MIDI_MapDisplay *midiWidget = createWidget<MIDI_MapDisplay>(mm2px(Vec(3.4, 14.839)));
		midiWidget->box.size = mm2px(Vec(43.999, 102.664));
		midiWidget->setMidiIO(module ? &module->midiInput : NULL);
		midiWidget->setModule(module);
		addChild(midiWidget);
	}
};


Model *modelMIDI_Map = createModel<MIDI_Map, MIDI_MapWidget>("MIDI-Map");
