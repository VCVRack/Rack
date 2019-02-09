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
	/** Channel ID of the learning session */
	int learningId;
	/** Whether the CC has been set during the learning session */
	bool learnedCc;
	/** Whether the param has been set during the learning session */
	bool learnedParam;
	/** The learned CC number of each channel */
	int learnedCcs[8];
	/** The learned module handle of each channel */
	ModuleHandle learnedModuleHandles[8];
	/** The learned param ID of each channel */
	int learnedParamIds[8];
	/** The value of each CC number */
	int8_t values[128];
	/** The smoothing processor (normalized between 0 and 1) of each channel */
	dsp::ExponentialFilter valueFilters[8];

	MIDI_Map() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int i = 0; i < 8; i++) {
			valueFilters[i].lambda = 60.f;
		}
		onReset();
	}

	~MIDI_Map() {
		for (int i = 0; i < 8; i++) {
			unloadModuleHandle(i);
		}
	}

	void onReset() override {
		learningId = -1;
		learnedCc = false;
		learnedParam = false;
		for (int i = 0; i < 8; i++) {
			learnedCcs[i] = -1;
			unloadModuleHandle(i);
			learnedModuleHandles[i].id = -1;
			learnedParamIds[i] = 0;
			valueFilters[i].reset();
		}
		for (int i = 0; i < 128; i++) {
			values[i] = -1;
		}
		midiInput.reset();
	}

	void step() override {
		midi::Message msg;
		while (midiInput.shift(&msg)) {
			processMessage(msg);
		}

		float deltaTime = APP->engine->getSampleTime();

		// Check touched params when learning
		if (learningId >= 0) {
			Module *module;
			int paramId;
			APP->engine->getTouchedParam(module, paramId);
			APP->engine->setTouchedParam(NULL, 0);
			if (module) {
				unloadModuleHandle(learningId);
				learnedModuleHandles[learningId].id = module->id;
				loadModuleHandle(learningId);
				learnedParamIds[learningId] = paramId;
				learnedParam = true;
				commitLearn();
			}
		}

		// Step channels
		for (int i = 0; i < 8; i++) {
			int cc = learnedCcs[i];
			if (cc < 0)
				continue;
			// Check if CC value has been set
			if (values[cc] < 0)
				continue;
			// Get module
			Module *module = learnedModuleHandles[i].module;
			if (!module)
				continue;
			// Get param
			int paramId = learnedParamIds[i];
			Param *param = &module->params[paramId];
			if (!param->isBounded())
				continue;
			// Set param
			float v = rescale(values[cc], 0, 127, 0.f, 1.f);
			v = valueFilters[i].process(deltaTime, v);
			v = rescale(v, 0.f, 1.f, param->minValue, param->maxValue);
			APP->engine->setParam(module, paramId, v);
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
		if (learningId >= 0 && values[cc] != msg.getValue()) {
			learnedCcs[learningId] = cc;
			valueFilters[learningId].reset();
			learnedCc = true;
			commitLearn();
		}
		values[cc] = msg.getValue();
	}

	void loadModuleHandle(int i) {
		if (learnedModuleHandles[i].id >= 0) {
			APP->engine->addModuleHandle(&learnedModuleHandles[i]);
		}
	}

	void unloadModuleHandle(int i) {
		if (learnedModuleHandles[i].id >= 0) {
			APP->engine->removeModuleHandle(&learnedModuleHandles[i]);
		}
	}

	void commitLearn() {
		if (learningId < 0)
			return;
		if (!learnedCc)
			return;
		if (!learnedParam)
			return;
		// Reset learned state
		learnedCc = false;
		learnedParam = false;
		// Find next unlearned channel
		while (++learningId < 8) {
			if (learnedCcs[learningId] < 0 || learnedModuleHandles[learningId].id < 0)
				return;
		}
		learningId = -1;
	}

	void clearLearn(int id) {
		disableLearn(id);
		learnedCcs[id] = -1;
		unloadModuleHandle(id);
		learnedModuleHandles[id].id = -1;
		loadModuleHandle(id);
	}

	void enableLearn(int id) {
		if (learningId != id) {
			learningId = id;
			learnedCc = false;
			learnedParam = false;
		}
	}

	void disableLearn(int id) {
		if (learningId == id) {
			learningId = -1;
			learnedCc = false;
			learnedParam = false;
		}
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		json_t *ccsJ = json_array();
		for (int i = 0; i < 8; i++) {
			json_array_append_new(ccsJ, json_integer(learnedCcs[i]));
		}
		json_object_set_new(rootJ, "ccs", ccsJ);

		json_t *moduleIdsJ = json_array();
		for (int i = 0; i < 8; i++) {
			json_array_append_new(moduleIdsJ, json_integer(learnedModuleHandles[i].id));
		}
		json_object_set_new(rootJ, "moduleIds", moduleIdsJ);

		json_t *paramIdsJ = json_array();
		for (int i = 0; i < 8; i++) {
			json_array_append_new(paramIdsJ, json_integer(learnedParamIds[i]));
		}
		json_object_set_new(rootJ, "paramIds", paramIdsJ);

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

		json_t *moduleIdsJ = json_object_get(rootJ, "moduleIds");
		if (moduleIdsJ) {
			for (int i = 0; i < 8; i++) {
				json_t *moduleIdJ = json_array_get(moduleIdsJ, i);
				unloadModuleHandle(i);
				if (moduleIdJ)
					learnedModuleHandles[i].id = json_integer_value(moduleIdJ);
				loadModuleHandle(i);
			}
		}

		json_t *paramIdsJ = json_object_get(rootJ, "paramIds");
		if (paramIdsJ) {
			for (int i = 0; i < 8; i++) {
				json_t *paramIdJ = json_array_get(paramIdsJ, i);
				if (paramIdJ)
					learnedParamIds[i] = json_integer_value(paramIdJ);
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

	void onButton(const event::Button &e) override {
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			APP->engine->setTouchedParam(NULL, 0);
			e.consume(this);
		}

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (module) {
				module->clearLearn(id);
			}
			e.consume(this);
		}
	}

	void onSelect(const event::Select &e) override {
		if (!module)
			return;
		module->enableLearn(id);
		e.consume(this);
	}

	void onDeselect(const event::Deselect &e) override {
		if (!module)
			return;
		module->disableLearn(id);
	}

	void step() override {
		if (!module)
			return;

		// Set bgColor and selected state
		if (module->learningId == id) {
			bgColor = color;
			bgColor.a = 0.15;

			// HACK
			if (APP->event->selectedWidget != this)
				APP->event->setSelected(this);
		}
		else {
			bgColor = nvgRGBA(0, 0, 0, 0);

			// HACK
			// Don't let the event state call onDeselect()
			if (APP->event->selectedWidget == this)
				APP->event->setSelected(NULL);
		}

		// Set text
		text = "";
		if (module->learnedCcs[id] >= 0) {
			text += string::f("CC%d ", module->learnedCcs[id]);
		}
		if (module->learnedModuleHandles[id].id >= 0) {
			text += getParamName();
		}
		if (module->learnedCcs[id] < 0 && module->learnedModuleHandles[id].id < 0) {
			if (module->learningId == id) {
				text = "Mapping...";
			}
			else {
				text = "Unmapped";
			}
		}

		// Set text color
		if ((module->learnedCcs[id] >= 0 && module->learnedModuleHandles[id].id >= 0) || module->learningId == id) {
			color.a = 1.0;
		}
		else {
			color.a = 0.5;
		}
	}

	std::string getParamName() {
		if (!module)
			return "";
		ModuleHandle *moduleHandle = &module->learnedModuleHandles[id];
		if (moduleHandle->id < 0)
			return "";
		ModuleWidget *mw = APP->scene->rackWidget->getModule(moduleHandle->id);
		if (!mw)
			return "";
		// Get the Module from the ModuleWidget instead of the ModuleHandle.
		// I think this is more elegant since this method is called in the app world instead of the engine world.
		Module *m = mw->module;
		if (!m)
			return "";
		int paramId = module->learnedParamIds[id];
		if (paramId >= (int) m->params.size())
			return "";
		Param *param = &m->params[paramId];
		std::string s;
		s += mw->model->name;
		s += " ";
		s += param->label;
		return s;
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
