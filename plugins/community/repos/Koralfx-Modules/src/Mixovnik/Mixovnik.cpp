#include "Mixovnik.hpp"

Mixovnik::Mixovnik() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

void Mixovnik::step() {

	float SUM_L = 0.0;
	float SUM_R = 0.0;

	float SUM_AUX1_L = 0.0;
	float SUM_AUX1_R = 0.0;
	float SUM_AUX2_L = 0.0;
	float SUM_AUX2_R = 0.0;

	float cvVolumeRatio[16];
	

	//First Solo test
	bool soloTest = false;
	int step = 0;
	do  {
		if (params[SOLO_PARAM + step].value == 1) soloTest = true;
		step++;
	} while (!soloTest && step < 16);



	//Anti Pop Mix
	antiPopMixLeft -= (params[MIX_L_MUTE].value ==1) ? antiPopSpeed : -antiPopSpeed;
	if (antiPopMixLeft <0  )  antiPopMixLeft = 0;
	if (antiPopMixLeft >1  )  antiPopMixLeft = 1;
	
	antiPopMixRight -= (params[MIX_R_MUTE].value ==1) ? antiPopSpeed : -antiPopSpeed;
	if (antiPopMixRight <0  )  antiPopMixRight = 0;
	if (antiPopMixRight >1  )  antiPopMixRight = 1;

	//Anti Pop Auxes
	antiPopAux1 -= (params[AUX1_MUTE].value ==1) ? antiPopSpeed : -antiPopSpeed;
	if (antiPopAux1 <0  )  antiPopAux1 = 0;
	if (antiPopAux1 >1  )  antiPopAux1 = 1;
	
	antiPopAux2 -= (params[AUX2_MUTE].value ==1) ? antiPopSpeed : -antiPopSpeed;
	if (antiPopAux2 <0  )  antiPopAux2 = 0;
	if (antiPopAux2 >1  )  antiPopAux2 = 1;



	// Main loop
	for (int i = 0; i < 16; i++) {

		//Get signal
		float INPUT_SIGNAL = 0;
		cvVolumeRatio[i] = 1;



		if (inputs[STRIPE_CV_VOL_INPUT + i].active) cvVolumeRatio[i] = inputs[STRIPE_CV_VOL_INPUT + i].value/10;


		bool nieparzystyStripe = (i%2 == 0) ? true : false;
		bool linkActive = (params[LINK_PARAM + ((i-1)/2)].value == 1) ? true : false;
		
		//stripes 1,3,5,7,9,11,13,15
		if (nieparzystyStripe) {
			INPUT_SIGNAL = inputs[STRIPE_INPUT + i].value * cvVolumeRatio[i] * params[VOLUME_PARAM + i].value;
		//stripes 2,4,6,8,10,12,14,16
		} else {
			//link with left stripes?
			if (linkActive) {
				INPUT_SIGNAL = inputs[STRIPE_INPUT + i].value * cvVolumeRatio[i-1] * params[VOLUME_PARAM + (i - 1)].value ;
			} else {
				INPUT_SIGNAL = inputs[STRIPE_INPUT + i].value * cvVolumeRatio[i] * params[VOLUME_PARAM + i].value;
			}
		}


		//MUTE SOLO test for Anti Pop

		//SOLO
		if (soloTest) {
			if (nieparzystyStripe) {
				antiPopCurrentSpeed[i] = (params[MUTE_PARAM + i].value == 0 && params[SOLO_PARAM + i].value == 1)
					? +antiPopSpeed : -antiPopSpeed;
			} else {
				if (linkActive) {
					antiPopCurrentSpeed[i] = (params[MUTE_PARAM + i -1].value == 0 && params[SOLO_PARAM + i -1].value == 1)
						? +antiPopSpeed : -antiPopSpeed;
				} else {
					antiPopCurrentSpeed[i] = (params[MUTE_PARAM + i].value == 0 && params[SOLO_PARAM + i].value == 1)
						? +antiPopSpeed : -antiPopSpeed;
				}
			}
		//NO SOLO	
		} else {
			if (nieparzystyStripe) {
				antiPopCurrentSpeed[i] = (params[MUTE_PARAM + i].value == 1) ? -antiPopSpeed : +antiPopSpeed;
			} else {
				if (linkActive) {
					antiPopCurrentSpeed[i] = (params[MUTE_PARAM + i -1].value == 1) ? -antiPopSpeed : +antiPopSpeed;
				} else {
					antiPopCurrentSpeed[i] = (params[MUTE_PARAM + i].value == 1) ? -antiPopSpeed : +antiPopSpeed;
				}
			}
		}


		//Anti Pop final

		if (!inputs[STRIPE_INPUT + i].active) antiPopCurrentSpeed[i] = -antiPopSpeed;

		antiPop[i] += antiPopCurrentSpeed[i]; 
		if (antiPop[i] <0  )  antiPop[i] = 0;
		if (antiPop[i] >1  )  antiPop[i] = 1;
		INPUT_SIGNAL *= antiPop[i];

		//Constant-power panning
		float KNOB_PAN_POS = params[PAN_PARAM + i].value + (inputs[STRIPE_CV_PAN_INPUT + i].value / 5);

		//Anti invert phase
		if (KNOB_PAN_POS < -1) KNOB_PAN_POS = -1;
		if (KNOB_PAN_POS > 1) KNOB_PAN_POS = 1;

		double angle = KNOB_PAN_POS * PI_4;
		float GAIN_SIGNAL_L = (float) (SQRT2_2 * (cos(angle) - sin(angle)));
		float GAIN_SIGNAL_R = (float) (SQRT2_2 * (cos(angle) + sin(angle)));

		
		SUM_L += INPUT_SIGNAL * GAIN_SIGNAL_L;
		SUM_R += INPUT_SIGNAL * GAIN_SIGNAL_R;




		//AUX1 stripe send
		float KNOB_AUX1_POS = params[AUX1_PARAM + i].value; 
		SUM_AUX1_L += KNOB_AUX1_POS * INPUT_SIGNAL * GAIN_SIGNAL_L ;
		SUM_AUX1_R += KNOB_AUX1_POS * INPUT_SIGNAL * GAIN_SIGNAL_R;

		//AUX2 stripe send
		float KNOB_AUX2_POS = params[AUX2_PARAM + i].value;
		SUM_AUX2_L += KNOB_AUX2_POS * INPUT_SIGNAL * GAIN_SIGNAL_L;
		SUM_AUX2_R += KNOB_AUX2_POS * INPUT_SIGNAL * GAIN_SIGNAL_R;

		//Lights
		float SIGNAL_LEVEL_ABS = fabs(INPUT_SIGNAL);

		if (SIGNAL_LEVEL_ABS == 0) {
			lights[SIGNAL_LIGHT_NORMAL + i*2 +0].value = 0;
			lights[SIGNAL_LIGHT_NORMAL + i*2 +1].value = 0;
		} 

		if (SIGNAL_LEVEL_ABS > 0 && SIGNAL_LEVEL_ABS < 5.0) {
			lights[SIGNAL_LIGHT_NORMAL + i*2 +0].value = 1;
			lights[SIGNAL_LIGHT_NORMAL + i*2 +1].value = 0;
		} 

		if (SIGNAL_LEVEL_ABS > 5) {
			lights[SIGNAL_LIGHT_NORMAL + i*2 +0].value = 0;
			lights[SIGNAL_LIGHT_NORMAL + i*2 +1].value = 1;
		} 

	}

	//AUX1 and AUX2 sends
	outputs[AUX1_OUTPUT_L].value = SUM_AUX1_L;
	outputs[AUX1_OUTPUT_R].value = SUM_AUX1_R;

	outputs[AUX2_OUTPUT_L].value = SUM_AUX2_L;
	outputs[AUX2_OUTPUT_R].value = SUM_AUX2_R;

	//AUX1 and AUX2 returns
	float AUX1_SUM_L =
		inputs[AUX1_INPUT_L].value * params[AUX1_VOLUME].value * antiPopAux1;

	float AUX1_SUM_R =
		inputs[AUX1_INPUT_R].value * params[AUX1_VOLUME].value * antiPopAux1;
	
	float AUX2_SUM_L =
		inputs[AUX2_INPUT_L].value * params[AUX2_VOLUME].value * antiPopAux2;
		
	float AUX2_SUM_R =
		inputs[AUX2_INPUT_R].value * params[AUX2_VOLUME].value * antiPopAux2;
	
	SUM_L += AUX1_SUM_L + AUX2_SUM_L;
	SUM_R += AUX1_SUM_R + AUX2_SUM_R;


	//Exteranal mix
	SUM_L +=  inputs[STEREO_INPUT_L].value;
	SUM_R +=  inputs[STEREO_INPUT_R].value;

	//Mix sliders
	SUM_L *=  params[MIX_L_VOLUME].value;
	SUM_R *= ((params[MIX_LINK].value ==  0) ? params[MIX_R_VOLUME].value : params[MIX_L_VOLUME].value);

	//Final mix with mute switches
	SUM_L *= antiPopMixLeft;

	if (params[MIX_LINK].value == 1) {
		SUM_R *= antiPopMixLeft;
	} else {
		SUM_R *= antiPopMixRight;
	}


	//Final out
	outputs[STEREO_OUTPUT_L].value = SUM_L;
	outputs[STEREO_OUTPUT_R].value = SUM_R;

	//Mix lights
	lights[MIX_LIGHT_L].value = (fabs(SUM_L) > 5) ? 1.0 : 0.0;
	lights[MIX_LIGHT_R].value = (fabs(SUM_R) > 5) ? 1.0 : 0.0;

	lights[AUX1_LIGHT].value = (fabs(AUX1_SUM_L) > 5 || fabs(AUX1_SUM_R)  > 5) ? 1.0 : 0.0;
	lights[AUX2_LIGHT].value = (fabs(AUX2_SUM_L) > 5 || fabs(AUX2_SUM_R)  > 5) ? 1.0 : 0.0;

}

///////////////////////////////////////////////////////////////////////////////
// Store variables
///////////////////////////////////////////////////////////////////////////////

json_t *Mixovnik::toJson() {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "panelStyle", json_integer(panelStyle));

	return rootJ;
}

void Mixovnik::fromJson(json_t *rootJ) {
	json_t *j_panelStyle = json_object_get(rootJ, "panelStyle");
	panelStyle = json_integer_value(j_panelStyle);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// GUI ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

MixovnikWidget::MixovnikWidget(Mixovnik *module) : ModuleWidget(module){

	box.size = Vec(58 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		DynamicPanelWidget *panel = new DynamicPanelWidget();
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Mixovnik-Dark.svg")));
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Mixovnik-Light.svg")));
		box.size = panel->box.size;
		panel->mode = &module->panelStyle;
		addChild(panel);
	}

//Set base position
	float xPos = 17;
	float yPos = 20;
	float xDelta = 40;
	
	//AUX1 inputs and outputs
	addInput(Port::create<PJ301MPort>(Vec(694, yPos +7), Port::INPUT, module, Mixovnik::AUX1_INPUT_L));
	addInput(Port::create<PJ301MPort>(Vec(733, yPos +7), Port::INPUT, module, Mixovnik::AUX1_INPUT_R));

	addOutput(Port::create<PJ301MPort>(Vec(780, yPos +7), Port::OUTPUT, module, Mixovnik::AUX1_OUTPUT_L));
	addOutput(Port::create<PJ301MPort>(Vec(815, yPos +7), Port::OUTPUT, module, Mixovnik::AUX1_OUTPUT_R));

	//AUX2 inputs and outputs
	addInput(Port::create<PJ301MPort>(Vec(694, yPos + 45), Port::INPUT, module, Mixovnik::AUX2_INPUT_L));
	addInput(Port::create<PJ301MPort>(Vec(733, yPos + 45), Port::INPUT, module, Mixovnik::AUX2_INPUT_R));

	addOutput(Port::create<PJ301MPort>(Vec(780, yPos + 45), Port::OUTPUT, module, Mixovnik::AUX2_OUTPUT_L));
	addOutput(Port::create<PJ301MPort>(Vec(815, yPos + 45), Port::OUTPUT, module, Mixovnik::AUX2_OUTPUT_R));

	//External stereo inputs
	addInput(Port::create<PJ301MPort>(Vec(699, yPos + 285), Port::INPUT, module, Mixovnik::STEREO_INPUT_L));
	addInput(Port::create<PJ301MPort>(Vec(733, yPos + 285), Port::INPUT, module, Mixovnik::STEREO_INPUT_R));

	//Stereo mix outputs
	addOutput(Port::create<PJ301MPort>(Vec(782, yPos + 285), Port::OUTPUT, module, Mixovnik::STEREO_OUTPUT_L));
	addOutput(Port::create<PJ301MPort>(Vec(816, yPos + 285), Port::OUTPUT, module, Mixovnik::STEREO_OUTPUT_R));

  
	//Stripes elements
	for (int i = 0; i < 16; i++) {
		addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>	(Vec(xPos + 27 + i*xDelta,	yPos + 110),
			module, Mixovnik::SIGNAL_LIGHT_NORMAL + i*2));
		
		addParam(ParamWidget::create<Koralfx_RoundBlackKnob> 			(Vec(xPos - 0.5 + i*xDelta,	yPos -   0.5),
			module, Mixovnik::AUX1_PARAM + i, 0, 1, 0.0));
		
		addParam(ParamWidget::create<Koralfx_RoundBlackKnob>			(Vec(xPos - 0.5 + i*xDelta,	yPos +  36.5),
			module, Mixovnik::AUX2_PARAM + i, 0, 1, 0.0));
		
		addParam(ParamWidget::create<Koralfx_RoundBlackKnob>			(Vec(xPos - 0.5 + i*xDelta,	yPos +  75.5),
			module, Mixovnik::PAN_PARAM + i, -1, 1, 0.0));
		
		addParam(ParamWidget::create<Koralfx_SliderPot>					(Vec(xPos + 3 + i*xDelta,	yPos + 110),
			module, Mixovnik::VOLUME_PARAM + i, 0.0f, 1.0f, 0.9f));
		
		addParam(ParamWidget::create<Koralfx_Switch_Red>				(Vec(xPos + 8 + i*xDelta,	yPos + 228),
			module, Mixovnik::MUTE_PARAM + i, 0.0, 1.0, 0.0));
		
		if (i%2 == 0) addParam(ParamWidget::create<Koralfx_Switch_Blue>(Vec(xPos + 8 + (i+0.5)*xDelta,	yPos + 228),
			module, Mixovnik::LINK_PARAM + (i/2), 0.0, 1.0, 0.0));
		
		addInput(Port::create<PJ301MPort>								(Vec(xPos + 3 + i*xDelta,	yPos + 266),
			Port::INPUT, module, Mixovnik::STRIPE_INPUT + i));
		
		addInput(Port::create<PJ301MPort>								(Vec(xPos + 3 + i*xDelta,	yPos + 292),
			Port::INPUT, module, Mixovnik::STRIPE_CV_PAN_INPUT + i));
		
		addInput(Port::create<PJ301MPort>								(Vec(xPos + 3 + i*xDelta,	yPos + 318),
			Port::INPUT, module, Mixovnik::STRIPE_CV_VOL_INPUT + i));
		
		addParam(ParamWidget::create<Koralfx_Switch_Green>				(Vec(xPos + 8 + i*xDelta,	yPos + 245),
			module, Mixovnik::SOLO_PARAM + i, 0.0, 1.0, 0.0));
	}

	//Final volume sliders
	addParam(ParamWidget::create<Koralfx_SliderPot>(Vec(xPos - 2 + 17*xDelta, yPos + 110),
		module, Mixovnik::AUX1_VOLUME, 0.0f, 1.0f, 0.9f));

	addParam(ParamWidget::create<Koralfx_SliderPot>(Vec(xPos - 2 + 18*xDelta, yPos + 110),
		module, Mixovnik::AUX2_VOLUME, 0.0f, 1.0f, 0.9f));

	addParam(ParamWidget::create<Koralfx_SliderPot>(Vec(xPos + 3 + 19*xDelta, yPos + 110),
		module, Mixovnik::MIX_L_VOLUME, 0.0f, 1.0f, 0.9f));

	addParam(ParamWidget::create<Koralfx_SliderPot>(Vec(xPos + 1 + 20*xDelta, yPos + 110),
		module, Mixovnik::MIX_R_VOLUME, 0.0f, 1.0f, 0.9f));

	//Final mute switches
	addParam(ParamWidget::create<Koralfx_Switch_Red>(Vec(xPos + 3 + 17*xDelta, yPos + 227),
		module, Mixovnik::AUX1_MUTE, 0, 1, 0));

	addParam(ParamWidget::create<Koralfx_Switch_Red>(Vec(xPos + 3 + 18*xDelta, yPos + 227),
		module, Mixovnik::AUX2_MUTE, 0, 1, 0));

	addParam(ParamWidget::create<Koralfx_Switch_Red>(Vec(xPos + 8 + 19*xDelta, yPos + 227),
		module, Mixovnik::MIX_L_MUTE, 0, 1, 0));

	addParam(ParamWidget::create<Koralfx_Switch_Red>(Vec(xPos + 6 + 20*xDelta, yPos + 227),
		module, Mixovnik::MIX_R_MUTE, 0, 1, 0));

	//Stereo mix link switch
	addParam(ParamWidget::create<Koralfx_Switch_Blue>(Vec(xPos + 7 + 19.5*xDelta, yPos + 227),
		module, Mixovnik::MIX_LINK, 0, 1, 0));

	//Final mix lights
	addChild(ModuleLightWidget::create<SmallLight<RedLight>>	(Vec(703,120), module, Mixovnik::AUX1_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<RedLight>>	(Vec(743,120), module, Mixovnik::AUX2_LIGHT));

	addChild(ModuleLightWidget::create<SmallLight<RedLight>>	(Vec(788,120), module, Mixovnik::MIX_LIGHT_L));
	addChild(ModuleLightWidget::create<SmallLight<RedLight>>	(Vec(826,120), module, Mixovnik::MIX_LIGHT_R));

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Context Menu ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

//Context menu code is adapted from The Dexter by Dale Johnson
//https://github.com/ValleyAudio

struct MixovnikPanelStyleItem : MenuItem {
	Mixovnik* module;
	int panelStyle;
	void onAction(EventAction &e) override {
		module->panelStyle = panelStyle;
	}
	void step() override {
		rightText = (module->panelStyle == panelStyle) ? "✔" : "";
		MenuItem::step();
	}
};

void MixovnikWidget::appendContextMenu(Menu *menu) {
	Mixovnik *module = dynamic_cast<Mixovnik*>(this->module);
	assert(module);

	// Panel style
	menu->addChild(construct<MenuLabel>());
	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Frame of mind"));

	menu->addChild(construct<MixovnikPanelStyleItem>(&MenuItem::text, "Dark Calm Night",
		&MixovnikPanelStyleItem::module, module, &MixovnikPanelStyleItem::panelStyle, 0));

	menu->addChild(construct<MixovnikPanelStyleItem>(&MenuItem::text, "Happy Bright Day",
		&MixovnikPanelStyleItem::module, module, &MixovnikPanelStyleItem::panelStyle, 1));

}

////////////////////////////////////////////////////////////////////////////////////////////////////

RACK_PLUGIN_MODEL_INIT(Koralfx, Mixovnik) {
   Model *modelMixovnik = Model::create<Mixovnik, MixovnikWidget>("Koralfx-Modules", "Mixovnik", "Mixovnik", MIXER_TAG);
   return modelMixovnik;
}
