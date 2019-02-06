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
		ENUMS(MAP_LIGHTS, 16),
		NUM_LIGHTS
	};

	midi::InputQueue midiInput;
	int8_t values[128];
	int learningId = -1;
	int learnedCcs[16] = {};
	bool mapped[16] = {};

	MIDI_Map() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		onReset();
	}

	void onReset() override {
		for (int i = 0; i < 128; i++) {
			values[i] = 0;
		}
		for (int i = 0; i < 16; i++) {
			learnedCcs[i] = i;
		}
		learningId = -1;
		midiInput.reset();
	}

	void step() override {
		for (int i = 0; i < 16; i++) {
			lights[MAP_LIGHTS + i].setBrightness(mapped[i]);
		}
	}
};


struct CKD6Button : SvgButton {
	MIDI_Map *module;
	int id;

	CKD6Button() {
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/CKD6_0.svg")));
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/CKD6_1.svg")));
	}

	void onAction(const event::Action &e) override {
		module->mapped[id] ^= true;
	}
};

template <class TWidget>
TWidget *createMapButtonCentered(math::Vec pos, MIDI_Map *module, int id) {
	TWidget *o = new TWidget;
	o->box.pos = pos.minus(o->box.size.div(2));;
	o->module = module;
	o->id = id;
	return o;
}


struct MIDI_MapWidget : ModuleWidget {
	MIDI_MapWidget(MIDI_Map *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::system("res/Core/MIDI-Map.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(8, 77)), module, 0));
		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(20, 77)), module, 1));
		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(31, 77)), module, 2));
		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(43, 77)), module, 3));
		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(8, 89)), module, 4));
		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(20, 89)), module, 5));
		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(31, 89)), module, 6));
		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(43, 89)), module, 7));
		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(8, 101)), module, 8));
		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(20, 101)), module, 9));
		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(31, 101)), module, 10));
		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(43, 101)), module, 11));
		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(8, 112)), module, 12));
		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(20, 112)), module, 13));
		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(31, 112)), module, 14));
		addChild(createMapButtonCentered<CKD6Button>(mm2px(Vec(43, 112)), module, 15));

		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(8, 77)), module, MIDI_Map::MAP_LIGHTS + 0));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(20, 77)), module, MIDI_Map::MAP_LIGHTS + 1));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(31, 77)), module, MIDI_Map::MAP_LIGHTS + 2));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(43, 77)), module, MIDI_Map::MAP_LIGHTS + 3));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(8, 89)), module, MIDI_Map::MAP_LIGHTS + 4));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(20, 89)), module, MIDI_Map::MAP_LIGHTS + 5));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(31, 89)), module, MIDI_Map::MAP_LIGHTS + 6));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(43, 89)), module, MIDI_Map::MAP_LIGHTS + 7));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(8, 101)), module, MIDI_Map::MAP_LIGHTS + 8));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(20, 101)), module, MIDI_Map::MAP_LIGHTS + 9));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(31, 101)), module, MIDI_Map::MAP_LIGHTS + 10));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(43, 101)), module, MIDI_Map::MAP_LIGHTS + 11));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(8, 112)), module, MIDI_Map::MAP_LIGHTS + 12));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(20, 112)), module, MIDI_Map::MAP_LIGHTS + 13));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(31, 112)), module, MIDI_Map::MAP_LIGHTS + 14));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(43, 112)), module, MIDI_Map::MAP_LIGHTS + 15));

		typedef Grid16MidiWidget<CcChoice<MIDI_Map>> TMidiWidget;
		TMidiWidget *midiWidget = createWidget<TMidiWidget>(mm2px(Vec(3.399621, 14.837339)));
		midiWidget->box.size = mm2px(Vec(44, 54.667));
		if (module)
			midiWidget->midiIO = &module->midiInput;
		midiWidget->setModule(module);
		addChild(midiWidget);
	}
};


Model *modelMIDI_Map = createModel<MIDI_Map, MIDI_MapWidget>("MIDI-Map");

