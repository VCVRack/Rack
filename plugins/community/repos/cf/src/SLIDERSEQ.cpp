#include "cf.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_cf {

struct SLIDERSEQ : Module {
	enum ParamIds {
		OFFSET_PARAM,
		LVL_PARAM,
		ON_PARAM = LVL_PARAM +16,
		NUM_PARAMS
	};
	enum InputIds {
		RST_INPUT,
		UP_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		TR_OUTPUT,
		NUM_OUTPUTS
	};
    enum LightIds {
		OFFSET_LIGHT,
		LED_LIGHT,
		NUM_LIGHTS = LED_LIGHT + 16
	};



int pas = 0;
bool OFFSET_STATE = false ;

SchmittTrigger rstTrigger;
SchmittTrigger upTrigger;
SchmittTrigger offsetTrigger;


	SLIDERSEQ() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;



json_t *toJson() override {
		json_t *rootJ = json_object();
		

		json_object_set_new(rootJ, "offsetstate", json_integer(OFFSET_STATE));
		return rootJ;
		}

void fromJson(json_t *rootJ) override {
		

		json_t *offsetstateJ = json_object_get(rootJ, "offsetstate");
		if (offsetstateJ)
			OFFSET_STATE = json_integer_value(offsetstateJ);
	
	}


};


void SLIDERSEQ::step() {

	

	if (upTrigger.process(inputs[UP_INPUT].value))
			{
				if (pas <15) pas = pas+1; else pas =0;
			}

	if (rstTrigger.process(inputs[RST_INPUT].value))
			{
			pas = 0;
			}

	for (int i = 0; i < 16; i++)
			{
			if (i==pas) lights[LED_LIGHT +i].value=1; else lights[LED_LIGHT +i].value= 0;
			}

	if (offsetTrigger.process(params[OFFSET_PARAM].value))
			{if (OFFSET_STATE == 0) OFFSET_STATE = 1; else OFFSET_STATE = 0;}

	if (OFFSET_STATE==1) lights[OFFSET_LIGHT].value=true; else lights[OFFSET_LIGHT].value=false;

	outputs[TR_OUTPUT].value=params[LVL_PARAM +pas].value*10-OFFSET_STATE*5.0;

}


struct SLIDERSEQWidget : ModuleWidget {
	SLIDERSEQWidget(SLIDERSEQ *module);
};

SLIDERSEQWidget::SLIDERSEQWidget(SLIDERSEQ *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/SLIDERSEQ.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	addInput(Port::create<PJ301MPort>(Vec(14, 320), Port::INPUT, module, SLIDERSEQ::RST_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(43, 320), Port::INPUT, module, SLIDERSEQ::UP_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(96, 320), Port::OUTPUT, module, SLIDERSEQ::TR_OUTPUT));
	
//addParam(ParamWidget::create<CKSS>(Vec(74, 322), module, SLIDERSEQ::OFFSET_PARAM, 0.0f, 1.0f, 0.0f));

     	addParam(ParamWidget::create<LEDButton>(Vec(72, 323), module, SLIDERSEQ::OFFSET_PARAM, 0.0, 1.0, 0.0));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(76.4, 327.4), module, SLIDERSEQ::OFFSET_LIGHT));


	for (int i = 0; i < 8; i++) {
		addParam(ParamWidget::create<LEDSliderBlue>(Vec(4+i*15, 30+ 30), module, SLIDERSEQ::LVL_PARAM+i, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(10+i*15, 30+ 115), module, SLIDERSEQ::LED_LIGHT + i));
	}
	for (int i = 8; i < 16; i++) {
		addParam(ParamWidget::create<LEDSliderBlue>(Vec(4+(i-8)*15, 30+ 155), module, SLIDERSEQ::LVL_PARAM+i, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(10+(i-8)*15, 30+ 240), module, SLIDERSEQ::LED_LIGHT + i));
	}
	
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, SLIDERSEQ) {
   Model *modelSLIDERSEQ = Model::create<SLIDERSEQ, SLIDERSEQWidget>("cf", "SLIDERSEQ", "Sliderseq", SEQUENCER_TAG);
   return modelSLIDERSEQ;
}

