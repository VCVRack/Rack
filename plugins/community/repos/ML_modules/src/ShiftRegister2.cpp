#include "ML_modules.hpp"

#include "dsp/digital.hpp"

#include <cmath>
#include <cstdlib>

namespace rack_plugin_ML_modules {

struct ShiftRegister2 : Module {
	enum ParamIds {
		NUM_STEPS_PARAM,
		PROB1_PARAM,
		PROB2_PARAM,
		MIX1_PARAM,
		AUX_OFFSET_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		TRIGGER_INPUT,
		NUM_STEPS_INPUT,
		PROB1_INPUT,
		PROB2_INPUT,
		MIX1_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		AUX_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	ShiftRegister2() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) { reset(); };



	void step() override;

	int numSteps;
	float values[32] = {};

	SchmittTrigger trigTrigger;

	inline float randf() {return rand()/(RAND_MAX-1.0);}

	void reset() override {

	};

};




void ShiftRegister2::step() {

	numSteps = roundf(clamp(params[NUM_STEPS_PARAM].value * clamp(inputs[NUM_STEPS_INPUT].normalize(5.0f),0.0f,5.0f)/5.0f,1.0f,16.0f));


	if( inputs[TRIGGER_INPUT].active ) {

		if( trigTrigger.process(inputs[TRIGGER_INPUT].value) ) {

			float new_in1 = inputs[IN1_INPUT].normalize( randf()*10.0-5.0 );
			float new_in2 = inputs[IN2_INPUT].normalize( new_in1 + 1.0 );

			for(int i=32; i>0; i--) values[i] = values[i-1];

			float p1 = params[PROB1_PARAM].value + clamp(inputs[PROB1_INPUT].normalize(0.0f),-10.0f,10.0f)/10.0f;
			float p2 = params[PROB2_PARAM].value + clamp(inputs[PROB2_INPUT].normalize(0.0f),-10.0f,10.0f)/10.0f;

			bool replace = ( randf() < p1 );
			bool rnd2 = ( randf() < p2 );

			float a = params[MIX1_PARAM].value + clamp(inputs[MIX1_INPUT].normalize(0.0f),-10.0f,10.0f)/10.0f;


			if(replace) {
				values[0] = a* (rnd2?new_in2:new_in1) + (1-a)*values[numSteps];
			} else {
				values[0] = values[numSteps];
			};

		};

	};

	outputs[OUT_OUTPUT].value = values[0];

	int offset = roundf(params[AUX_OFFSET_PARAM].value);

	outputs[AUX_OUTPUT].value = values[offset];
};

struct IntDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  IntDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };

  void draw(NVGcontext *vg) override {
    // Background
    NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
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

//    std::string to_display = std::to_string( (unsigned) *value);

    char displayStr[3];
//    while(to_display.length()<1) to_display = ' ' + to_display;

    sprintf(displayStr, "%2u", (unsigned) *value);

    Vec textPos = Vec(6.0f, 17.0f);

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\", NULL);

    textColor = nvgRGB(0xf0, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, displayStr,  NULL);
  }
};


struct ShiftRegister2Widget : ModuleWidget {
	ShiftRegister2Widget(ShiftRegister2 *module);
};

ShiftRegister2Widget::ShiftRegister2Widget(ShiftRegister2 *module) : ModuleWidget(module) {

	box.size = Vec(15*8, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/ShiftReg2.svg")));

		addChild(panel);
	}

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 365)));




	const float column1 = 19, column2 = 74;


	IntDisplayWidget *display = new IntDisplayWidget();
	display->box.pos = Vec(65,46);
	display->box.size = Vec(40, 20);
	display->value = &module->numSteps;
	addChild(display);

	addInput(Port::create<MLPort>(Vec(column1,  44), Port::INPUT, module, ShiftRegister2::TRIGGER_INPUT));
	addInput(Port::create<MLPort>(Vec(column1, 96), Port::INPUT, module, ShiftRegister2::NUM_STEPS_INPUT));
        
	addParam(ParamWidget::create<RedSnapMLKnob>(Vec(65,  86), module, ShiftRegister2::NUM_STEPS_PARAM, 1.0, 16.0, 8.0));

	addInput(Port::create<MLPort>(Vec(column1+8,  135), Port::INPUT, module, ShiftRegister2::IN1_INPUT));
	addInput(Port::create<MLPort>(Vec(column2-8,  135), Port::INPUT, module, ShiftRegister2::IN2_INPUT));


	addInput(Port::create<MLPort>(Vec(column1+3,  183), Port::INPUT, module, ShiftRegister2::PROB1_INPUT));
    addParam(ParamWidget::create<SmallBlueMLKnob>(Vec(column2, 176), module, ShiftRegister2::PROB1_PARAM, 0.0, 1.0, 0.0));
	
	addInput(Port::create<MLPort>(Vec(column1+3,  229), Port::INPUT, module, ShiftRegister2::PROB2_INPUT));
    addParam(ParamWidget::create<SmallBlueMLKnob>(Vec(column2, 222), module, ShiftRegister2::PROB2_PARAM, 0.0, 1.0, 0.0));
	
	addInput(Port::create<MLPort>(Vec(column1+3,  275), Port::INPUT, module, ShiftRegister2::MIX1_INPUT));
	addParam(ParamWidget::create<SmallBlueMLKnob>(Vec(column2,  268), module, ShiftRegister2::MIX1_PARAM, 0.0, 1.0, 1.0));


	addParam(ParamWidget::create<Trimpot>(Vec(56,  318), module, ShiftRegister2::AUX_OFFSET_PARAM, 1.0, 16.0, 1.0));


	addOutput(Port::create<MLPort>(Vec(column1-2, 328 ), Port::OUTPUT, module, ShiftRegister2::OUT_OUTPUT));
	addOutput(Port::create<MLPort>(Vec(column2+2, 328 ), Port::OUTPUT, module, ShiftRegister2::AUX_OUTPUT));
}

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, ShiftRegister2) {
   Model *modelShiftRegister2 = Model::create<ShiftRegister2, ShiftRegister2Widget>("ML modules", "Evolution", "Evolution", SEQUENCER_TAG, SAMPLE_AND_HOLD_TAG);
   return modelShiftRegister2;
}
