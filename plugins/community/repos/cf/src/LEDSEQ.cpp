#include "cf.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_cf {

struct LEDSEQ : Module {
	enum ParamIds {
		EDIT_PARAM,
		ON_PARAM,
		NUM_PARAMS = ON_PARAM + 80
	};
	enum InputIds {
		RST_INPUT,
		UP_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		TR_OUTPUT,
		NUM_OUTPUTS = TR_OUTPUT + 5
	};
    enum LightIds {
		EDIT_LIGHT,
		LED_LIGHT,
		NUM_LIGHTS = LED_LIGHT + 80
	};



int pas = 0;
bool ledState[80] = {};
int tempState[5] = {};
bool editState = false ;
SchmittTrigger rstTrigger;
SchmittTrigger upTrigger;
SchmittTrigger editTrigger;
SchmittTrigger ledTrigger[80] ={};


	LEDSEQ() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

json_t *toJson() override {
		json_t *rootJ = json_object();

		// leds
		json_t *ledsJ = json_array();
		for (int i = 0; i < 80; i++) {
			json_t *ledJ = json_integer((int) ledState[i]);
			json_array_append_new(ledsJ, ledJ);
		}
		json_object_set_new(rootJ, "leds", ledsJ);

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {

		// leds
		json_t *ledsJ = json_object_get(rootJ, "leds");
		if (ledsJ) {
			for (int i = 0; i < 80; i++) {
				json_t *ledJ = json_array_get(ledsJ, i);
				if (ledJ)
					ledState[i] = !!json_integer_value(ledJ);
			}
		}

	}

	void reset() override {
		for (int i = 0; i < 80; i++) {
			ledState[i] = false;
		}
	}

	void randomize() override {
		for (int i = 0; i < 80; i++) {
			ledState[i] = (randomUniform() > 0.5);
		}
	}

};


void LEDSEQ::step() {

	if (rstTrigger.process(inputs[RST_INPUT].value))
			{
			pas = 0;
			}

	if (upTrigger.process(inputs[UP_INPUT].value))
			{
				for (int i = 0; i < 5; i++) {
					if (ledState[(i+pas*5)%80]) tempState [i] = 50;
				}
				if (pas <15) pas = pas+1; else pas =0;
			}

	if (editTrigger.process(params[EDIT_PARAM].value))
			{
			editState = !editState ;
			lights[EDIT_LIGHT].value= editState ;
			}
	if (!editState)
		{
			for (int i = 0; i < 80; i++) {lights[LED_LIGHT +i].value=ledState[(i+pas*5)%80];}

			
				for (int i = 0; i < 80; i++) {
					if (ledTrigger[i].process(params[ON_PARAM +i].value)) {ledState[(i+pas*5)%80]=!ledState[(i+pas*5)%80];}
			};

		} else {
			for (int i = 0; i < 80; i++) {lights[LED_LIGHT +i].value=ledState[i];}
			
				for (int i = 0; i < 80; i++) {
					if (ledTrigger[i].process(params[ON_PARAM +i].value)) {ledState[i]=!ledState[i];}
				};
		}

	for (int i = 0; i < 5; i++) {
			if (tempState [i]>0) {tempState [i] = tempState [i]-1;outputs[TR_OUTPUT+i].value=10.0f;} else outputs[TR_OUTPUT+i].value=0.0f;
		}



}


struct LButton : SVGSwitch, MomentarySwitch {
	LButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/L.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Ldown.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct LEDSEQWidget : ModuleWidget {
	LEDSEQWidget(LEDSEQ *module);
};

LEDSEQWidget::LEDSEQWidget(LEDSEQ *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/LEDSEQ.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));




	for (int i = 0; i < 16; i++) {
	for (int j = 0; j < 5; j++) {
     		addParam(ParamWidget::create<LButton>(Vec(j*15+10-0.8, i*15+35-0.8+51), module, LEDSEQ::ON_PARAM + (i*5+j), 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(j*15+10, i*15+35+51), module, LEDSEQ::LED_LIGHT + (i*5+j)));
	}}
	addInput(Port::create<PJ301MPort>(Vec(4, 340), Port::INPUT, module, LEDSEQ::RST_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(60, 340), Port::INPUT, module, LEDSEQ::UP_INPUT));  

	addParam(ParamWidget::create<LEDButton>(Vec(35, 340), module, LEDSEQ::EDIT_PARAM, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(39.4, 344.4), module, LEDSEQ::EDIT_LIGHT));

	for (int i = 0; i < 5; i++) {
		addOutput(Port::create<PJ301MPort>(Vec(4+i*14, 30+ 22*(i%2)), Port::OUTPUT, module, LEDSEQ::TR_OUTPUT +i));
	}


	
	
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, LEDSEQ) {
   Model *modelLEDSEQ = Model::create<LEDSEQ, LEDSEQWidget>("cf", "LEDSEQ", "Ledseq", SEQUENCER_TAG);
   return modelLEDSEQ;
}

