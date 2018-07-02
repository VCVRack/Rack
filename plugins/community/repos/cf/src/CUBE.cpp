
#include "cf.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_cf {

struct CUBE : Module {
	enum ParamIds {
		
		NUM_PARAMS
	};
	enum InputIds {
		X_INPUT,
		Y_INPUT,
		NUM_INPUTS
		
	};
	enum OutputIds {
		X_OUTPUT,
		NUM_OUTPUTS
	};

	float frameX = 0.0;
	float frameY = 0.0;

	float xx[12] = {-1.0, 1.0, 1.0,-1.0,-1.0, 1.0, 1.0,-1.0};
	float yy[12] = {-1.0,-1.0, 1.0, 1.0,-1.0,-1.0, 1.0, 1.0};
	float zz[12] = {-1.0,-1.0,-1.0,-1.0, 1.0, 1.0, 1.0, 1.0};

	float x[12] = {};
	float y[12] = {};
	float z[12] = {};

	float d = 0.0;
	float theta= 0.0 ;
	float gainX = 1.0;
	float gainY = 1.0;
	

	CUBE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;

};



void CUBE::step() { 
	gainX = 0.5f; gainY = 0.5f;
	if (inputs[X_INPUT].active) gainX=inputs[X_INPUT].value;
	if (inputs[Y_INPUT].active) gainY=inputs[Y_INPUT].value;

       	for(int i=0; i<12; i++)
        	{
			d = sqrt(yy[i]*yy[i] + zz[i]*zz[i]);
			theta = atan2(yy[i],zz[i])+frameX;
			x[i] = xx[i]; 
			y[i] = d * sin(theta); 
			z[i] = d * cos(theta);

			d = sqrt(x[i]*x[i] + z[i]*z[i]);
			theta = atan2(x[i],z[i])+frameY;
			x[i] = d * sin(theta); 
			y[i] = y[i]; 
			z[i] = d * cos(theta);
        	}
		
	if (frameX<100) frameX=frameX+gainX/engineGetSampleRate(); else frameX=0;
	if (frameY<100) frameY=frameY+gainY/engineGetSampleRate(); else frameY=0;


	outputs[X_OUTPUT].value=z[0]*5.0;
}

struct CUBEDisplay : TransparentWidget {

	float *xxxx[12] = {};
	float *yyyy[12] = {};

	CUBEDisplay() {
		
	}
	
	void draw(NVGcontext *vg) {

		nvgStrokeColor(vg, nvgRGBA(0x28, 0xb0, 0xf3, 0xff));
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, *xxxx[0]*20,*yyyy[0]*20);
			nvgLineTo(vg, *xxxx[1]*20,*yyyy[1]*20);
			nvgLineTo(vg, *xxxx[2]*20,*yyyy[2]*20);
			nvgLineTo(vg, *xxxx[3]*20,*yyyy[3]*20);
			nvgClosePath(vg);
		}
		nvgStroke(vg);

		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, *xxxx[4]*20,*yyyy[4]*20);
			nvgLineTo(vg, *xxxx[5]*20,*yyyy[5]*20);
			nvgLineTo(vg, *xxxx[6]*20,*yyyy[6]*20);
			nvgLineTo(vg, *xxxx[7]*20,*yyyy[7]*20);
			nvgClosePath(vg);
		}
		nvgStroke(vg);

		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, *xxxx[0]*20,*yyyy[0]*20);
			nvgLineTo(vg, *xxxx[4]*20,*yyyy[4]*20);
			nvgClosePath(vg);
		}
		nvgStroke(vg);

		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, *xxxx[1]*20,*yyyy[1]*20);
			nvgLineTo(vg, *xxxx[5]*20,*yyyy[5]*20);
			nvgClosePath(vg);
		}
		nvgStroke(vg);

		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, *xxxx[2]*20,*yyyy[2]*20);
			nvgLineTo(vg, *xxxx[6]*20,*yyyy[6]*20);
			nvgClosePath(vg);
		}
		nvgStroke(vg);

		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, *xxxx[3]*20,*yyyy[3]*20);
			nvgLineTo(vg, *xxxx[7]*20,*yyyy[7]*20);
			nvgClosePath(vg);
		}
		nvgStroke(vg);

	}
};



struct CUBEWidget : ModuleWidget {
	CUBEWidget(CUBE *module);
};

CUBEWidget::CUBEWidget(CUBE *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/CUBE.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	{
		CUBEDisplay *display = new CUBEDisplay();
		display->box.pos = Vec(60, 120);
		for (int i=0;i<12;i++) {
			display->xxxx[i] = &module->x[i] ;
			display->yyyy[i] = &module->y[i] ;	
		}
		addChild(display);
	}

	addInput(Port::create<PJ301MPort>(Vec(15, 321), Port::INPUT, module, CUBE::X_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(47, 321), Port::INPUT, module, CUBE::Y_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(80, 321), Port::OUTPUT, module, CUBE::X_OUTPUT));       
	
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, CUBE) {
   Model *modelCUBE = Model::create<CUBE, CUBEWidget>("cf", "CUBE", "Cube", LFO_TAG);
   return modelCUBE;
}
