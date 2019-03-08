
#include "cf.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_cf {

struct CUTS : Module {
	enum ParamIds {
		POTF_PARAM,
		POTR_PARAM,
		LINK_PARAM,
		POLE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		IN2_INPUT,
		F_INPUT,
		R_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
    enum LightIds {
		LINK_LIGHT,
		NUM_LIGHTS
	};

int poles = 4;

float prevf1[8];
float prevf2[8];
float prevf3[8];
float prevf4[8];

float delta=0.0;
float temp1=0.0;
float temp2=0.0;
float temp3=0.0;
float temp4=0.0;

float rin=0;
bool rv =false;
float fin=0;
bool fv =false;

bool LINK_STATE =false;
float link_delta;
SchmittTrigger linkTrigger;

	CUTS() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {reset();}
	void step() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();
		

		json_object_set_new(rootJ, "linkstate", json_integer(LINK_STATE));
		json_object_set_new(rootJ, "linkdelta", json_real(link_delta));
		return rootJ;
		}

void fromJson(json_t *rootJ) override {
		

		json_t *linkstateJ = json_object_get(rootJ, "linkstate");
		if (linkstateJ)
			LINK_STATE = json_integer_value(linkstateJ);

		json_t *ldJ = json_object_get(rootJ, "linkdelta");
		if (ldJ)
			link_delta = json_real_value(ldJ);
	
	}


};

struct Slide : LEDSliderBlue {

float *deltax ;
bool *visi ;
float xorigin = 0.0;

	Slide() {
		//setSVG(SVG::load(assetPlugin(plugin, "res/spiral.svg")));
	}
	void step() override{
		if (xorigin==0) xorigin=box.pos.x;
		dirty = true;

		if(*visi) {value=  *deltax; box.pos.x = xorigin;} 
		else {value =value; box.pos.x = xorigin+100;}

		LEDSliderBlue::step();
	}
	//void draw(NVGcontext *vg) override {LEDSliderBlue::draw(vg);}

};


void CUTS::step() {
		
poles = int(params[POLE_PARAM].value);

if (linkTrigger.process(params[LINK_PARAM].value))
			{if (LINK_STATE == 0) {LINK_STATE = 1; link_delta = fin-rin;} else LINK_STATE = 0;}
lights[LINK_LIGHT].value=LINK_STATE;

if (inputs[R_INPUT].active) {
			rv = true;
			rin = clamp(inputs[R_INPUT].value,0.0,10.0)/10.0;
		} else {
			rv = false;
			rin = params[POTR_PARAM].value;
		}

if (!LINK_STATE) {
	if (inputs[F_INPUT].active) {
				fv = true;
				fin = clamp(inputs[F_INPUT].value,0.0,10.0)/10.0;
			} else {
				fv = false;
				fin = params[POTF_PARAM].value;
			}
	} else {
		if (inputs[R_INPUT].active) {
				fv = true;
				fin = clamp(inputs[R_INPUT].value/10.0+link_delta,0.0f,1.0f);
			} else {
				if (inputs[F_INPUT].active) {
					rv = true;
					rin = clamp(inputs[F_INPUT].value/10-link_delta,0.0f,1.0f);
					fv = true;
					fin = clamp(inputs[F_INPUT].value/10,0.0f,1.0f);
				} else {
					fv = true;
					fin = clamp(params[POTR_PARAM].value+link_delta,0.0f,1.0f);
				}
			}
	}

if (inputs[IN_INPUT].active) {
	temp1 = inputs[IN_INPUT].value;

	for (int i=0;i<poles;i++){
		delta = temp1 - prevf1[i];
		delta = delta * fin;
		prevf1[i] = prevf1[i] + delta ;
		temp1= prevf1[i];
	}
	temp2=temp1;
	for (int i=0;i<poles;i++){
		delta = temp2 - prevf2[i];
		delta = delta * rin;
		prevf2[i] = prevf2[i] + delta ;
		temp2= prevf2[i];
	}
	outputs[OUT_OUTPUT].value = temp1 - temp2;

} else outputs[OUT_OUTPUT].value =0;


if (inputs[IN2_INPUT].active) {
	temp3 = inputs[IN2_INPUT].value;

	for (int i=0;i<poles;i++){
		delta = temp3 - prevf3[i];
		delta = delta * fin;
		prevf3[i] = prevf3[i] + delta ;
		temp3= prevf3[i];
	}
	temp4=temp3;
	for (int i=0;i<poles;i++){
		delta = temp4 - prevf4[i];
		delta = delta * rin;
		prevf4[i] = prevf4[i] + delta ;
		temp4= prevf4[i];
	}

	
	outputs[OUT2_OUTPUT].value = temp3 - temp4;

} else outputs[OUT2_OUTPUT].value =0;

}



struct CUTSWidget : ModuleWidget {
	CUTSWidget(CUTS *module);
};

CUTSWidget::CUTSWidget(CUTS *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/CUTS.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));


	addInput(Port::create<PJ301MPort>(Vec(3, 308), Port::INPUT, module, CUTS::IN_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(3, 334), Port::INPUT, module, CUTS::IN2_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(32, 308), Port::OUTPUT, module, CUTS::OUT_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(32, 334), Port::OUTPUT, module, CUTS::OUT2_OUTPUT));

	addParam(ParamWidget::create<RoundBlackSnapKnob>(Vec(15.5, 54), module, CUTS::POLE_PARAM, 1.0f, 8.0f, 4.0f));

	addParam(ParamWidget::create<LEDSliderWhite>(Vec(5, 131), module, CUTS::POTR_PARAM, 0.0f, 1.0f, 0.0f));
	addInput(Port::create<PJ301MPort>(Vec(3, 252), Port::INPUT, module, CUTS::R_INPUT));
	addParam(ParamWidget::create<LEDSliderWhite>(Vec(35, 131), module, CUTS::POTF_PARAM, 0.0f, 1.0f, 1.0f));
	addInput(Port::create<PJ301MPort>(Vec(32, 252), Port::INPUT, module, CUTS::F_INPUT));

	Slide *rslider = new Slide();
	rslider->box.pos = Vec(5,131);
	rslider->deltax = &module->rin;
	rslider->visi = &module->rv;
	addChild(rslider);

	Slide *fslider = new Slide();
	fslider->box.pos = Vec(35,131);
	fslider->deltax = &module->fin;
	fslider->visi = &module->fv;
	addChild(fslider);

     	addParam(ParamWidget::create<LEDButton>(Vec(21.5, 104), module, CUTS::LINK_PARAM, 0.0, 1.0, 0.0));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(25.9, 108.4), module, CUTS::LINK_LIGHT));

}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, CUTS) {
   Model *modelCUTS = Model::create<CUTS, CUTSWidget>("cf", "CUTS", "Cuts", FILTER_TAG);
   return modelCUTS;
}

