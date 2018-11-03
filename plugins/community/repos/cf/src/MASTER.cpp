#include "cf.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_cf {

struct MASTER : Module {
	enum ParamIds {
	        GAIN_PARAM,
		ON_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ONT_INPUT,
		GAIN_INPUT,
		LEFT_INPUT,
		RIGHT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		LEFT_MAIN_OUTPUT,
		RIGHT_MAIN_OUTPUT,
		NUM_OUTPUTS
	};
    enum LightIds {
		ON_LIGHT,
		L_LEVEL_LIGHTS,
		R_LEVEL_LIGHTS = L_LEVEL_LIGHTS +11,
		NUM_LIGHTS = R_LEVEL_LIGHTS +11
	};


float SIGNAL_LEFT = 0.0 ;
float SIGNAL_RIGHT = 0.0 ;
bool ON_STATE = false ;
int l_lightState[11] = {};
int r_lightState[11] = {};
SchmittTrigger onTrigger;
SchmittTrigger oninTrigger;


	MASTER() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {onReset();}
	void step() override;

void onReset() override {
			ON_STATE = true;
			}

json_t *toJson() override {
		json_t *rootJ = json_object();
		

		// solo
		json_object_set_new(rootJ, "onstate", json_integer(ON_STATE));
		return rootJ;
		}

void fromJson(json_t *rootJ) override {
		

		// solo
		json_t *onstateJ = json_object_get(rootJ, "onstate");
		if (onstateJ)
			ON_STATE = json_integer_value(onstateJ);
	
	}


};


void MASTER::step() {

        SIGNAL_LEFT = inputs[LEFT_INPUT].value ;
	SIGNAL_RIGHT = inputs[RIGHT_INPUT].value ;

	outputs[LEFT_OUTPUT].value =  SIGNAL_LEFT ;
	outputs[RIGHT_OUTPUT].value = SIGNAL_RIGHT ;


	if (onTrigger.process(params[ON_PARAM].value)+oninTrigger.process(inputs[ONT_INPUT].value))
			{if (ON_STATE == 0) ON_STATE = 1; else ON_STATE = 0;}

	SIGNAL_LEFT = SIGNAL_LEFT * ON_STATE * params[GAIN_PARAM].value/5.0;
	SIGNAL_RIGHT = SIGNAL_RIGHT * ON_STATE * params[GAIN_PARAM].value/5.0;

	outputs[LEFT_MAIN_OUTPUT].value =  SIGNAL_LEFT ;
	outputs[RIGHT_MAIN_OUTPUT].value = SIGNAL_RIGHT ;
	

	if (ON_STATE==1) lights[ON_LIGHT].value=true; else lights[ON_LIGHT].value=false;
	

	for (int i = 0; i < 11; i++) {
		if (SIGNAL_LEFT> i) {if (i<10) l_lightState[i]=5000;else l_lightState[i]=20000;}
	}
	for (int i = 0; i < 11; i++) {
		if (l_lightState[i]> 0) {l_lightState[i]=l_lightState[i]-1;lights[L_LEVEL_LIGHTS + i].value=true;} else lights[L_LEVEL_LIGHTS + i].value=false;
	}
	for (int i = 0; i < 11; i++) {
		if (SIGNAL_RIGHT> i) {if (i<10) r_lightState[i]=5000;else r_lightState[i]=20000;}
	}
	for (int i = 0; i < 11; i++) {
		if (r_lightState[i]> 0) {r_lightState[i]=r_lightState[i]-1;lights[R_LEVEL_LIGHTS + i].value=true;} else lights[R_LEVEL_LIGHTS + i].value=false;
	}
}


struct MASTERWidget : ModuleWidget {
	MASTERWidget(MASTER *module);
};

MASTERWidget::MASTERWidget(MASTER *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/MASTER.svg")));


	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));


    	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(27, 247), module, MASTER::GAIN_PARAM, 0.0, 10.0, 5.0));


     	addParam(ParamWidget::create<LEDButton>(Vec(38, 208), module, MASTER::ON_PARAM, 0.0, 1.0, 0.0));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(42.4, 212.4), module, MASTER::ON_LIGHT));
    
	
	addOutput(Port::create<PJ301MPort>(Vec(54, 61), Port::OUTPUT, module, MASTER::LEFT_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(54, 91), Port::OUTPUT, module, MASTER::RIGHT_OUTPUT));

	addOutput(Port::create<PJ301MPort>(Vec(54, 308), Port::OUTPUT, module, MASTER::LEFT_MAIN_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(54, 334), Port::OUTPUT, module, MASTER::RIGHT_MAIN_OUTPUT));

	addInput(Port::create<PJ301MPort>(Vec(11, 61), Port::INPUT, module, MASTER::LEFT_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(11, 91), Port::INPUT, module, MASTER::RIGHT_INPUT));


	for (int i = 0; i < 11; i++) {
		if (i<10) addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(15, 242-i*12), module, MASTER::L_LEVEL_LIGHTS + i));
			else addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(15, 242-i*12), module, MASTER::L_LEVEL_LIGHTS + i));
		if (i<10) addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(68, 242-i*12), module, MASTER::R_LEVEL_LIGHTS + i));
			else addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(68, 242-i*12), module, MASTER::R_LEVEL_LIGHTS + i));
	}
	
	
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, MASTER) {
   Model *modelMASTER = Model::create<MASTER, MASTERWidget>("cf", "MASTER", "Master", MIXER_TAG);
   return modelMASTER;
}
