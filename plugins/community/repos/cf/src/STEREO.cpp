#include "cf.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_cf {

struct STEREO : Module {
	enum ParamIds {
		PAN_PARAM,
      GAIN_PARAM,
      SOLO_PARAM,
		ON_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		SOLOT_INPUT,
		ONT_INPUT,
		PAN_INPUT,
		GAIN_INPUT,
		EXTSOLO_INPUT,
		LEFT_INPUT,
		RIGHT_INPUT,
		PANPOT_INPUT,
		IN1_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		EXTSOLO_OUTPUT,
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		TLEFT_OUTPUT,
		TRIGHT_OUTPUT,
		NUM_OUTPUTS
	};
    enum LightIds {
		SOLO_LIGHT,
		ON_LIGHT,
		LEVEL_LIGHTS,
		NUM_LIGHTS = LEVEL_LIGHTS +11
	};


   float SIGNAL1 = 0.0 ;
   float SIGNAL2 = 0.0 ;
   bool ON_STATE = false ;
   bool SOLO_STATE = false ;
   bool soloed = false;
   int lightState[11] = {};
   int cligno =0;
   int retard =0;
   int retard2 =0;
   SchmittTrigger onTrigger;
   SchmittTrigger oninTrigger;
   SchmittTrigger soloTrigger;
   SchmittTrigger soloinTrigger;
   float or_gain ;
   int or_affi ;
   float orp_gain ;
   int orp_affi ;

	STEREO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {onReset();}
	void step() override;

void onReset() override {
			ON_STATE = true;
			SOLO_STATE = false;
			}

json_t *toJson() override {
		json_t *rootJ = json_object();
		// solo
		json_object_set_new(rootJ, "solostate", json_integer(SOLO_STATE));

		// solo
		json_object_set_new(rootJ, "onstate", json_integer(ON_STATE));
		return rootJ;
		}

void fromJson(json_t *rootJ) override {
		// solo
		json_t *solostateJ = json_object_get(rootJ, "solostate");
		if (solostateJ)
			SOLO_STATE = json_integer_value(solostateJ);

		// solo
		json_t *onstateJ = json_object_get(rootJ, "onstate");
		if (onstateJ)
			ON_STATE = json_integer_value(onstateJ);
	
	}

};


void STEREO::step() {

        SIGNAL1 = inputs[IN1_INPUT].value ;
	SIGNAL2 = inputs[IN2_INPUT].value ;

	if (!inputs[GAIN_INPUT].active) {
		SIGNAL1 = SIGNAL1 * params[GAIN_PARAM].value/5.0 ;
		SIGNAL2 = SIGNAL2 * params[GAIN_PARAM].value/5.0 ;
		or_affi =0;
		}
		else {
		SIGNAL1 = SIGNAL1 * clamp(inputs[GAIN_INPUT].value/5.0,0.0f,2.0f) ;
		SIGNAL2 = SIGNAL2 * clamp(inputs[GAIN_INPUT].value/5.0,0.0f,2.0f) ;
		or_affi=1;or_gain=clamp(inputs[GAIN_INPUT].value,0.0f,10.0f);
		}

	if (onTrigger.process(params[ON_PARAM].value)+oninTrigger.process(inputs[ONT_INPUT].value))
			{if (ON_STATE == 0) ON_STATE = 1; else ON_STATE = 0;}

	if (inputs[EXTSOLO_INPUT].value == 0) soloed = 0;
	if (inputs[EXTSOLO_INPUT].value == 10) soloed = 1;

	if (soloTrigger.process(params[SOLO_PARAM].value)+soloinTrigger.process(inputs[SOLOT_INPUT].value))
			{if (SOLO_STATE == 0) {SOLO_STATE = 1;} else {SOLO_STATE = 0;soloed=0;}}

// // #define and &&
	if ((!SOLO_STATE && !soloed) && (retard > 0)) retard = 0; else if (retard < 1000) retard = retard + 1;
// // #undef and


	outputs[EXTSOLO_OUTPUT].value=round(10*retard/1000);

	if (!SOLO_STATE) {SIGNAL1 = SIGNAL1 * ON_STATE ; SIGNAL2 = SIGNAL2 * ON_STATE ;}
// // #define and &&
	if (soloed && !SOLO_STATE) {SIGNAL1 = 0; SIGNAL2 = 0;}
// // #undef and
	

	if (!inputs[PAN_INPUT].active) {
			outputs[LEFT_OUTPUT].value = inputs[LEFT_INPUT].value + SIGNAL1*(1-clamp(params[PAN_PARAM].value,0.0f,1.0f));
			outputs[RIGHT_OUTPUT].value = inputs[RIGHT_INPUT].value + SIGNAL2*(1-clamp(-params[PAN_PARAM].value,0.0f,1.0f));
			outputs[TLEFT_OUTPUT].value = SIGNAL1*(1-clamp(params[PAN_PARAM].value,0.0f,1.0f));
			outputs[TRIGHT_OUTPUT].value = SIGNAL2*(1-clamp(-params[PAN_PARAM].value,0.0f,1.0f));
			orp_affi = 0;
		} else {
			outputs[LEFT_OUTPUT].value = inputs[LEFT_INPUT].value + SIGNAL1*(1-(clamp(inputs[PAN_INPUT].value,5.0f,10.0f)-5)/5);
			outputs[RIGHT_OUTPUT].value = inputs[RIGHT_INPUT].value + SIGNAL2*(1-(clamp(inputs[PAN_INPUT].value,0.0f,5.0f)+5)/5);
			outputs[TLEFT_OUTPUT].value = SIGNAL1*(1-(clamp(inputs[PAN_INPUT].value,5.0f,10.0f)-5)/5);
			outputs[TRIGHT_OUTPUT].value = SIGNAL2*(1-(clamp(inputs[PAN_INPUT].value,0.0f,5.0f)+5)/5);
			orp_affi = 1;orp_gain = clamp(inputs[PAN_INPUT].value,0.0f,10.0f);
		}

	if (ON_STATE==1) lights[ON_LIGHT].value=true; else lights[ON_LIGHT].value=false;
	
	if (SOLO_STATE==1) {if (cligno == 0) cligno =20000; else cligno=cligno-1;} else cligno = 0;
	if (cligno>5000) lights[SOLO_LIGHT].value =1; else lights[SOLO_LIGHT].value =0;

	for (int i = 0; i < 11; i++) {
#ifdef _MSC_VER
#define sMAX(a,b) (((a)>(b))?(a):(b))
		if (sMAX(SIGNAL1,SIGNAL2)> i) {if (i<10) lightState[i]=5000;else lightState[i]=20000;}
#else
		if (std::max(SIGNAL1,SIGNAL2)> i) {if (i<10) lightState[i]=5000;else lightState[i]=20000;}
#endif
	}
	for (int i = 0; i < 11; i++) {
		if (lightState[i]> 0) {lightState[i]=lightState[i]-1;lights[LEVEL_LIGHTS + i].value=true;} else lights[LEVEL_LIGHTS + i].value=false;
	}
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


struct STEREOWidget : ModuleWidget {
	STEREOWidget(STEREO *module);
};

STEREOWidget::STEREOWidget(STEREO *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/STEREO.svg")));


	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));


	addParam(ParamWidget::create<Trimpot>(Vec(38, 127), module, STEREO::PAN_PARAM, -1.0f, 1.0f, 0.0f));
	addInput(Port::create<PJ301MPort>(Vec(11, 131), Port::INPUT, module, STEREO::PAN_INPUT));
	{
		MOTORPOTDisplay *pdisplay = new MOTORPOTDisplay();
		pdisplay->box.pos = Vec(47, 136);
		pdisplay->d = 9.2;
		pdisplay->gainX = &module->orp_gain;
		pdisplay->affich = &module->orp_affi;
		addChild(pdisplay);
	}

    	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(27, 247), module, STEREO::GAIN_PARAM, 0.0f, 10.0f, 5.0f));
	addInput(Port::create<PJ301MPort>(Vec(11, 281), Port::INPUT, module, STEREO::GAIN_INPUT));
	{
		MOTORPOTDisplay *display = new MOTORPOTDisplay();
		display->box.pos = Vec(46, 266);
		display->d = 19.1;
		display->gainX = &module->or_gain;
		display->affich = &module->or_affi;
		addChild(display);
	}


   	addParam(ParamWidget::create<LEDButton>(Vec(38, 167), module, STEREO::SOLO_PARAM, 0.0f, 1.0f, 0.0f));
 	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(42.4, 171.4), module, STEREO::SOLO_LIGHT));
	addInput(Port::create<PJ301MPort>(Vec(11, 171), Port::INPUT, module, STEREO::SOLOT_INPUT));

     	addParam(ParamWidget::create<LEDButton>(Vec(38, 208), module, STEREO::ON_PARAM, 0.0f, 1.0f, 0.0f));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(42.4, 212.4), module, STEREO::ON_LIGHT));
	addInput(Port::create<PJ301MPort>(Vec(11, 211), Port::INPUT, module, STEREO::ONT_INPUT));
    

	addInput(Port::create<PJ301MPort>(Vec(11, 308), Port::INPUT, module, STEREO::IN1_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(11, 334), Port::INPUT, module, STEREO::IN2_INPUT));
	
	addOutput(Port::create<PJ301MPort>(Vec(54, 31), Port::OUTPUT, module, STEREO::EXTSOLO_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(54, 61), Port::OUTPUT, module, STEREO::LEFT_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(54, 91), Port::OUTPUT, module, STEREO::RIGHT_OUTPUT));

	addInput(Port::create<PJ301MPort>(Vec(11, 31), Port::INPUT, module, STEREO::EXTSOLO_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(11, 61), Port::INPUT, module, STEREO::LEFT_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(11, 91), Port::INPUT, module, STEREO::RIGHT_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(54, 308), Port::OUTPUT, module, STEREO::TLEFT_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(54, 334), Port::OUTPUT, module, STEREO::TRIGHT_OUTPUT));

	for (int i = 0; i < 11; i++) {
		if (i<10) addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(70, 242-i*12), module, STEREO::LEVEL_LIGHTS + i));
			else addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(70, 242-i*12), module, STEREO::LEVEL_LIGHTS + i));
	}
	
	
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, STEREO) {
   Model *modelSTEREO = Model::create<STEREO, STEREOWidget>("cf", "STEREO", "Stereo", MIXER_TAG);
   return modelSTEREO;
}
