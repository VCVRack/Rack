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
	dsp::ExponentialFilter valueFilters[8];

	MIDI_Map() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		onReset();
	}

	void onReset() override {
		midiInput.reset();
	}

	void step() override {
	}
};


struct MIDI_MapChoice : LedDisplayChoice {
	MIDI_Map *module;

	void setModule(MIDI_Map *module) {
		this->module = module;
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
