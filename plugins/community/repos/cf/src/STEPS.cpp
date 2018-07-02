#include "cf.hpp"

namespace rack_plugin_cf {

struct STEPS : Module {
	enum ParamIds {
		LEVEL1_PARAM,
		TRIM1_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		LIN1_INPUT,
		IN1_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		NUM_OUTPUTS
	};
int max_steps = 8 ;

	STEPS() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;
};


void STEPS::step() {


if (inputs[LIN1_INPUT].active) 
	{
	max_steps = round(clamp(params[LEVEL1_PARAM].value + inputs[LIN1_INPUT].value*0.32*params[TRIM1_PARAM].value,1.0f,32.0f));
	outputs[OUT1_OUTPUT].value = floor((inputs[IN1_INPUT].value * round(clamp(params[LEVEL1_PARAM].value + inputs[LIN1_INPUT].value*0.32*params[TRIM1_PARAM].value,1.0f,32.0f))) / 10.01) * (10 / round(clamp(params[LEVEL1_PARAM].value + inputs[LIN1_INPUT].value*0.32*params[TRIM1_PARAM].value,1.0f,32.0f))) ;
	} 
	else 
	{
	max_steps = round(params[LEVEL1_PARAM].value);
	outputs[OUT1_OUTPUT].value = floor((inputs[IN1_INPUT].value * round(params[LEVEL1_PARAM].value)) / 10.01) * (10 / round(params[LEVEL1_PARAM].value)) ; 
	}
}

struct NumbeDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  NumbeDisplayWidget() {
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



struct STEPSWidget : ModuleWidget {
	STEPSWidget(STEPS *module);
};

STEPSWidget::STEPSWidget(STEPS *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/STEPS.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(27, 157), module, STEPS::LEVEL1_PARAM, 1.0f, 32.0f, 8.1f));
	addParam(ParamWidget::create<Trimpot>(Vec(37, 207), module, STEPS::TRIM1_PARAM, -10.0f, 10.0f, 0.0f));

	addInput(Port::create<PJ301MPort>(Vec(34, 250), Port::INPUT, module, STEPS::LIN1_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(11, 321), Port::INPUT, module, STEPS::IN1_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(54, 321), Port::OUTPUT, module, STEPS::OUT1_OUTPUT));

NumbeDisplayWidget *display = new NumbeDisplayWidget();
	display->box.pos = Vec(20,56);
	display->box.size = Vec(50, 20);
	display->value = &module->max_steps;
	addChild(display);

	
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, STEPS) {
   Model *modelSTEPS = Model::create<STEPS, STEPSWidget>("cf", "STEPS", "Steps", UTILITY_TAG);
   return modelSTEPS;
}
