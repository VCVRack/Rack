#include "cf.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_cf {

struct LEDS : Module {
	enum ParamIds {
		ON_PARAM,
		NUM_PARAMS = ON_PARAM + 100
	};
	enum InputIds {
		RND_INPUT,
		UP_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
    enum LightIds {
		LED_LIGHT,
		NUM_LIGHTS = LED_LIGHT + 100
	};


int wait = 0;
bool ledState[100] = {};
bool tempState[5] = {};
SchmittTrigger rndTrigger;
SchmittTrigger upTrigger;


	LEDS() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

json_t *toJson() override {
		json_t *rootJ = json_object();

		// leds
		json_t *ledsJ = json_array();
		for (int i = 0; i < 100; i++) {
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
			for (int i = 0; i < 100; i++) {
				json_t *ledJ = json_array_get(ledsJ, i);
				if (ledJ)
					ledState[i] = !!json_integer_value(ledJ);
			}
		}

	}

	void reset() override {
		for (int i = 0; i < 100; i++) {
			ledState[i] = false;
		}
	}

	void randomize() override {
		for (int i = 0; i < 100; i++) {
			ledState[i] = (randomUniform() > 0.5);
		}
	}

};


void LEDS::step() {

	if (rndTrigger.process(inputs[RND_INPUT].value))
			{for (int i = 0; i < 100; i++) 
				{ledState[i] = (randomUniform() > 0.5);}
			}

	if (upTrigger.process(inputs[UP_INPUT].value))
			{
			for (int i = 0; i < 5; i++) 
				{tempState[i] = ledState[i];}

			for (int i = 0; i < 95; i++) 
				{ledState[i] = ledState[i+5];}

			for (int i = 0; i < 5; i++) 
				{ledState[i+95] = tempState[i];}
			}

	if (wait == 0) {
		for (int i = 0; i < 100; i++) {
			
			if (params[ON_PARAM +i].value) {ledState[i]=!ledState[i]; wait = 20000;}
			lights[LED_LIGHT +i].value=ledState[i];
	}} else wait = wait-1;


}


struct LButton : SVGSwitch, MomentarySwitch {
	LButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/L.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Ldown.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct LEDSWidget : ModuleWidget {
	LEDSWidget(LEDS *module);
};

LEDSWidget::LEDSWidget(LEDS *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/LEDS.svg")));


	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	for (int i = 0; i < 20; i++) {
	for (int j = 0; j < 5; j++) {
     		addParam(ParamWidget::create<LButton>(Vec(j*15+10-0.8, i*15+35-0.8), module, LEDS::ON_PARAM + (i*5+j), 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(j*15+10, i*15+35), module, LEDS::LED_LIGHT + (i*5+j)));
	}}
	addInput(Port::create<PJ301MPort>(Vec(11, 340), Port::INPUT, module, LEDS::RND_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(54, 340), Port::INPUT, module, LEDS::UP_INPUT));

	
	
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, LEDS) {
   Model *modelLEDS = Model::create<LEDS, LEDSWidget>("cf", "LEDS", "Leds", VISUAL_TAG);
   return modelLEDS;
}
