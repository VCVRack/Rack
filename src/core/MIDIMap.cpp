#include "plugin.hpp"


namespace rack {
namespace core {


static const int MAX_CHANNELS = 128;


struct MIDIMap : Module {
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

	bool smooth;
	/** Number of maps */
	int mapLen = 0;
	/** The mapped CC number of each channel */
	int ccs[MAX_CHANNELS];
	/** The mapped param handle of each channel */
	ParamHandle paramHandles[MAX_CHANNELS];

	/** Channel ID of the learning session */
	int learningId;
	/** Whether the CC has been set during the learning session */
	bool learnedCc;
	/** Whether the param has been set during the learning session */
	bool learnedParam;

	/** The value of each CC number */
	int8_t values[128];
	/** The smoothing processor (normalized between 0 and 1) of each channel */
	dsp::ExponentialFilter valueFilters[MAX_CHANNELS];
	bool filterInitialized[MAX_CHANNELS] = {};
	dsp::ClockDivider divider;

	MIDIMap() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int id = 0; id < MAX_CHANNELS; id++) {
			paramHandles[id].color = nvgRGB(0xff, 0xff, 0x40);
			APP->engine->addParamHandle(&paramHandles[id]);
		}
		for (int i = 0; i < MAX_CHANNELS; i++) {
			valueFilters[i].setTau(1 / 30.f);
		}
		divider.setDivision(32);
		onReset();
	}

	~MIDIMap() {
		for (int id = 0; id < MAX_CHANNELS; id++) {
			APP->engine->removeParamHandle(&paramHandles[id]);
		}
	}

	void onReset() override {
		smooth = true;
		learningId = -1;
		learnedCc = false;
		learnedParam = false;
		// Use NoLock because we're already in an Engine write-lock if Engine::resetModule().
		// We also might be in the MIDIMap() constructor, which could cause problems, but when constructing, all ParamHandles will point to no Modules anyway.
		clearMaps_NoLock();
		mapLen = 1;
		for (int i = 0; i < 128; i++) {
			values[i] = -1;
		}
		midiInput.reset();
	}

	void process(const ProcessArgs& args) override {
		if (!divider.process())
			return;

		midi::Message msg;
		while (midiInput.tryPop(&msg, args.frame)) {
			processMessage(msg);
		}

		// Step channels
		for (int id = 0; id < mapLen; id++) {
			int cc = ccs[id];
			if (cc < 0)
				continue;
			// Get Module
			Module* module = paramHandles[id].module;
			if (!module)
				continue;
			// Get ParamQuantity from ParamHandle
			int paramId = paramHandles[id].paramId;
			ParamQuantity* paramQuantity = module->paramQuantities[paramId];
			if (!paramQuantity)
				continue;
			if (!paramQuantity->isBounded())
				continue;
			// Set filter from param value if filter is uninitialized
			if (!filterInitialized[id]) {
				valueFilters[id].out = paramQuantity->getScaledValue();
				filterInitialized[id] = true;
				continue;
			}
			// Check if CC has been set by the MIDI device
			if (values[cc] < 0)
				continue;
			float value = values[cc] / 127.f;
			// Detect behavior from MIDI buttons.
			if (smooth && std::fabs(valueFilters[id].out - value) < 1.f) {
				// Smooth value with filter
				valueFilters[id].process(args.sampleTime * divider.getDivision(), value);
			}
			else {
				// Jump value
				valueFilters[id].out = value;
			}
			paramQuantity->setScaledValue(valueFilters[id].out);
		}
	}

	void processMessage(const midi::Message& msg) {
		// DEBUG("MIDI: %01x %01x %02x %02x", msg.getStatus(), msg.getChannel(), msg.getNote(), msg.getValue());

		switch (msg.getStatus()) {
			// cc
			case 0xb: {
				processCC(msg);
			} break;
			default: break;
		}
	}

	void processCC(const midi::Message& msg) {
		uint8_t cc = msg.getNote();
		int8_t value = msg.getValue();
		// Learn
		if (0 <= learningId && values[cc] != value) {
			ccs[learningId] = cc;
			valueFilters[learningId].reset();
			learnedCc = true;
			commitLearn();
			updateMapLen();
			refreshParamHandleText(learningId);
		}
		// Ignore negative values generated using the nonstandard 8-bit MIDI extension from the gamepad driver
		if (value < 0)
			return;
		values[cc] = value;
	}

	void clearMap(int id) {
		learningId = -1;
		ccs[id] = -1;
		APP->engine->updateParamHandle(&paramHandles[id], -1, 0, true);
		valueFilters[id].reset();
		updateMapLen();
		refreshParamHandleText(id);
	}

	void clearMaps_NoLock() {
		learningId = -1;
		for (int id = 0; id < MAX_CHANNELS; id++) {
			ccs[id] = -1;
			APP->engine->updateParamHandle_NoLock(&paramHandles[id], -1, 0, true);
			valueFilters[id].reset();
			refreshParamHandleText(id);
		}
		mapLen = 0;
	}

	void updateMapLen() {
		// Find last nonempty map
		int id;
		for (id = MAX_CHANNELS - 1; id >= 0; id--) {
			if (ccs[id] >= 0 || paramHandles[id].moduleId >= 0)
				break;
		}
		mapLen = id + 1;
		// Add an empty "Mapping..." slot
		if (mapLen < MAX_CHANNELS)
			mapLen++;
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
		// Find next incomplete map
		while (++learningId < MAX_CHANNELS) {
			if (ccs[learningId] < 0 || paramHandles[learningId].moduleId < 0)
				return;
		}
		learningId = -1;
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

	void learnParam(int id, int64_t moduleId, int paramId) {
		APP->engine->updateParamHandle(&paramHandles[id], moduleId, paramId, true);
		learnedParam = true;
		commitLearn();
		updateMapLen();
	}

	void refreshParamHandleText(int id) {
		std::string text;
		if (ccs[id] >= 0)
			text = string::f("CC%02d", ccs[id]);
		else
			text = "MIDI-Map";
		paramHandles[id].text = text;
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		json_t* mapsJ = json_array();
		for (int id = 0; id < mapLen; id++) {
			json_t* mapJ = json_object();
			json_object_set_new(mapJ, "cc", json_integer(ccs[id]));
			json_object_set_new(mapJ, "moduleId", json_integer(paramHandles[id].moduleId));
			json_object_set_new(mapJ, "paramId", json_integer(paramHandles[id].paramId));
			json_array_append_new(mapsJ, mapJ);
		}
		json_object_set_new(rootJ, "maps", mapsJ);

		json_object_set_new(rootJ, "smooth", json_boolean(smooth));
		json_object_set_new(rootJ, "midi", midiInput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		// Use NoLock because we're already in an Engine write-lock.
		clearMaps_NoLock();

		json_t* mapsJ = json_object_get(rootJ, "maps");
		if (mapsJ) {
			json_t* mapJ;
			size_t mapIndex;
			json_array_foreach(mapsJ, mapIndex, mapJ) {
				json_t* ccJ = json_object_get(mapJ, "cc");
				json_t* moduleIdJ = json_object_get(mapJ, "moduleId");
				json_t* paramIdJ = json_object_get(mapJ, "paramId");
				if (!(ccJ && moduleIdJ && paramIdJ))
					continue;
				if (mapIndex >= MAX_CHANNELS)
					continue;
				ccs[mapIndex] = json_integer_value(ccJ);
				APP->engine->updateParamHandle_NoLock(&paramHandles[mapIndex], json_integer_value(moduleIdJ), json_integer_value(paramIdJ), false);
				refreshParamHandleText(mapIndex);
			}
		}

		updateMapLen();

		json_t* smoothJ = json_object_get(rootJ, "smooth");
		if (smoothJ)
			smooth = json_boolean_value(smoothJ);

		json_t* midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiInput.fromJson(midiJ);
	}
};


struct MIDIMapChoice : LedDisplayChoice {
	MIDIMap* module;
	int id;
	int disableLearnFrames = -1;

	void setModule(MIDIMap* module) {
		this->module = module;
	}

	void onButton(const ButtonEvent& e) override {
		e.stopPropagating();
		if (!module)
			return;

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			e.consume(this);
		}

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
			module->clearMap(id);
			e.consume(this);
		}
	}

	void onSelect(const SelectEvent& e) override {
		if (!module)
			return;

		ScrollWidget* scroll = getAncestorOfType<ScrollWidget>();
		scroll->scrollTo(box);

		// Reset touchedParam
		APP->scene->rack->touchedParam = NULL;
		module->enableLearn(id);
	}

	void onDeselect(const DeselectEvent& e) override {
		if (!module)
			return;
		// Check if a ParamWidget was touched
		ParamWidget* touchedParam = APP->scene->rack->touchedParam;
		if (touchedParam) {
			APP->scene->rack->touchedParam = NULL;
			int64_t moduleId = touchedParam->module->id;
			int paramId = touchedParam->paramId;
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
				APP->event->setSelectedWidget(this);
		}
		else {
			bgColor = nvgRGBA(0, 0, 0, 0);

			// HACK
			if (APP->event->selectedWidget == this)
				APP->event->setSelectedWidget(NULL);
		}

		// Set text
		text = "";
		if (module->ccs[id] >= 0) {
			text += string::f("CC%02d: ", module->ccs[id]);
		}
		if (module->paramHandles[id].moduleId >= 0) {
			text += getParamName();
		}
		if (module->ccs[id] < 0 && module->paramHandles[id].moduleId < 0) {
			if (module->learningId == id) {
				text = "Mapping...";
			}
			else {
				text = "Unmapped";
			}
		}

		// Set text color
		if ((module->ccs[id] >= 0 && module->paramHandles[id].moduleId >= 0) || module->learningId == id) {
			color.a = 1.0;
		}
		else {
			color.a = 0.5;
		}
	}

	std::string getParamName() {
		if (!module)
			return "";
		if (id >= module->mapLen)
			return "";
		ParamHandle* paramHandle = &module->paramHandles[id];
		if (paramHandle->moduleId < 0)
			return "";
		ModuleWidget* mw = APP->scene->rack->getModule(paramHandle->moduleId);
		if (!mw)
			return "";
		// Get the Module from the ModuleWidget instead of the ParamHandle.
		// I think this is more elegant since this method is called in the app world instead of the engine world.
		Module* m = mw->module;
		if (!m)
			return "";
		int paramId = paramHandle->paramId;
		if (paramId >= (int) m->params.size())
			return "";
		ParamQuantity* paramQuantity = m->paramQuantities[paramId];
		std::string s;
		s += paramQuantity->name;
		s += " (";
		s += mw->model->name;
		s += ")";
		return s;
	}
};


struct MIDIMapDisplay : MidiDisplay {
	MIDIMap* module;
	ScrollWidget* scroll;
	MIDIMapChoice* choices[MAX_CHANNELS];
	LedDisplaySeparator* separators[MAX_CHANNELS];

	void setModule(MIDIMap* module) {
		this->module = module;

		scroll = new ScrollWidget;
		scroll->box.pos = channelChoice->box.getBottomLeft();
		scroll->box.size.x = box.size.x;
		scroll->box.size.y = box.size.y - scroll->box.pos.y;
		addChild(scroll);

		LedDisplaySeparator* separator = createWidget<LedDisplaySeparator>(scroll->box.pos);
		separator->box.size.x = box.size.x;
		addChild(separator);
		separators[0] = separator;

		Vec pos;
		for (int id = 0; id < MAX_CHANNELS; id++) {
			if (id > 0) {
				LedDisplaySeparator* separator = createWidget<LedDisplaySeparator>(pos);
				separator->box.size.x = box.size.x;
				scroll->container->addChild(separator);
				separators[id] = separator;
			}

			MIDIMapChoice* choice = createWidget<MIDIMapChoice>(pos);
			choice->box.size.x = box.size.x;
			choice->id = id;
			choice->setModule(module);
			scroll->container->addChild(choice);
			choices[id] = choice;

			pos = choice->box.getBottomLeft();
		}
	}

	void step() override {
		if (module) {
			int mapLen = module->mapLen;
			for (int id = 0; id < MAX_CHANNELS; id++) {
				choices[id]->visible = (id < mapLen);
				separators[id]->visible = (id < mapLen);
			}
		}

		MidiDisplay::step();
	}
};


struct MIDIMapWidget : ModuleWidget {
	MIDIMapWidget(MIDIMap* module) {
		setModule(module);
		setPanel(Svg::load(asset::system("res/Core/MIDIMap.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		MIDIMapDisplay* display = createWidget<MIDIMapDisplay>(mm2px(Vec(0.0, 12.869)));
		display->box.size = mm2px(Vec(50.8, 105.059));
		display->setMidiPort(module ? &module->midiInput : NULL);
		display->setModule(module);
		addChild(display);
	}

	void appendContextMenu(Menu* menu) override {
		MIDIMap* module = dynamic_cast<MIDIMap*>(this->module);

		menu->addChild(new MenuSeparator);

		menu->addChild(createBoolPtrMenuItem("Smooth CC", "", &module->smooth));
	}
};


Model* modelMIDIMap = createModel<MIDIMap, MIDIMapWidget>("MIDI-Map");


} // namespace core
} // namespace rack
