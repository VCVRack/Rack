#include "cf.hpp"
#include "dsp/digital.hpp"
//#include "cmath"




using namespace std;

namespace rack_plugin_cf {

struct VARIABLE : Module {
	enum ParamIds {
		PREV_PARAM,
		NEXT_PARAM,
		HOLD_PARAM,
		VARIABLE_PARAM,
		NUM_PARAMS 
	};
	enum InputIds {
		IN_INPUT,
		TRIG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		HOLD_LIGHT,
		NUM_LIGHTS
	};
	

	bool lock = false ;
	bool plugged = false ;
	float value = 0;
	SchmittTrigger trigTrigger;
	SchmittTrigger holdTrigger;
	SchmittTrigger nextTrigger;
	SchmittTrigger prevTrigger;

VARIABLE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) { }

	void step() override;
	
json_t *toJson() override {
		json_t *rootJ = json_object();
		
		json_object_set_new(rootJ, "loc", json_integer(lock));
		json_object_set_new(rootJ, "plu", json_integer(plugged));
		json_object_set_new(rootJ, "val", json_real(value));
		return rootJ;
		}

void fromJson(json_t *rootJ) override {
		
		json_t *locJ = json_object_get(rootJ, "loc");
		if (locJ)
			lock = json_integer_value(locJ);

		json_t *pluJ = json_object_get(rootJ, "plu");
		if (pluJ)
			plugged = json_integer_value(pluJ);

		json_t *valJ = json_object_get(rootJ, "val");
		if (valJ)
			value = json_real_value(valJ);
	
	}
};



void VARIABLE::step() {

	if (inputs[IN_INPUT].active & !plugged) {plugged = true; lock = false;}
	if (!inputs[IN_INPUT].active) {plugged = false;}

	if (inputs[IN_INPUT].active & !lock) value = inputs[IN_INPUT].value;


		if ( ( holdTrigger.process(params[HOLD_PARAM].value) || trigTrigger.process(inputs[TRIG_INPUT].value) ) & inputs[IN_INPUT].active) 
			{
			value = inputs[IN_INPUT].value;
			lock = true;
			}
	

		if (nextTrigger.process(params[NEXT_PARAM].value))
			{
			if ((value<0)&(value!=int(value))) value = int(value) ; else value = int(value+1);
			}
				
			
		if (prevTrigger.process(params[PREV_PARAM].value))
			{
			if ((value>=0)&(value!=int(value))) value = int(value) ; else value = int(value-1);
			} 

		
	lights[HOLD_LIGHT].value = lock ;
	outputs[OUT_OUTPUT].value = value ;
		
}

struct upButton : SVGSwitch, MomentarySwitch {
	upButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/upButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/upButtonDown.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};
struct downButton : SVGSwitch, MomentarySwitch {
	downButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/downButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/downButtonDown.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct VARIABLEDisplay : TransparentWidget {
	VARIABLE *module;

	int frame = 0;
	shared_ptr<Font> font;

	VARIABLEDisplay() {
		font = Font::load(assetPlugin(plugin, "res/LEDCalculator.ttf"));
	}
	
	void draw(NVGcontext *vg) override {
		std::string to_display = "";
		std::string fileDesc = "";
		if (module->value>=0) 
			fileDesc = "+" + std::to_string(module->value); else fileDesc = std::to_string(module->value);
		for (int i=0; i<9; i++) to_display = to_display + fileDesc[i];
		nvgFontSize(vg, 24);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 0);
		nvgFillColor(vg, nvgRGBA(0x4c, 0xc7, 0xf3, 0xff));
		nvgRotate(vg, -M_PI / 2.0f);	
		nvgTextBox(vg, 5, 5,350, to_display.c_str(), NULL);
	}
};


struct VARIABLEWidget : ModuleWidget {
	VARIABLEWidget(VARIABLE *module);

};

VARIABLEWidget::VARIABLEWidget(VARIABLE *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/VARIABLE.svg")));

	
	{
		VARIABLEDisplay *gdisplay = new VARIABLEDisplay();
		gdisplay->module = module;
		gdisplay->box.pos = Vec(18, 268);
		gdisplay->box.size = Vec(130, 250);
		addChild(gdisplay);
	}


	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	addInput(Port::create<PJ301MPort>(Vec(3, 31), Port::INPUT, module, VARIABLE::IN_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(3, 96), Port::INPUT, module, VARIABLE::TRIG_INPUT));
	addParam(ParamWidget::create<LEDButton>(Vec(6, 66+3), module, VARIABLE::HOLD_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(6+4.4, 69+4.4), module, VARIABLE::HOLD_LIGHT));

	addOutput(Port::create<PJ301MPort>(Vec(3, 321), Port::OUTPUT, module, VARIABLE::OUT_OUTPUT));

	addParam(ParamWidget::create<upButton>(Vec(6, 296+2), module, VARIABLE::PREV_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<downButton>(Vec(6, 276+2), module, VARIABLE::NEXT_PARAM, 0.0f, 1.0f, 0.0f));
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, VARIABLE) {
   Model *modelVARIABLE = Model::create<VARIABLE, VARIABLEWidget>("cf", "VARIABLE", "Variable", UTILITY_TAG);
   return modelVARIABLE;
}
