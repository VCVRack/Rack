#include "Beatovnik.hpp"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Step ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Beatovnik::Beatovnik() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

void Beatovnik::step() {
	float deltaTime = engineGetSampleTime();
	if (inputs[CLOCK_INPUT].active) {
		float clockInput = inputs[CLOCK_INPUT].value;
		//A rising slope
		if ((clockTrigger.process(inputs[CLOCK_INPUT].value)) && !inMemory) {
			beatCount ++;
			stepper 					= 0;
			lights[CLOCK_LIGHT].value 	= 1;
			inMemory 					= true;
			LightPulseGenerator.trigger(0.1);

			//Set divide with tempo knob 
			int choice 	= round(params[TIME_PARAM].value);
			float ratio =0;
			switch(choice) {
				case 0:
				ratio = 0.25; 
				break;
				
				case 1:
				ratio = 0.5; 
				break;
				
				case 2:
				ratio = 0.75; 
				break;

				case 3:
				ratio = 1.0; 
				break;

				case 4:
				ratio = 1.25 ;
				break;

				case 5:
				ratio = 1.5 ;
				break;

				case 6:
				ratio = 2 ;
				break;

				case 7:
				ratio = 4 ;
				break;
			}

			float divide 	= ratio * ((params[TIME_T_PARAM].value) ? 0.333 : 1);
			float CV 		= (log(beatOld * 1000 * divide) /log(10 / 1e-3));

			outputs[CV_OUTPUT].value 	= CV * 10;

			for (int i = 0; i < 9; i++) {
				lights[NOTE_LIGHT + i].value = ((choice == i) ? 1 : 0);
			}

			//BPM is locked
			if (beatCount == 2) {
				lights[CLOCK_LOCK_LIGHT].value = 1;
				beatLock = true;
				beatOld = beatTime;
				tempo = std::to_string((int)round(60/beatOld));
				colorDisplay 	= nvgRGB(0xff, 0xcc, 0x00);
			}

			//BPM lost
			if (beatCount > 2) {
				if (fabs(beatOld - beatTime) > 0.005) {
					beatLock = false;
					beatCount = 0;
					lights[CLOCK_LOCK_LIGHT].value = 0;
					stepper = 0;
					stepperInc = false;
					for (int i = 0; i < 13; i++) {
						lights[CLOCK_LIGHT + i].value = 0;
						tempo = "---" ;
						colorDisplay 	= nvgRGB(0xff, 0x00, 0x00);
					}
				}
			}
			beatTime = 0;
		}

		//Falling slope
		if (clockInput <= 0 && inMemory) {
			//lights[CLOCK_LIGHT].value = 0;
			inMemory = false;
		}

		//When BPM is locked...
		if (beatLock) {
			int division = 16;
			int bM = round((beatOld / division) * 1000000);
			int bT = round(beatTime * 1000000);
			
			if ( bT % bM <= deltaTime * 1000000) {
				stepperInc = true;
			}

			//MULTIPLY 4X
			int noteRate = 4;
			if (stepperInc) {
				if ( stepper % noteRate == 0) {
					lights[BEAT4X_MUL_LIGHT].value 			= 1;
					outputs[BEAT4X_MUL_OUTPUT].value 		= 10;
					gateDec4xMul 							= true;
				}
			}

			//led off
			if (gateDec4xMul) gateWidth4xMul -= 1;
			if (gateWidth4xMul <= 0 ) {
				gateDec4xMul 	= false;
				gateWidth4xMul 	= params[WIDTH_PARAM].value * (beatOld / deltaTime / noteRate);
				lights[BEAT4X_MUL_LIGHT].value 				= 0;
				outputs[BEAT4X_MUL_OUTPUT].value 			= 0;
			}

			//MULTIPLY 2X
			noteRate = 8;
			if (stepperInc) {
				if ( stepper % noteRate == 0) {
					lights[BEAT2X_MUL_LIGHT].value 			= 1;
					outputs[BEAT2X_MUL_OUTPUT].value 		= 10;
					gateDec2xMul 							= true;
				}
			}

			//led off
			if (gateDec2xMul) gateWidth2xMul -= 1;
			if (gateWidth2xMul <= 0 ) {
				gateDec2xMul 	= false;
				gateWidth2xMul 	= params[WIDTH_PARAM].value * (beatOld / deltaTime / noteRate) * 4;
				lights[BEAT2X_MUL_LIGHT].value 				= 0;
				outputs[BEAT2X_MUL_OUTPUT].value 			= 0;
			}

			if (beatCountMemory != beatCount) {
				//DIV 2X
				if (stepperInc) {
					if ( beatCount % 2 == 0) {
						lights[BEAT2X_DIV_LIGHT].value 		= 1;
						outputs[BEAT2X_DIV_OUTPUT].value 	= 10;
						gateDec2xDiv 						= true;
					}
				}

				//DIV 4X
				if (stepperInc) {
					if ( beatCount % 4 == 0) {
						lights[BEAT4X_DIV_LIGHT].value 		= 1;
						outputs[BEAT4X_DIV_OUTPUT].value 	= 10;
						gateDec4xDiv 						= true;
					}
				}

			}

			//DIV 2X led off
			if (gateDec2xDiv) gateWidth2xDiv 				-= 1;
			if (gateWidth2xDiv <= 0 ) {
				gateDec2xDiv = false;
				gateWidth2xDiv = params[WIDTH_PARAM].value * (beatOld / deltaTime / noteRate) * 16;
				lights[BEAT2X_DIV_LIGHT].value 				= 0;
				outputs[BEAT2X_DIV_OUTPUT].value 			= 0;
			}

			//DIV 4X led off
			if (gateDec4xDiv) gateWidth4xDiv 				-= 1;
			if (gateWidth4xDiv <= 0 ) {
				gateDec4xDiv 								= false;
				gateWidth4xDiv = params[WIDTH_PARAM].value * (beatOld / deltaTime / noteRate) * 32;
				lights[BEAT4X_DIV_LIGHT].value 				= 0;
				outputs[BEAT4X_DIV_OUTPUT].value 			= 0;
			}

			//Next step of stepper
			if (stepperInc) {
				stepper++;
				stepperInc = false;
			}

		} //end of beatLock routine



		beatTime += deltaTime;

		//when beat is lost
		if (beatTime > 2 ) {
			beatLock = false;
			beatCount = 0;
			lights[CLOCK_LOCK_LIGHT].value = 0;
			stepper = 0;
			stepperInc = false;
			tempo = "---" ;
			colorDisplay 	= nvgRGB(0xff, 0x00, 0x00);
			for (int i = 0; i < 13; i++) {
				lights[CLOCK_LIGHT + i].value = 0;
			}
		}
		beatCountMemory = beatCount;
		} else {
			beatLock = false;
			beatCount = 0;
			tempo = "OFF" ;
			colorDisplay 	= nvgRGB(0x00, 0xcc, 0xff);
			for (int i = 0; i < 13; i++) {
				lights[CLOCK_LIGHT + i].value = 0;
		}

	} //end of input active routine

	bool pulse = LightPulseGenerator.process(1.0 / engineGetSampleRate());
	if (pulse == 0) lights[CLOCK_LIGHT].value = 0;

}

///////////////////////////////////////////////////////////////////////////////
// Store variables
///////////////////////////////////////////////////////////////////////////////

json_t *Beatovnik::toJson() {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "panelStyle", json_integer(panelStyle));

	return rootJ;
}

void Beatovnik::fromJson(json_t *rootJ) {
	json_t *j_panelStyle = json_object_get(rootJ, "panelStyle");
	panelStyle = json_integer_value(j_panelStyle);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// GUI ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BeatovnikWidget::BeatovnikWidget(Beatovnik *module) : ModuleWidget(module){

	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		DynamicPanelWidget *panel = new DynamicPanelWidget();
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Beatovnik-Dark.svg")));
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Beatovnik-Light.svg")));
		box.size = panel->box.size;
		panel->mode = &module->panelStyle;
		addChild(panel);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Displays
	///////////////////////////////////////////////////////////////////////////////

	Seg3DisplayWidget *display = new Seg3DisplayWidget();
	display->box.pos = Vec(6,290);
	display->box.size = Vec(45, 20);
	display->value = &module->tempo;
	display->colorDisplay = &module->colorDisplay;
	addChild(display);

	//Knobs
	addParam(ParamWidget::create<Koralfx_RoundBlackKnob>(Vec(50.5, 248.5),
		module, Beatovnik::WIDTH_PARAM,	0.01, 0.99, 0.5));
	addParam(ParamWidget::create<Koralfx_StepRoundLargeBlackKnob>(Vec(6, 73),
		module, Beatovnik::TIME_PARAM,	0, 7, 2));

	//Buttons
	addParam(ParamWidget::create<Koralfx_Switch_Blue>(Vec(54,60), module, Beatovnik::TIME_T_PARAM, 0, 1, 0));


	//Inputs
	addInput(Port::create<PJ301MPort>	(Vec(13, 251),	Port::INPUT, module, Beatovnik::CLOCK_INPUT));

	//Outputs
	addOutput(Port::create<PJ301MPort>(Vec(52, 88), Port::OUTPUT, module, Beatovnik::CV_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(14, 146), Port::OUTPUT, module, Beatovnik::BEAT2X_MUL_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(52, 146), Port::OUTPUT, module, Beatovnik::BEAT4X_MUL_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(14, 206), Port::OUTPUT, module, Beatovnik::BEAT2X_DIV_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(52, 206), Port::OUTPUT, module, Beatovnik::BEAT4X_DIV_OUTPUT));

	//Lights
	addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(17, 323), module, Beatovnik::CLOCK_LIGHT));
	addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(69, 321), module, Beatovnik::CLOCK_LOCK_LIGHT));


	addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(30, 176), module, Beatovnik::BEAT2X_MUL_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(69, 176), module, Beatovnik::BEAT4X_MUL_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(30, 236), module, Beatovnik::BEAT2X_DIV_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(69, 236), module, Beatovnik::BEAT4X_DIV_LIGHT));


	for (int i = 0; i < 8; i++) {
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(9+i*9.5, 53), module, Beatovnik::NOTE_LIGHT + i));
	}





}

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Context Menu ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

//Context menu code is adapted from The Dexter by Dale Johnson
//https://github.com/ValleyAudio

struct BeatovnikPanelStyleItem : MenuItem {
	Beatovnik* module;
	int panelStyle;
	void onAction(EventAction &e) override {
		module->panelStyle = panelStyle;
	}
	void step() override {
		rightText = (module->panelStyle == panelStyle) ? "✔" : "";
		MenuItem::step();
	}
};

void BeatovnikWidget::appendContextMenu(Menu *menu) {
	Beatovnik *module = dynamic_cast<Beatovnik*>(this->module);
	assert(module);

	// Panel style
	menu->addChild(construct<MenuLabel>());
	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Frame of mind"));
	
	menu->addChild(construct<BeatovnikPanelStyleItem>(&MenuItem::text, "Dark Calm Night",
		&BeatovnikPanelStyleItem::module, module, &BeatovnikPanelStyleItem::panelStyle, 0));
	
	menu->addChild(construct<BeatovnikPanelStyleItem>(&MenuItem::text, "Happy Bright Day",
		&BeatovnikPanelStyleItem::module, module, &BeatovnikPanelStyleItem::panelStyle, 1));

}

////////////////////////////////////////////////////////////////////////////////////////////////////

RACK_PLUGIN_MODEL_INIT(Koralfx, Beatovnik) {
   Model *modelBeatovnik = Model::create<Beatovnik, BeatovnikWidget>("Koralfx-Modules", "Beatovnik", "Beatovnik",
                                                                     CLOCK_TAG, CLOCK_MODULATOR_TAG);
   return modelBeatovnik;
}
