#include "cf.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_cf {

struct FOUR : Module {
	enum ParamIds {
       		S_PARAM,
        	M_PARAM=S_PARAM + 4,
		NUM_PARAMS = M_PARAM + 4
	};
	enum InputIds {
		TRM_INPUT,
		TRS_INPUT=TRM_INPUT+4,
		IN_INPUT=TRS_INPUT+4,
		NUM_INPUTS=IN_INPUT+4
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS=OUT_OUTPUT+4
	};
    enum LightIds {
		M_LIGHT,
		S_LIGHT=M_LIGHT+4,
		NUM_LIGHTS=S_LIGHT+4
	};


bool muteState[8] = {};
int solo = 0;
int cligno = 0;

SchmittTrigger muteTrigger[8];
SchmittTrigger soloTrigger[8];


FOUR() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {onReset();}

void step() override;

void onReset() override {
		for (int i = 0; i < 4; i++) {
			muteState[i] = true;
			muteState[i+4] = false;
		}
		solo = 0;
		
		
	}

void onRandomize() override {
		for (int i = 0; i < 8; i++) {
			muteState[i] = (randomUniform() < 0.5);
		}
	}

json_t *toJson() override {
		json_t *rootJ = json_object();
		
		// states
		json_t *mutestatesJ = json_array();
		for (int i = 0; i < 8; i++) {
			json_t *mutestateJ = json_boolean(muteState[i]);
			json_array_append_new(mutestatesJ, mutestateJ);
			}
		json_object_set_new(rootJ, "mutestates", mutestatesJ);

		// solo
		json_object_set_new(rootJ, "solo", json_integer(solo));
		return rootJ;
		}

void fromJson(json_t *rootJ) override {
		
		// states
		json_t *mutestatesJ = json_object_get(rootJ, "mutestates");
		if (mutestatesJ) {
			for (int i = 0; i < 8; i++) {
				json_t *mutestateJ = json_array_get(mutestatesJ, i);
				if (mutestateJ)
					muteState[i] = json_boolean_value(mutestateJ);
			}
		}
		// solo
		json_t *soloJ = json_object_get(rootJ, "solo");
		if (soloJ)
			solo = json_integer_value(soloJ);
	
	}


};


void FOUR::step() {
		
	for (int i = 0; i < 4; i++) {
	
		if (soloTrigger[i].process(params[S_PARAM + i].value)+soloTrigger[i+4].process(inputs[TRS_INPUT + i].value))
			{
			muteState[i+4] ^= true;
			solo = (i+1)*muteState[i+4];
			};		
		if (solo==i+1)
		{
			float in = inputs[IN_INPUT + i].value;
			outputs[OUT_OUTPUT + i].value = in;
			
		} else {muteState[i+4] = false;lights[S_LIGHT + i].value = 0;outputs[OUT_OUTPUT + i].value = 0.0;}
		if (muteState[i+4]==true)
		{
			cligno = cligno + 1;
			if (cligno ==10000) {lights[S_LIGHT + i].value = !lights[S_LIGHT + i].value;cligno =0;}
		}		
	}

	for (int i = 0; i < 4; i++) {
		if (muteTrigger[i].process(params[M_PARAM + i].value)+muteTrigger[i+4].process(inputs[TRM_INPUT + i].value))
			muteState[i] ^= true;
		float in = inputs[IN_INPUT + i].value;
		if (solo == 0) outputs[OUT_OUTPUT + i].value = muteState[i] ? in : 0.0;
		lights[M_LIGHT + i].value = muteState[i];
	}

	
}

struct FOURWidget : ModuleWidget {
	FOURWidget(FOUR *module);
};

FOURWidget::FOURWidget(FOUR *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/FOUR.svg")));

	int y = 56;


	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	for (int i = 0; i < 4; i++) {

	  addInput(Port::create<PJ301MPort>(Vec(15, y),Port::INPUT, module, FOUR::IN_INPUT + i));

	  addInput(Port::create<PJ301MPort>(Vec(21, y+25),Port::INPUT, module, FOUR::TRS_INPUT + i));
        	addParam(ParamWidget::create<LEDButton>(Vec(45, y+4), module, FOUR::S_PARAM + i, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(45+4.4, y+8.4), module, FOUR::S_LIGHT + i));

	  addInput(Port::create<PJ301MPort>(Vec(46, y+31),Port::INPUT, module, FOUR::TRM_INPUT + i));
   		addParam(ParamWidget::create<LEDButton>(Vec(70, y+4), module, FOUR::M_PARAM + i, 0.0f, 1.0f, 0.0f));
 		addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(70+4.4, y+8.4), module, FOUR::M_LIGHT + i));

	  addOutput(Port::create<PJ301MPort>(Vec(95, y), Port::OUTPUT, module, FOUR::OUT_OUTPUT + i));

	  y = y + 75 ;
	}
	
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, FOUR) {
   Model *modelFOUR = Model::create<FOUR, FOURWidget>("cf", "FOUR", "Four", UTILITY_TAG);
   return modelFOUR;
}
