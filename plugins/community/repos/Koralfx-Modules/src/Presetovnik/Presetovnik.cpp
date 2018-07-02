#include "Presetovnik.hpp"
#include "app.hpp"
#include "../Koralfx-Modules.hpp"

Presetovnik::Presetovnik() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	onReset();
}


void Presetovnik::onReset() {
	//Default values
	for (int i = 0; i < 10 ; i += 1) {
		for (int k = 0; k < 8 ; k += 1) {
			presetKnobMemory[i][k] = 0.5f;
			presetUniMemory[i][k] = true;
		}
	}
	presetChange = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Step ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Presetovnik::step() {

	//Test change unipolar led buttons
	int unipolarLed = -1;
	for (int i = 0; i < 8 ; i += 1) {
		if (unipolarTrigger[i].process(params[LED_UNI_PARAM + i].value)) {
			unipolarChange = true;
			unipolarLed = i;
		}
	}

	//Some unipolar button is pressed
	if (unipolarChange) {
		lights[UNI_LIGHT + unipolarLed].value = (lights[UNI_LIGHT + unipolarLed].value == 1) ? 0 : 1;
		presetUniMemory [preset][unipolarLed] = (lights[UNI_LIGHT + unipolarLed].value == 1) ? true : false;
		unipolarChange = false;
	}

	//CV Preset input active changed
	if (inputs[CV_PRESET_INPUT].active != cvPresetInputActiveOld) {
		for (int i = 0; i < 30 ; i += 1) {
			lights[PRESET_LIGHT + i ].value = 0;
		}
		presetChange = true;
		cvPresetInputActiveOld = !cvPresetInputActiveOld;
		sourcePreset = (cvPresetInputActiveOld) ? 1 : 2;
	}

	//Preset internal
	if (!inputs[CV_PRESET_INPUT].active) {
		for (int i = 0; i < 10 ; i += 1) {
			if (presetTrigger[i].process(params[LED_BUTTON_PRESET_PARAM + i].value)) {
				preset = i;
				presetChange = true;
				sourcePreset = 2;
			}
		}
	} else {
		//preset external
		if (cvPresetInputOld != inputs[CV_PRESET_INPUT].value) {
			sourcePreset = 1;
			cvPresetInputOld = inputs[CV_PRESET_INPUT].value;
			preset = floor(cvPresetInputOld) - 1;
			if (preset == -1) preset = 0;
			presetChange = true;
			sourcePreset = 1;

		}
	}

	//Preset is changed
	if (presetChange) {
		lights[PRESET_LIGHT + (presetOld) * 3 + sourcePreset ].value = 0;
		lights[PRESET_LIGHT + (preset) * 3 + sourcePreset ].value = 1;
		presetChange = false;
		presetOld = preset;
		outputs[CV_PRESET_OUTPUT].value = preset+1;
		//Restore knob pointers and uni leds from preset memory
		for (int i = 0; i < 8 ; i += 1) {
			pointerKnob [i] = presetKnobMemory [preset][i];
			lights[UNI_LIGHT + i].value = presetUniMemory [preset][i];
		}
	}

	//Display and store knobs pointers
	for (int i = 0; i < 8 ; i += 1) {
		float knobValue = params[KNOB_PARAM + i].value;
		float pointerValue = pointerKnob [i];
		if (fabs(knobValue - pointerValue)<0.001) {
			pointerKnob [i] = params[KNOB_PARAM + i].value;
			presetKnobMemory [preset][i] =  pointerKnob [i];
		}
	}

	for (int i = 0; i < 8 ; i += 1) {
		float output = pointerKnob [i] * 10;
		float uniOutput = (lights[UNI_LIGHT + i].value == 0) ? 5: 0;
		if (!inputs[CV_PARAM_INPUT + i].active) {
			outputs[CV_PARAM_OUTPUT+ i].value = output - uniOutput;
			colorPointer[i] = nvgRGB(0x55, 0xaa, 0xff);
		} else {
			outputs[CV_PARAM_OUTPUT+ i].value = inputs[CV_PARAM_INPUT + i].value - uniOutput;
			pointerKnob [i] = inputs[CV_PARAM_INPUT + i].value / 10;
			colorPointer[i] = nvgRGB(0x55, 0xff, 0x55);
			//Store value in preset memory ?
			//presetKnobMemory [preset][i] =  pointerKnob [i];
		}
	}

}

///////////////////////////////////////////////////////////////////////////////
// Store variables
///////////////////////////////////////////////////////////////////////////////

json_t *Presetovnik::toJson() {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "panelStyle", json_integer(panelStyle));

    //preset

    json_object_set_new(rootJ, "preset", json_integer(preset));

	// knobs
	int count = 0;
	json_t *knobsJ = json_array();
	for (int i = 0; i < 10; i++) {
		for (int k = 0; k < 8; k++) {
			json_array_insert_new(knobsJ, count, json_real(presetKnobMemory[i][k]));
			count++;
		}
	}
	json_object_set_new(rootJ, "knobs", knobsJ);

	// uni
	count = 0;
	json_t *uniJ = json_array();
	for (int i = 0; i < 10; i++) {
		for (int k = 0; k < 8; k++) {
			json_array_insert_new(uniJ, count , json_integer(presetUniMemory[i][k]));
			count++;
		}
	}
	json_object_set_new(rootJ, "uni", uniJ);

    return rootJ;
}

void Presetovnik::fromJson(json_t *rootJ) {
	json_t *j_panelStyle = json_object_get(rootJ, "panelStyle");
	panelStyle = json_integer_value(j_panelStyle);


	json_t *j_preset = json_object_get(rootJ, "preset");
	preset = json_integer_value(j_preset);


	int count = 0;
	// knobs
	json_t *knobsJ = json_object_get(rootJ, "knobs");
	if (knobsJ) {
		for (int i = 0; i < 10; i++) {
			for (int k = 0; k < 8; k++) {
				json_t *knobJ = json_array_get(knobsJ, count);
				presetKnobMemory[i][k] = json_real_value(knobJ);
				count++;
			}
		}
	}


	count = 0;
	json_t *unisJ = json_object_get(rootJ, "uni");
	if (unisJ) {
		for (int i = 0; i < 10; i++) {
			for (int k = 0; k < 8; k++) {
				json_t *uniJ = json_array_get(unisJ, count);
				presetUniMemory[i][k] = (json_integer_value(uniJ) == 1) ? true : false;
				count++;
			}
		}
	}

}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// GUI ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

PresetovnikWidget::PresetovnikWidget(Presetovnik *module) : ModuleWidget(module){

	box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		DynamicPanelWidget *panel = new DynamicPanelWidget();
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Presetovnik-Dark.svg")));
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Presetovnik-Light.svg")));
		box.size = panel->box.size;
		panel->mode = &module->panelStyle;
		addChild(panel);
	}

	float leftPos = 127;
	float topPos = 96;
	float deltaY = 50;

	for (int i = 0; i < 4 ; i += 1) {
		for (int k = 0; k < 2 ; k += 1) {
			{	
				Koralfx_knobRing *scale = new Koralfx_knobRing();
				scale->box.pos = Vec(leftPos + 16.5 + (k * 65), topPos + 18.5 + i *deltaY);
				scale->pointerKnob = &module->pointerKnob[i * 2 + k];
				scale->colorPointer = &module->colorPointer[i * 2 + k];
				addChild(scale);
			}
				addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(leftPos - 2 + (k * 65), topPos+ i * deltaY),
					module, Presetovnik::KNOB_PARAM + (i * 2 + k), 0.0f, 1.0f, 0.5f));
				addInput(Port::create<PJ301MPort>(Vec(leftPos - 116 + (k * 27), topPos + 4 + i * deltaY), Port::INPUT,
					module, Presetovnik::CV_PARAM_INPUT + (i * 2 + k)));

				addParam(ParamWidget::create<Koralfx_LEDButton>
					(Vec(leftPos - 60 + (k * 17), topPos + 10 + i * deltaY),
						module, Presetovnik::LED_UNI_PARAM + (i * 2 + k),    0, 1, 0));

				addChild(ModuleLightWidget::create<SmallLight<BlueLight>>
					(Vec(leftPos - 56 + (k * 17), topPos + 14 + i * deltaY),
						module, Presetovnik::UNI_LIGHT + (i * 2 + k)));

			
		}
	}
	for (int i = 0; i < 8 ; i += 1) {
		addOutput(Port::create<PJ301MPort>(Vec(13 + (i * 27), 302), Port::OUTPUT,
			module, Presetovnik::CV_PARAM_OUTPUT + i));
	}

	addInput(Port::create<PJ301MPort>	(Vec(10, 47),	Port::INPUT, module, Presetovnik::CV_PRESET_INPUT));
	addOutput(Port::create<PJ301MPort>	(Vec(205, 47),	Port::OUTPUT, module, Presetovnik::CV_PRESET_OUTPUT));

	for (int i = 0; i < 10 ; i += 1) {
		addParam(ParamWidget::create<Koralfx_LEDButton>(Vec(40 + (i * 16), 56),
			module, Presetovnik::LED_BUTTON_PRESET_PARAM + i,    0, 1, 0));

		addChild(ModuleLightWidget::create<SmallLight<RedGreenBlueLight>>(Vec(44 + (i * 16), 60),
			module, Presetovnik::PRESET_LIGHT + (i * 3)));
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Context Menu ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

//Context menu code is adapted from The Dexter by Dale Johnson
//https://github.com/ValleyAudio

struct PresetovnikPanelStyleItem : MenuItem {
    Presetovnik* module;
    int panelStyle;
    void onAction(EventAction &e) override {
        module->panelStyle = panelStyle;
    }
    void step() override {
        rightText = (module->panelStyle == panelStyle) ? "✔" : "";
        MenuItem::step();
    }
};

void PresetovnikWidget::appendContextMenu(Menu *menu) {
    Presetovnik *module = dynamic_cast<Presetovnik*>(this->module);
    assert(module);

    // Panel style
    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Frame of mind"));
    menu->addChild(construct<PresetovnikPanelStyleItem>(&MenuItem::text, "Dark Calm Night",
    	&PresetovnikPanelStyleItem::module, module, &PresetovnikPanelStyleItem::panelStyle, 0));
    menu->addChild(construct<PresetovnikPanelStyleItem>(&MenuItem::text, "Happy Bright Day",
    	&PresetovnikPanelStyleItem::module, module, &PresetovnikPanelStyleItem::panelStyle, 1));

}

////////////////////////////////////////////////////////////////////////////////////////////////////

RACK_PLUGIN_MODEL_INIT(Koralfx, Presetovnik) {
   Model *modelPresetovnik = Model::create<Presetovnik, PresetovnikWidget>("Koralfx-Modules", "Presetovnik", "Presetovnik",
                                                                           UTILITY_TAG);
   return modelPresetovnik;
}
