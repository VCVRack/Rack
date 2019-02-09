#include "plugin.hpp"


static const int CHANNELS = 8;


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
	int learnedCcs[CHANNELS];
	/** The learned param handle of each channel */
	ParamHandle learnedParamHandles[CHANNELS];
	/** The value of each CC number */
	int8_t values[128];
	/** The smoothing processor (normalized between 0 and 1) of each channel */
	dsp::ExponentialFilter valueFilters[CHANNELS];

	MIDI_Map() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int i = 0; i < CHANNELS; i++) {
			APP->engine->addParamHandle(&learnedParamHandles[i]);
			valueFilters[i].lambda = 60.f;
		}
		onReset();
	}

	~MIDI_Map() {
		for (int i = 0; i < CHANNELS; i++) {
			APP->engine->removeParamHandle(&learnedParamHandles[i]);
		}
	}

	void onReset() override {
		learningId = -1;
		learnedCc = false;
		learnedParam = false;
		for (int i = 0; i < CHANNELS; i++) {
			learnedCcs[i] = -1;
			APP->engine->updateParamHandle(&learnedParamHandles[i], -1, 0);
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

		// Step channels
		for (int id = 0; id < CHANNELS; id++) {
			int cc = learnedCcs[id];
			if (cc < 0)
				continue;
			// DEBUG("%d %d %p %d", id, learnedCcs[id], learnedParamHandles[id].module, learnedParamHandles[id].paramId);
			// Check if CC value has been set
			if (values[cc] < 0)
				continue;
			// Get module
			Module *module = learnedParamHandles[id].module;
			if (!module)
				continue;
			// Get param
			int paramId = learnedParamHandles[id].paramId;
			Param *param = &module->params[paramId];
			if (!param->isBounded())
				continue;
			// Set param
			float v = rescale(values[cc], 0, 127, 0.f, 1.f);
			v = valueFilters[id].process(deltaTime, v);
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
		while (++learningId < CHANNELS) {
			if (learnedCcs[learningId] < 0 || learnedParamHandles[learningId].moduleId < 0)
				return;
		}
		learningId = -1;
	}

	void clearLearn(int id) {
		disableLearn(id);
		learnedCcs[id] = -1;
		APP->engine->updateParamHandle(&learnedParamHandles[id], -1, 0);
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
		}
	}

	void learnParam(int id, int moduleId, int paramId) {
		APP->engine->updateParamHandle(&learnedParamHandles[id], moduleId, paramId, true);
		learnedParam = true;
		commitLearn();
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		json_t *ccsJ = json_array();
		for (int i = 0; i < 8; i++) {
			json_array_append_new(ccsJ, json_integer(learnedCcs[i]));
		}
		json_object_set_new(rootJ, "ccs", ccsJ);

		json_t *moduleIdsJ = json_array();
		json_t *paramIdsJ = json_array();
		for (int i = 0; i < CHANNELS; i++) {
			json_array_append_new(moduleIdsJ, json_integer(learnedParamHandles[i].moduleId));
			json_array_append_new(paramIdsJ, json_integer(learnedParamHandles[i].paramId));
		}
		json_object_set_new(rootJ, "moduleIds", moduleIdsJ);
		json_object_set_new(rootJ, "paramIds", paramIdsJ);

		json_object_set_new(rootJ, "midi", midiInput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *ccsJ = json_object_get(rootJ, "ccs");
		if (ccsJ) {
			for (int i = 0; i < CHANNELS; i++) {
				json_t *ccJ = json_array_get(ccsJ, i);
				if (ccJ)
					learnedCcs[i] = json_integer_value(ccJ);
			}
		}

		json_t *moduleIdsJ = json_object_get(rootJ, "moduleIds");
		json_t *paramIdsJ = json_object_get(rootJ, "paramIds");
		if (moduleIdsJ && paramIdsJ) {
			for (int i = 0; i < CHANNELS; i++) {
				json_t *moduleIdJ = json_array_get(moduleIdsJ, i);
				json_t *paramIdJ = json_array_get(paramIdsJ, i);
				if (moduleIdJ && paramIdsJ)
					APP->engine->updateParamHandle(&learnedParamHandles[i], json_integer_value(moduleIdJ), json_integer_value(paramIdJ), false);
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
	int disableLearnFrames = -1;

	void setModule(MIDI_Map *module) {
		this->module = module;
	}

	void onButton(const event::Button &e) override {
		if (!module)
			return;

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			e.consume(this);
		}

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
			module->clearLearn(id);
			e.consume(this);
		}
	}

	void onSelect(const event::Select &e) override {
		if (!module)
			return;
		// Reset touchedParam
		APP->scene->rackWidget->touchedParam = NULL;
		module->enableLearn(id);
		e.consume(this);
	}

	void onDeselect(const event::Deselect &e) override {
		if (!module)
			return;
		// Check if a ParamWidget was touched
		ParamWidget *touchedParam = APP->scene->rackWidget->touchedParam;
		if (touchedParam) {
			APP->scene->rackWidget->touchedParam = NULL;
			int moduleId = touchedParam->paramQuantity->module->id;
			int paramId = touchedParam->paramQuantity->paramId;
			module->learnParam(id, moduleId, paramId);
		}
		else {
			module->disableLearn(id);
		}
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
			if (APP->event->selectedWidget == this)
				APP->event->setSelected(NULL);
		}

		// Set text
		text = "";
		if (module->learnedCcs[id] >= 0) {
			text += string::f("CC%d ", module->learnedCcs[id]);
		}
		if (module->learnedParamHandles[id].moduleId >= 0) {
			text += getParamName();
		}
		if (module->learnedCcs[id] < 0 && module->learnedParamHandles[id].moduleId < 0) {
			if (module->learningId == id) {
				text = "Mapping...";
			}
			else {
				text = "Unmapped";
			}
		}

		// Set text color
		if ((module->learnedCcs[id] >= 0 && module->learnedParamHandles[id].moduleId >= 0) || module->learningId == id) {
			color.a = 1.0;
		}
		else {
			color.a = 0.5;
		}
	}

	std::string getParamName() {
		if (!module)
			return "";
		ParamHandle *paramHandle = &module->learnedParamHandles[id];
		if (paramHandle->moduleId < 0)
			return "";
		ModuleWidget *mw = APP->scene->rackWidget->getModule(paramHandle->moduleId);
		if (!mw)
			return "";
		// Get the Module from the ModuleWidget instead of the ParamHandle.
		// I think this is more elegant since this method is called in the app world instead of the engine world.
		Module *m = mw->module;
		if (!m)
			return "";
		int paramId = paramHandle->paramId;
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
		// ScrollWidget *scroll = new ScrollWidget;
		// scroll->box.pos = channelChoice->box.getBottomLeft();
		// scroll->box.size.x = box.size.x;
		// scroll->box.size.y = box.size.y - scroll->box.pos.y;
		// addChild(scroll);

		Vec pos = channelChoice->box.getBottomLeft();

		for (int i = 0; i < CHANNELS; i++) {
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
