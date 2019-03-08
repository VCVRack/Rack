//**************************************************************************************
//4 channel mixer module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//Based on DrumsMixer VCV Rack by Autodafe http://www.autodafe.net
//Based on code taken from the Fundamentals plugins by Andrew Belt http://www.vcvrack.com
//**************************************************************************************

#include "AS.hpp"
#include "dsp/digital.hpp"

struct Mixer4ch : Module {
	enum ParamIds {
		MIX_PARAM,
		CH1_PARAM,
		CH2_PARAM,
		CH3_PARAM,
		CH4_PARAM,

		CH1_PAN_PARAM,
		CH2_PAN_PARAM,
		CH3_PAN_PARAM,
		CH4_PAN_PARAM,

		CH1MUTE,
		CH2MUTE,
		CH3MUTE,
		CH4MUTE,

		MASTER_MUTE,
		NUM_PARAMS
	};
	enum InputIds {
		MIX_CV_INPUT,

		CH1_INPUT,
		CH1_CV_INPUT,
		CH1_CV_PAN_INPUT,

		CH2_INPUT,
		CH2_CV_INPUT,
		CH2_CV_PAN_INPUT,

		CH3_INPUT,
		CH3_CV_INPUT,
		CH3_CV_PAN_INPUT,

		CH4_INPUT,
		CH4_CV_INPUT,
		CH4_CV_PAN_INPUT,


		LINK_L,
		LINK_R,
		NUM_INPUTS
	};
	enum OutputIds {
		MIX_OUTPUTL,
		MIX_OUTPUTR,
		CH1_OUTPUT,
		CH2_OUTPUT,
		CH3_OUTPUT,
		CH4_OUTPUT,

		NUM_OUTPUTS
	};

	enum LightIds {
		MUTE_LIGHT1,
		MUTE_LIGHT2,
		MUTE_LIGHT3,
		MUTE_LIGHT4,
		MUTE_LIGHT_MASTER,
		NUM_LIGHTS
	};
	Mixer4ch() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		reset();
	}
	void step() override;

	SchmittTrigger ch1mute;
	SchmittTrigger ch2mute;
	SchmittTrigger ch3mute;
	SchmittTrigger ch4mute;

	SchmittTrigger chMmute;

	float ch1m = false;
	float ch2m = false;
	float ch3m = false;
	float ch4m = false;
	float chMm = false;

	float mixL = 0.0f;
	float mixR = 0.0f;


 	json_t *toJson()override {
		json_t *rootJm = json_object();

		json_t *mutesJ = json_array();

			json_t *muteJ1 = json_integer((int) ch1m);
			json_t *muteJ2 = json_integer((int) ch2m);
			json_t *muteJ3 = json_integer((int) ch3m);
			json_t *muteJ4 = json_integer((int) ch4m);

			json_t *muteJ5 = json_integer((int) chMm);

			json_array_append_new(mutesJ, muteJ1);
			json_array_append_new(mutesJ, muteJ2);
			json_array_append_new(mutesJ, muteJ3);
			json_array_append_new(mutesJ, muteJ4);

			json_array_append_new(mutesJ, muteJ5);

		json_object_set_new(rootJm, "as_Mixer4Mutes", mutesJ);

		return rootJm;
	}

	void fromJson(json_t *rootJm)override {
		json_t *mutesJ = json_object_get(rootJm, "as_Mixer4Mutes");

			json_t *muteJ1 = json_array_get(mutesJ, 0);
			json_t *muteJ2 = json_array_get(mutesJ, 1);
			json_t *muteJ3 = json_array_get(mutesJ, 2);
			json_t *muteJ4 = json_array_get(mutesJ, 3);

			json_t *muteJ5 = json_array_get(mutesJ, 4);


			ch1m = !!json_integer_value(muteJ1);
			ch2m = !!json_integer_value(muteJ2);
			ch3m = !!json_integer_value(muteJ3);
			ch4m = !!json_integer_value(muteJ4);

			chMm = !!json_integer_value(muteJ5);

	}
	//PAN LEVEL
	float PanL(float balance, float cv) { // -1...+1
		float p, inp;
		inp = balance + cv / 5;
		p = M_PI * (clamp(inp, -1.0f, 1.0f) + 1) / 4;
		return ::cos(p);
	}

	float PanR(float balance , float cv) {
		float p, inp;
		inp = balance + cv / 5;
		p = M_PI * (clamp(inp, -1.0f, 1.0f) + 1) / 4;
		return ::sin(p);
	}
};

void Mixer4ch::step() {
	//MUTE BUTTONS
	if (ch1mute.process(params[CH1MUTE].value)) {
		ch1m = !ch1m;
	}
	lights[MUTE_LIGHT1].value = ch1m ? 1.0f : 0.0f;
	if (ch2mute.process(params[CH2MUTE].value)) {
		ch2m = !ch2m;
	}
	lights[MUTE_LIGHT2].value = ch2m ? 1.0f : 0.0f;
	if (ch3mute.process(params[CH3MUTE].value)) {
		ch3m = !ch3m;
	}
	lights[MUTE_LIGHT3].value = ch3m ? 1.0f : 0.0f;
	if (ch4mute.process(params[CH4MUTE].value)) {
		ch4m = !ch4m;
	}
	lights[MUTE_LIGHT4].value = ch4m ? 1.0f : 0.0f;

	if (chMmute.process(params[MASTER_MUTE].value)) {
		chMm = !chMm;
	}
	lights[MUTE_LIGHT_MASTER].value = chMm ? 1.0f : 0.0f;
	//CHANNEL RESULTS
	float ch1L =  (1-ch1m) * (inputs[CH1_INPUT].value) * params[CH1_PARAM].value * PanL(params[CH1_PAN_PARAM].value,(inputs[CH1_CV_PAN_INPUT].value))* clamp(inputs[CH1_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
	float ch1R =  (1-ch1m) * (inputs[CH1_INPUT].value) * params[CH1_PARAM].value * PanR(params[CH1_PAN_PARAM].value,(inputs[CH1_CV_PAN_INPUT].value)) * clamp(inputs[CH1_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);

	float ch2L = (1-ch2m) *(inputs[CH2_INPUT].value) * params[CH2_PARAM].value * PanL(params[CH2_PAN_PARAM].value,(inputs[CH2_CV_PAN_INPUT].value)) * clamp(inputs[CH2_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
	float ch2R = (1-ch2m) *(inputs[CH2_INPUT].value) * params[CH2_PARAM].value * PanR(params[CH2_PAN_PARAM].value,(inputs[CH2_CV_PAN_INPUT].value)) * clamp(inputs[CH2_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);

	float ch3L = (1-ch3m) *(inputs[CH3_INPUT].value) * params[CH3_PARAM].value * PanL(params[CH3_PAN_PARAM].value,(inputs[CH3_CV_PAN_INPUT].value)) * clamp(inputs[CH3_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
	float ch3R = (1-ch3m) *(inputs[CH3_INPUT].value) * params[CH3_PARAM].value * PanR(params[CH3_PAN_PARAM].value,(inputs[CH3_CV_PAN_INPUT].value)) * clamp(inputs[CH3_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);

	float ch4L = (1-ch4m) *(inputs[CH4_INPUT].value) * params[CH4_PARAM].value * PanL(params[CH4_PAN_PARAM].value,(inputs[CH4_CV_PAN_INPUT].value)) * clamp(inputs[CH4_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
	float ch4R = (1-ch4m) *(inputs[CH4_INPUT].value) * params[CH4_PARAM].value * PanR(params[CH4_PAN_PARAM].value,(inputs[CH4_CV_PAN_INPUT].value)) * clamp(inputs[CH4_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);

	if(!chMm){
		mixL = (ch1L + ch2L + ch3L +ch4L) * params[MIX_PARAM].value * clamp(inputs[MIX_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
		mixR = (ch1R + ch2R + ch3R +ch4R) * params[MIX_PARAM].value * clamp(inputs[MIX_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
		//CHECK FOR INPUT FROM ANOTHER MIXER
		if(inputs[LINK_L].active && inputs[LINK_R].active){
			mixL += inputs[LINK_L].value;
			mixR += inputs[LINK_R].value;
		}
	}else{
		mixL = 0.0f;
		mixR = 0.0f;
	}

	outputs[CH1_OUTPUT].value= ch1L+ch1R;
	outputs[CH2_OUTPUT].value= ch2L+ch2R;
	outputs[CH3_OUTPUT].value= ch3L+ch3R;
	outputs[CH4_OUTPUT].value= ch4L+ch4R;

	//check for MONO OUTPUT

	if(!outputs[MIX_OUTPUTR].active){
		outputs[MIX_OUTPUTL].value= mixL+mixR;
		outputs[MIX_OUTPUTR].value= 0.0f;
	}else{
		outputs[MIX_OUTPUTL].value= mixL;
		outputs[MIX_OUTPUTR].value= mixR;
	}

		//outputs[MIX_OUTPUTL].value= mixL;
		//outputs[MIX_OUTPUTR].value= mixR;

}


struct Mixer4chWidget : ModuleWidget
{
    Mixer4chWidget(Mixer4ch *module);
};


Mixer4chWidget::Mixer4chWidget(Mixer4ch *module) : ModuleWidget(module) {

  setPanel(SVG::load(assetPlugin(plugin, "res/4chMixer.svg")));

	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	//PAN KNOBS
 	static const float columnPos[8] = {33,73,113,153, 193, 233, 273, 313};
	static const float panPosY = 180;
	addParam(ParamWidget::create<as_KnobBlack>(Vec(columnPos[0]-5, panPosY), module, Mixer4ch::CH1_PAN_PARAM, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<as_KnobBlack>(Vec(columnPos[1]-5, panPosY), module, Mixer4ch::CH2_PAN_PARAM, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<as_KnobBlack>(Vec(columnPos[2]-5, panPosY), module, Mixer4ch::CH3_PAN_PARAM, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<as_KnobBlack>(Vec(columnPos[3]-5, panPosY), module, Mixer4ch::CH4_PAN_PARAM, -1.0f, 1.0f, 0.0f));

	//VOLUME FADERS
	static const float volPosY = 223;
	addParam(ParamWidget::create<as_FaderPot>(Vec(columnPos[0]+2, volPosY), module, Mixer4ch::CH1_PARAM, 0.0f, 1.0f, 0.8f));
	addParam(ParamWidget::create<as_FaderPot>(Vec(columnPos[1]+2, volPosY), module, Mixer4ch::CH2_PARAM, 0.0f, 1.0f, 0.8f));
	addParam(ParamWidget::create<as_FaderPot>(Vec(columnPos[2]+2, volPosY), module, Mixer4ch::CH3_PARAM, 0.0f, 1.0f, 0.8f));
	addParam(ParamWidget::create<as_FaderPot>(Vec(columnPos[3]+2, volPosY), module, Mixer4ch::CH4_PARAM, 0.0f, 1.0f, 0.8f));

	//MUTES
	static const float mutePosY = 310;
	addParam(ParamWidget::create<LEDBezel>(Vec(columnPos[0]+3, mutePosY), module, Mixer4ch::CH1MUTE , 0.0f, 1.0f, 0.0f));
  	addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(columnPos[0]+5.2, mutePosY+2), module, Mixer4ch::MUTE_LIGHT1));
	addParam(ParamWidget::create<LEDBezel>(Vec(columnPos[1]+3, mutePosY), module, Mixer4ch::CH2MUTE , 0.0f, 1.0f, 0.0f));
  	addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(columnPos[1]+5.2, mutePosY+2), module, Mixer4ch::MUTE_LIGHT2));
	addParam(ParamWidget::create<LEDBezel>(Vec(columnPos[2]+3, mutePosY), module, Mixer4ch::CH3MUTE , 0.0f, 1.0f, 0.0f));
  	addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(columnPos[2]+5.2, mutePosY+2), module, Mixer4ch::MUTE_LIGHT3));
	addParam(ParamWidget::create<LEDBezel>(Vec(columnPos[3]+3, mutePosY), module, Mixer4ch::CH4MUTE , 0.0f, 1.0f, 0.0f));
  	addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(columnPos[3]+5.2, mutePosY+2), module, Mixer4ch::MUTE_LIGHT4));

	//PORTS
 	static const float portsY[4] = {60,90,120,150};
	addInput(Port::create<as_PJ301MPort>(Vec(columnPos[0], portsY[0]), Port::INPUT, module, Mixer4ch::CH1_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(columnPos[0], portsY[1]), Port::INPUT, module, Mixer4ch::CH1_CV_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(columnPos[0], portsY[2]), Port::INPUT, module, Mixer4ch::CH1_CV_PAN_INPUT));

	addInput(Port::create<as_PJ301MPort>(Vec(columnPos[1], portsY[0]), Port::INPUT, module, Mixer4ch::CH2_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(columnPos[1], portsY[1]), Port::INPUT, module, Mixer4ch::CH2_CV_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(columnPos[1], portsY[2]), Port::INPUT, module, Mixer4ch::CH2_CV_PAN_INPUT));

	addInput(Port::create<as_PJ301MPort>(Vec(columnPos[2], portsY[0]), Port::INPUT, module, Mixer4ch::CH3_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(columnPos[2], portsY[1]), Port::INPUT, module, Mixer4ch::CH3_CV_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(columnPos[2], portsY[2]), Port::INPUT, module, Mixer4ch::CH3_CV_PAN_INPUT));

	addInput(Port::create<as_PJ301MPort>(Vec(columnPos[3], portsY[0]), Port::INPUT, module, Mixer4ch::CH4_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(columnPos[3], portsY[1]), Port::INPUT, module, Mixer4ch::CH4_CV_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(columnPos[3], portsY[2]), Port::INPUT, module, Mixer4ch::CH4_CV_PAN_INPUT));


	addOutput(Port::create<as_PJ301MPort>(Vec(columnPos[0], portsY[3]), Port::OUTPUT, module, Mixer4ch::CH1_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(columnPos[1], portsY[3]), Port::OUTPUT, module, Mixer4ch::CH2_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(columnPos[2], portsY[3]), Port::OUTPUT, module, Mixer4ch::CH3_OUTPUT));
	addOutput(Port::create<as_PJ301MPort>(Vec(columnPos[3], portsY[3]), Port::OUTPUT, module, Mixer4ch::CH4_OUTPUT));

	//OUTPUT
	static const float mstrX = 206;
	addOutput(Port::create<as_PJ301MPort>(Vec(mstrX, portsY[0]), Port::OUTPUT, module, Mixer4ch::MIX_OUTPUTL));
	addOutput(Port::create<as_PJ301MPort>(Vec(mstrX, portsY[1]), Port::OUTPUT, module, Mixer4ch::MIX_OUTPUTR));
	addInput(Port::create<as_PJ301MPort>(Vec(mstrX, portsY[3]), Port::INPUT, module, Mixer4ch::MIX_CV_INPUT));
	addParam(ParamWidget::create<as_FaderPot>(Vec(mstrX, volPosY), module, Mixer4ch::MIX_PARAM, 0.0f, 1.0f, 0.8f));
	addParam(ParamWidget::create<LEDBezel>(Vec(mstrX, mutePosY), module, Mixer4ch::MASTER_MUTE , 0.0f, 1.0f, 0.0f));
  	addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(mstrX+2.2, mutePosY+2), module, Mixer4ch::MUTE_LIGHT_MASTER));

	//LINK
	addInput(Port::create<as_PJ301MPort>(Vec(columnPos[0], 30), Port::INPUT, module, Mixer4ch::LINK_L));
	addInput(Port::create<as_PJ301MPort>(Vec(columnPos[1], 30), Port::INPUT, module, Mixer4ch::LINK_R));

}

RACK_PLUGIN_MODEL_INIT(AS, Mixer4ch) {
   Model *modelMixer4ch = Model::create<Mixer4ch, Mixer4chWidget>("AS", "Mixer4ch", "4-CH Mixer", MIXER_TAG, AMPLIFIER_TAG);
   return modelMixer4ch;
}
