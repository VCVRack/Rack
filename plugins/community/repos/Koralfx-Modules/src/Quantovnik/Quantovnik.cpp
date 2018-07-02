#include "Quantovnik.hpp"

Quantovnik::Quantovnik() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Step ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Quantovnik::step() {
	float octave 	= params[OCTAVE_PARAM].value;
	float cv 		= inputs[CV_PITCH_INPUT].value + inputs[CV_COARSE_INPUT].value + (params[COARSE_PARAM].value/12.0);

	//Convert to Unipolar
	if (params[CV_IN_PARAM].value == 0) cv += 5;

	float note 			= round(cv  * 12);
	int noteKey 		= int(note) % 12;
	cv 					= round(octave) + (note / 12);
	int octaveNumber 	= floor(cv);
	//Convert to Bipolar
	if (params[CV_OUT_PARAM].value == 0) cv -= 5;
	outputs[CV_PITCH_OUTPUT].value = cv;

	//Light the right note light
	for (int i = 0; i < 12; i++) {
		lights[NOTE_LIGHT + i].value = (noteKey ==  i) ? 1.0 : 0.0;
	}
	//Light the right octave light
	for (int i = 0; i < 7; i++) {
		lights[OCTAVE_LIGHT + i].value = (octaveNumber ==  i+2) ? 1.0 : 0.0;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Store variables
///////////////////////////////////////////////////////////////////////////////

json_t *Quantovnik::toJson() {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "panelStyle", json_integer(panelStyle));

    return rootJ;
}

void Quantovnik::fromJson(json_t *rootJ) {
	json_t *j_panelStyle = json_object_get(rootJ, "panelStyle");
	panelStyle = json_integer_value(j_panelStyle);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// GUI ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

QuantovnikWidget::QuantovnikWidget(Quantovnik *module) : ModuleWidget(module){

	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		DynamicPanelWidget *panel = new DynamicPanelWidget();
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Quantovnik-Dark.svg")));
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Quantovnik-Light.svg")));
		box.size = panel->box.size;
		panel->mode = &module->panelStyle;
		addChild(panel);
	}

	//Knobs
	addParam(ParamWidget::create<Koralfx_StepRoundLargeBlackKnob>(Vec(26, 45),
		module, Quantovnik::OCTAVE_PARAM, -4, 4, 0.0));
	addParam(ParamWidget::create<Koralfx_StepRoundLargeBlackKnob>(Vec(45, 113),
		module, Quantovnik::COARSE_PARAM, -12, 12, 0.0));

	//Switches
	addParam(ParamWidget::create<Koralfx_Switch_Red>(Vec(18, 253),	module, Quantovnik::CV_IN_PARAM, 0.0, 1.0, 1.0));
	addParam(ParamWidget::create<Koralfx_Switch_Red>(Vec(58, 253),	module, Quantovnik::CV_OUT_PARAM, 0.0, 1.0, 0.0));

	//Inputs
	addInput(Port::create<PJ301MPort>	(Vec(13, 298),	Port::INPUT, module, Quantovnik::CV_PITCH_INPUT));
	addInput(Port::create<PJ301MPort>	(Vec(10, 121),	Port::INPUT, module, Quantovnik::CV_COARSE_INPUT));

	//Outputs
	addOutput(Port::create<PJ301MPort>(Vec(52, 298), Port::OUTPUT, module, Quantovnik::CV_PITCH_OUTPUT));

	//Note lights - set base position
	float xPos		=   9;
	float yPos1		= 192;
	float yPos2		= 175;
	float xDelta	=  11;
	

	float lightPos [12]	= {0.0, 0.5, 1.0, 1.5, 2.0, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0};
	float rowPos [12]	= {yPos1, yPos2, yPos1, yPos2, yPos1, yPos1, yPos2, yPos1, yPos2, yPos1, yPos2, yPos1};

	for (int i = 0; i < 12; i++) {
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(xPos + lightPos[i] * xDelta, rowPos[i]),
			module, Quantovnik::NOTE_LIGHT +  i));
	}

	for (int i = 0; i < 7; i++) {
		addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(xPos + i * xDelta, 211),
			module, Quantovnik::OCTAVE_LIGHT + i));
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Context Menu ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

//Context menu code is adapted from The Dexter by Dale Johnson
//https://github.com/ValleyAudio

struct QuantovnikPanelStyleItem : MenuItem {
    Quantovnik* module;
    int panelStyle;
    void onAction(EventAction &e) override {
        module->panelStyle = panelStyle;
    }
    void step() override {
        rightText = (module->panelStyle == panelStyle) ? "✔" : "";
        MenuItem::step();
    }
};

void QuantovnikWidget::appendContextMenu(Menu *menu) {
    Quantovnik *module = dynamic_cast<Quantovnik*>(this->module);
    assert(module);

    // Panel style
    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Frame of mind"));
    menu->addChild(construct<QuantovnikPanelStyleItem>(&MenuItem::text, "Dark Calm Night",
                                                       &QuantovnikPanelStyleItem::module, module, &QuantovnikPanelStyleItem::panelStyle, 0));
    menu->addChild(construct<QuantovnikPanelStyleItem>(&MenuItem::text, "Happy Bright Day",
                                                       &QuantovnikPanelStyleItem::module, module, &QuantovnikPanelStyleItem::panelStyle, 1));

}

////////////////////////////////////////////////////////////////////////////////////////////////////

RACK_PLUGIN_MODEL_INIT(Koralfx, Quantovnik) {
   Model *modelQuantovnik = Model::create<Quantovnik, QuantovnikWidget>("Koralfx-Modules", "Quantovnik", "Quantovnik",
                                                                        EFFECT_TAG);
   return modelQuantovnik;
}
