#include "ML_modules.hpp"
#include "dsp/digital.hpp"

#include <sstream>
#include <iomanip>

namespace rack_plugin_ML_modules {

struct Counter : Module {
	enum ParamIds {
		MAX_PARAM,
		START_PARAM,
		STOP_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		LENGTH_INPUT,
		GATE_INPUT,
		START_INPUT,
		STOP_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		GATE_OUTPUT,
		START_OUTPUT,
		STOP_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Counter() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) { reset(); };

	void step() override;

	void reset() override {counter=0; state=false; state2=false; gSampleRate=engineGetSampleRate();};

	int counter = 0;
	bool state = false;
	bool state2 = false;
	float stateLight;

	float gSampleRate;

	void onSampleRateChange() override {gSampleRate = engineGetSampleRate();}

        int max;
  
	SchmittTrigger startTrigger, gateTrigger, stopTrigger;
	PulseGenerator startPulse, stopPulse;
};



void Counter::step() {


	max = params[MAX_PARAM].value;


	if( inputs[LENGTH_INPUT].active ) max = max * clamp(inputs[LENGTH_INPUT].value/10.0f,0.0f,1.0f);

	if( startTrigger.process(inputs[START_INPUT].normalize(0.0) + params[START_PARAM].value )) {
		state=true; 
		counter=gateTrigger.isHigh()?1:0;
		startPulse.trigger(0.001);
	};

	if( stopTrigger.process(inputs[STOP_INPUT].normalize(0.0) + params[STOP_PARAM].value ))   {
		state=false; 
		counter=0;
	};

	if( gateTrigger.process(inputs[GATE_INPUT].normalize(0.0) ) ) {

		if(state) counter++; 

		if(counter > max) {
			
			counter = 0;
			state   = false;
			stopPulse.trigger(0.001);
		};
		
	};


	float out = (gateTrigger.isHigh()&&state) ? 10.0 : 0.0;

	float start = (startPulse.process(1.0/gSampleRate)) ? 10.0 : 0.0;
	float stop  = (stopPulse.process( 1.0/gSampleRate)) ? 10.0 : 0.0;

	outputs[GATE_OUTPUT].value  = out;
	outputs[START_OUTPUT].value = start;
	outputs[STOP_OUTPUT].value  = stop;

};

struct NumberDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  NumberDisplayWidget() {
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

    std::string to_display = std::to_string(*value);


    while(to_display.length()<3) to_display = ' ' + to_display;

    Vec textPos = Vec(6.0f, 17.0f);

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\\\", NULL);

    textColor = nvgRGB(0xf0, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.c_str(), NULL);
  }
};


struct CounterWidget : ModuleWidget {
	CounterWidget(Counter *module);
};

CounterWidget::CounterWidget(Counter *module) : ModuleWidget(module) {
	box.size = Vec(15*6, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/Counter.svg")));
		addChild(panel);
	}


	

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));

	addParam(ParamWidget::create<SmallBlueMLKnob>(Vec(12,  85), module, Counter::MAX_PARAM, 0.0, 128.0, 8.0));
	addInput(Port::create<MLPort>( Vec(53, 87), Port::INPUT, module, Counter::LENGTH_INPUT));

	addInput(Port::create<MLPort>(  Vec(13, 168), Port::INPUT, module, Counter::GATE_INPUT));
	addOutput(Port::create<MLPort>(Vec(53, 168), Port::OUTPUT, module, Counter::GATE_OUTPUT));


	addInput(Port::create<MLPort>(  Vec(13, 241), Port::INPUT, module, Counter::START_INPUT));
	addOutput(Port::create<MLPort>(Vec(53, 241), Port::OUTPUT, module, Counter::START_OUTPUT));
	addParam(ParamWidget::create<MLSmallButton>(   Vec(58, 222), module, Counter::START_PARAM, 0.0, 10.0, 0.0));


	addInput(Port::create<MLPort>(  Vec(13, 312), Port::INPUT, module, Counter::STOP_INPUT));
	addOutput(Port::create<MLPort>(Vec(53, 312), Port::OUTPUT, module, Counter::STOP_OUTPUT));
	addParam(ParamWidget::create<MLSmallButton>(   Vec(58, 293), module, Counter::STOP_PARAM, 0.0, 10.0, 0.0));

	NumberDisplayWidget *display = new NumberDisplayWidget();
	display->box.pos = Vec(20,56);
	display->box.size = Vec(50, 20);
	display->value = &module->max;
	addChild(display);

	
	NumberDisplayWidget *display2 = new NumberDisplayWidget();
	display2->box.pos = Vec(20,145);
	display2->box.size = Vec(50, 20);
	display2->value = &module->counter;
	addChild(display2);

}

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, Counter) {
   Model *modelCounter = Model::create<Counter, CounterWidget>("ML modules", "Counter", "Counter", UTILITY_TAG);
   return modelCounter;
}
