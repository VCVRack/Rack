
#include "cf.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_cf {

struct DISTO : Module {
	enum ParamIds {
		FOLD_PARAM,
		GAIN_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		GAIN_INPUT,
		FOLD_INPUT,
		NUM_INPUTS
		
	};
	enum OutputIds {
		X_OUTPUT,
		NUM_OUTPUTS
	};


	float x = 0;
	float y = 0;
	int length = 0;
	float fold_gain ;
	int fold_affi ;
	float gain_gain ;
	int gain_affi ;

	DISTO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;

};



void DISTO::step() { 

	if (inputs[FOLD_INPUT].active) {
		fold_affi =true; fold_gain = clamp(inputs[FOLD_INPUT].value,-0.001,10.001) ;} 
	 else {fold_affi =false; fold_gain = params[FOLD_PARAM].value ;}

	if (inputs[GAIN_INPUT].active) {
		gain_affi =true; gain_gain = clamp(inputs[GAIN_INPUT].value,-0.001,10.001) ;} 
	 else {gain_affi =false; gain_gain = params[GAIN_PARAM].value ;}

//////////DISTO
	x=inputs[IN_INPUT].value*5.0f*gain_gain;

	if (abs(x)>5) y = clamp((abs(x)-5)/2.2f,0.0f,58.0f); else y=0;

	for (int i =0; i<100; i++) {
      if (x<-5.0f) x=-5.0f+(-x-5.0f)*fold_gain/5.0; 
      if (x>5.0f) x=5.0f-(x-5.0f)*fold_gain/5.0;
      if ((x>=-5.0) & (x<=5.0)) i=1000;
      if (i==99) x=0;
   }
	
	outputs[X_OUTPUT].value=clamp(x,-5.0f,5.0f);

}

struct cachecl : SVGScrew {
	cachecl() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/distocach.svg")));
		box.size = sw->box.size;
	}
};

struct DISTODisplay : TransparentWidget {

	float *xxxx;
	int *llll;
	float bu[5] = {};
	int ind = 0;

	DISTODisplay() {
	
		
	}
	
	void draw(NVGcontext *vg) {
		bu[ind] = *xxxx ;
		for (int i = 0 ; i<5 ; i++){
		{//nvgStrokeColor(vg, nvgRGBA(0x28, 0xb0, 0xf3, 0xff));
			nvgBeginPath(vg);
			nvgCircle(vg, 0,0, bu[i]);
			nvgFillColor(vg, nvgRGBA(0x28, 0xb0, 0xf3, 0xff));
			nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
			nvgFill(vg);
			nvgClosePath(vg);
		}
		}
		//nvgStroke(vg);
		if (ind<4) ind = ind +1; else ind = 0;
	}
};

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

struct DISTOWidget : ModuleWidget {
	DISTOWidget(DISTO *module);
};

DISTOWidget::DISTOWidget(DISTO *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/DISTO.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	{
		DISTODisplay *distdisplay = new DISTODisplay();
			distdisplay->box.pos = Vec(60, 170);
			distdisplay->xxxx = &module->y ;
			distdisplay->llll = &module->length ;	
		addChild(distdisplay);
	}

	addInput(Port::create<PJ301MPort>(Vec(15, 321), Port::INPUT, module, DISTO::IN_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(47, 321), Port::INPUT, module, DISTO::GAIN_INPUT));
	addParam(ParamWidget::create<Trimpot>(Vec(50.4, 284), module, DISTO::GAIN_PARAM, 0.0f, 10.0f, 0.2f));
	{
		MOTORPOTDisplay *gaindisplay = new MOTORPOTDisplay();
		gaindisplay->box.pos = Vec(59.8, 293.2);
		gaindisplay->d = 9.3;
		gaindisplay->gainX = &module->gain_gain;
		gaindisplay->affich = &module->gain_affi;
		addChild(gaindisplay);
	}
	addInput(Port::create<PJ301MPort>(Vec(80, 321), Port::INPUT, module, DISTO::FOLD_INPUT));
	addParam(ParamWidget::create<Trimpot>(Vec(83.4, 284), module, DISTO::FOLD_PARAM, 0.0f, 10.0f, 0.0f)); 
	{
		MOTORPOTDisplay *folddisplay = new MOTORPOTDisplay();
		folddisplay->box.pos = Vec(92.8, 293.2);
		folddisplay->d = 9.3;
		folddisplay->gainX = &module->fold_gain;
		folddisplay->affich = &module->fold_affi;
		addChild(folddisplay);
	}
	addOutput(Port::create<PJ301MPort>(Vec(80, 31), Port::OUTPUT, module, DISTO::X_OUTPUT)); 
		addChild(Widget::create<cachecl>(Vec(0, 0)));
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, DISTO) {
   Model *modelDISTO = Model::create<DISTO, DISTOWidget>("cf", "DISTO", "Disto", DISTORTION_TAG);
   return modelDISTO;
}
