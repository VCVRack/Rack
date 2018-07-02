#include "cf.hpp"


namespace rack_plugin_cf {

struct PEAK : Module {
	enum ParamIds {
		TRESHOLD_PARAM,
		TRIM1_PARAM,
        GAIN_PARAM,
        G_PARAM,
        T_PARAM,
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
    enum LightIds {
		TRESHOLD_LIGHT,
		OVER_LIGHT,
		NUM_LIGHTS
	};
float max_GAIN = 1.0 ;
int affich = 1.0 ;
int reman_t = 0;
int reman_o = 0;
int sensiv = 10000;


	PEAK() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void PEAK::step() {

	max_GAIN = roundf(params[GAIN_PARAM].value*10);

	affich = round(max_GAIN);

	if (inputs[IN1_INPUT].active)
	{

        if (inputs[IN1_INPUT].value > params[TRESHOLD_PARAM].value)
        	{
            	outputs[OUT1_OUTPUT].value = (max_GAIN/10.0*(params[TRESHOLD_PARAM].value + ((inputs[IN1_INPUT].value-params[TRESHOLD_PARAM].value)/(1+(inputs[IN1_INPUT].value-params[TRESHOLD_PARAM].value)))));
            	reman_t = sensiv;
        	}
        else if (inputs[IN1_INPUT].value < 0-params[TRESHOLD_PARAM].value)
        	{
            	outputs[OUT1_OUTPUT].value = (max_GAIN/10.0*(0-(params[TRESHOLD_PARAM].value - ((inputs[IN1_INPUT].value+params[TRESHOLD_PARAM].value)/(1+(-inputs[IN1_INPUT].value-params[TRESHOLD_PARAM].value))))));
            	reman_t = sensiv;
        	}
        	else 
		{
          	  outputs[OUT1_OUTPUT].value =(max_GAIN*inputs[IN1_INPUT].value)/10.0;
       		}

        if (outputs[OUT1_OUTPUT].value >10) reman_o=sensiv;
        
	}
	else
	{
	outputs[OUT1_OUTPUT].value = max_GAIN/10;
        lights[TRESHOLD_LIGHT].value = 0.0;
        lights[OVER_LIGHT].value = 0.0;
	}

	if (reman_t >0) 
	{
	reman_t--;
	lights[TRESHOLD_LIGHT].value = 1;
	} 
	else lights[TRESHOLD_LIGHT].value = 0.0;

	if (reman_o >0) 
	{
	reman_o--;
	lights[OVER_LIGHT].value = 1;
	} 
	else lights[OVER_LIGHT].value = 0.0;
}

struct NumbDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  NumbDisplayWidget() {
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


    while(to_display.length()<3) to_display = '0' + to_display;

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
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x+1, textPos.y, " . ", NULL);
    nvgText(vg, textPos.x+1, textPos.y-1, " . ", NULL);

  }
};


struct PEAKWidget : ModuleWidget {
	PEAKWidget(PEAK *module);
};

PEAKWidget::PEAKWidget(PEAK *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/PEAK.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(27, 97), module, PEAK::GAIN_PARAM, 0.0f, 10.0f, 1.0f));
 		addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(42.4, 141.4), module, PEAK::OVER_LIGHT));

	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(27, 227), module, PEAK::TRESHOLD_PARAM, 0.0f, 10.0f, 10.0f));
		addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(42.4, 211.4), module, PEAK::TRESHOLD_LIGHT));

	addInput(Port::create<PJ301MPort>(Vec(11, 321), Port::INPUT, module, PEAK::IN1_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(54, 321), Port::OUTPUT, module, PEAK::OUT1_OUTPUT));

NumbDisplayWidget *display = new NumbDisplayWidget();
	display->box.pos = Vec(20,56);
	display->box.size = Vec(50, 20);
	display->value = &module->affich;
	addChild(display);

	
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, PEAK) {
   Model *modelPEAK = Model::create<PEAK, PEAKWidget>("cf", "PEAK", "Peak", LIMITER_TAG);
   return modelPEAK;
}
