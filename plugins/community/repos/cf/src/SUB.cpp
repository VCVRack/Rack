#include "cf.hpp"
#include "dsp/digital.hpp"


namespace rack_plugin_cf {

struct SUB : Module {
	enum ParamIds {
      GAIN_PARAM,
		GAIN2_PARAM,
      LINK_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		GAIN_INPUT,
		GAIN2_INPUT,
		M1_INPUT,
		M2_INPUT,
		IN1_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		M1_OUTPUT,
		OUT1_OUTPUT,
		M2_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
    enum LightIds {
		LINK_LIGHT,
		NUM_LIGHTS
	};


   float SIGNAL = 0.0 ;
   float SIGNAL2 = 0.0 ;
   float or_gain ;
   float or2_gain ;
   int or_affi ;
   int or2_affi ;
   bool LINK_STATE = false ;
   SchmittTrigger linkTrigger;


	SUB() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {reset();}
	void step() override;

   void reset() override {
      LINK_STATE = false;
   }
   
   json_t *toJson() override {
		json_t *rootJ = json_object();
		// solo
		json_object_set_new(rootJ, "linkstate", json_integer(LINK_STATE));

		return rootJ;
		}

void fromJson(json_t *rootJ) override {
		// solo
		json_t *linkstateJ = json_object_get(rootJ, "linkstate");
		if (linkstateJ)
			LINK_STATE = json_integer_value(linkstateJ);

	
	}

};


void SUB::step() {

	if (linkTrigger.process(params[LINK_PARAM].value))
			{LINK_STATE=!LINK_STATE;}
	lights[LINK_LIGHT].value=LINK_STATE;

   
   SIGNAL = inputs[IN1_INPUT].value ;

	outputs[OUT1_OUTPUT].value = SIGNAL;
   
	if (!inputs[GAIN_INPUT].active)
   {SIGNAL = SIGNAL * params[GAIN_PARAM].value/10.0 ;or_affi=0;}
   else {SIGNAL = SIGNAL * clamp(inputs[GAIN_INPUT].value/10.0f,0.0f,1.0f) ; or_affi=1;or_gain=clamp(inputs[GAIN_INPUT].value,0.0f,10.0f);}

	outputs[M1_OUTPUT].value = inputs[M1_INPUT].value + SIGNAL;


   SIGNAL2 = inputs[IN2_INPUT].value ;
   
	outputs[OUT2_OUTPUT].value = SIGNAL2;
   
	if (!LINK_STATE) {
		if (!inputs[GAIN2_INPUT].active) 
			{SIGNAL2 = SIGNAL2 * params[GAIN2_PARAM].value/10.0 ;or2_affi=0;}
      else {SIGNAL2 = SIGNAL2 * clamp(inputs[GAIN2_INPUT].value/10.0f,0.0f,1.0f) ; or2_affi=1;or2_gain=clamp(inputs[GAIN2_INPUT].value,0.0f,10.0f);}
	} else {
		if (!inputs[GAIN_INPUT].active)
			{SIGNAL2 = SIGNAL2 * params[GAIN_PARAM].value/10.0 ;or2_affi=1;or2_gain=clamp(params[GAIN_PARAM].value,0.0f,10.0f);}
		else {SIGNAL2 = SIGNAL2 * clamp(inputs[GAIN_INPUT].value/10.0f,0.0f,1.0f) ; or2_affi=1;or2_gain=clamp(inputs[GAIN_INPUT].value,0.0f,10.0f);}
	}

	outputs[M2_OUTPUT].value = inputs[M2_INPUT].value + SIGNAL2;


}

struct MOTORPOTDisplay : TransparentWidget {

	float d;
	float *gainX ;
	int *affich;

	MOTORPOTDisplay() {
		
	}
	
	void draw(NVGcontext *vg) {
		if (*affich==1) {
		float xx = d*sin(-(*gainX*0.17+0.15)*M_PI) ;
		float yy = d*cos((*gainX*0.17+0.15)*M_PI) ;

		
			nvgBeginPath(vg);
			nvgCircle(vg, 0,0, d);
			nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
			nvgFill(vg);	
		
			nvgStrokeWidth(vg,1.2);
			nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0xff));
			{
				nvgBeginPath(vg);
				nvgMoveTo(vg, 0,0);
				nvgLineTo(vg, xx,yy);
				nvgClosePath(vg);
			}
			nvgStroke(vg);
		}

	}
};


struct SUBWidget : ModuleWidget {
	SUBWidget(SUB *module);
};

SUBWidget::SUBWidget(SUB *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/SUB.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

   addParam(ParamWidget::create<LEDButton>(Vec(22, 179), module, SUB::LINK_PARAM, 0.0, 1.0, 0.0));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(26.5, 182.5), module, SUB::LINK_LIGHT));


   addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(27, 247), module, SUB::GAIN2_PARAM, 0.0, 10.0, 0.0));
	addInput(Port::create<PJ301MPort>(Vec(11, 281), Port::INPUT, module, SUB::GAIN2_INPUT));
	{
		MOTORPOTDisplay *display2 = new MOTORPOTDisplay();
		display2->box.pos = Vec(46, 266);
		display2->d = 19.1;
		display2->gainX = &module->or2_gain;
		display2->affich = &module->or2_affi;
		addChild(display2);
	}

    

	addInput(Port::create<PJ301MPort>(Vec(11, 321), Port::INPUT, module, SUB::IN2_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(54, 321), Port::OUTPUT, module, SUB::OUT2_OUTPUT));
	

	addOutput(Port::create<PJ301MPort>(Vec(54, 61+152), Port::OUTPUT, module, SUB::M2_OUTPUT));


	addInput(Port::create<PJ301MPort>(Vec(11, 61+152), Port::INPUT, module, SUB::M2_INPUT));


   addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(27, 247-182), module, SUB::GAIN_PARAM, 0.0, 10.0, 0.0));
	addInput(Port::create<PJ301MPort>(Vec(11, 281-182), Port::INPUT, module, SUB::GAIN_INPUT));
	{
		MOTORPOTDisplay *display = new MOTORPOTDisplay();
		display->box.pos = Vec(46, 266-182);
		display->d = 19.1;
		display->gainX = &module->or_gain;
		display->affich = &module->or_affi;
		addChild(display);
	}

    
	addInput(Port::create<PJ301MPort>(Vec(11, 321-182), Port::INPUT, module, SUB::IN1_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(54, 321-182), Port::OUTPUT, module, SUB::OUT1_OUTPUT));
	
	addOutput(Port::create<PJ301MPort>(Vec(54, 31), Port::OUTPUT, module, SUB::M1_OUTPUT));

	addInput(Port::create<PJ301MPort>(Vec(11, 31), Port::INPUT, module, SUB::M1_INPUT));
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, SUB) {
   Model *modelSUB = Model::create<SUB, SUBWidget>("cf", "SUB", "Sub", MIXER_TAG);
   return modelSUB;
}
