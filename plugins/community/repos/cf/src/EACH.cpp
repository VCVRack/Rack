#include "cf.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_cf {

struct EACH : Module {
	enum ParamIds {
		DIV_PARAM,
		BEAT_PARAM,
		ON_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		DOUZE_INPUT,
		START_INPUT,
		ON_INPUT,
		DIV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		DOUZE_OUTPUT,
		RESET_OUTPUT,
		BEAT_OUTPUT,
		START_OUTPUT,
		NUM_OUTPUTS
	};
    enum LightIds {
		ON_LIGHT,
		BEAT_LIGHT,
		NUM_LIGHTS
	};

int max_EACH = 3 ;
int stepa = 0 ;
int lum = 0 ;
int note = 0;
SchmittTrigger stTrigger;
SchmittTrigger dzTrigger;
float or_gain ;
int or_affi ;


	EACH() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void EACH::step() {
	if (!inputs[DIV_INPUT].active) {
		max_EACH = floor(params[DIV_PARAM].value);
		or_affi=0;
	} else {
		max_EACH = round(clamp((inputs[DIV_INPUT].value * 1.2)+1,1.0f,12.0f));
		or_gain = round(clamp(inputs[DIV_INPUT].value,0.0f,10.0f));
		or_affi=1;
	}

	if (inputs[START_INPUT].active) {
		outputs[START_OUTPUT].value = inputs[START_INPUT].value;
		outputs[RESET_OUTPUT].value = inputs[START_INPUT].value;
		if (dzTrigger.process(inputs[START_INPUT].value)) stepa = max_EACH-1 ;
	}

	if (stTrigger.process(inputs[DOUZE_INPUT].value)) stepa = stepa +1 ;

	if (inputs[DOUZE_INPUT].active) {
		
		if (stepa == max_EACH) {
			note = 5;
			stepa = 0; 
			lum = 2000;
			}
		outputs[DOUZE_OUTPUT].value = inputs[DOUZE_INPUT].value;
	} 
	if (note >0) {outputs[BEAT_OUTPUT].value = 10.f;note = note -1;} else outputs[BEAT_OUTPUT].value = 0.f;
	if (lum>0) {lights[BEAT_LIGHT].value = true;lum = lum -1;} else lights[BEAT_LIGHT].value = false;
}

struct NuDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  NuDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };

  void draw(NVGcontext *vg) {
    // Background
    NVGcolor backgroundColor = nvgRGB(0x44, 0x44, 0x44);
    NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);
    nvgStrokeWidth(vg, 1.0);
    nvgStrokeColor(vg, borderColor);
    nvgStroke(vg);

    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::string to_display = std::to_string(*value);


    while(to_display.length()<3) to_display = ' ' + to_display;

    Vec textPos = Vec(6.0f, 17.0f);

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);

    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\\\", NULL);


    textColor = nvgRGB(0x28, 0xb0, 0xf3);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.c_str(), NULL);
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


struct EACHWidget : ModuleWidget {
	EACHWidget(EACH *module);
};

EACHWidget::EACHWidget(EACH *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/EACH.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	addInput(Port::create<PJ301MPort>(Vec(11, 26), Port::INPUT, module, EACH::START_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(35, 275), Port::OUTPUT, module, EACH::RESET_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(11, 321), Port::OUTPUT, module, EACH::START_OUTPUT));

	addInput(Port::create<PJ301MPort>(Vec(54, 26), Port::INPUT, module, EACH::DOUZE_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(54, 321), Port::OUTPUT, module, EACH::DOUZE_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(35, 235), Port::OUTPUT, module, EACH::BEAT_OUTPUT));

	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(27, 107), module, EACH::DIV_PARAM, 1.0f, 12.1f, 3.1f));
	addInput(Port::create<PJ301MPort>(Vec(11, 141), Port::INPUT, module, EACH::DIV_INPUT));
	{
		MOTORPOTDisplay *display = new MOTORPOTDisplay();
		display->box.pos = Vec(46, 126);
		display->d = 19.1;
		display->gainX = &module->or_gain;
		display->affich = &module->or_affi;
		addChild(display);
	}

     	addParam(ParamWidget::create<LEDButton>(Vec(38, 197), module, EACH::BEAT_PARAM, 0.0f, 1.0f, 0.0f));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(42.4, 201.4), module, EACH::BEAT_LIGHT));
	
	
	NuDisplayWidget *display = new NuDisplayWidget();
	display->box.pos = Vec(20,56);
	display->box.size = Vec(50, 20);
	display->value = &module->max_EACH;
	addChild(display);

	
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, EACH) {
   Model *modelEACH = Model::create<EACH, EACHWidget>("cf", "EACH", "Each", CLOCK_TAG);
   return modelEACH;
}
