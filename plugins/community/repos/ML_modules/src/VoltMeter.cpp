#include "ML_modules.hpp"
#include "dsp/digital.hpp"

#include <sstream>
#include <iomanip>

namespace rack_plugin_ML_modules {

struct VoltMeter : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		IN3_INPUT,
		IN4_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};


	VoltMeter() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) { for(int i=0; i<4; i++) {volts[i] = 0.0f; active[i] = false;}};


	void step() override;

	float volts[4];
	bool active[4];

};



void VoltMeter::step() {

	for(int i=0; i<4; i++) {
		active[i] = inputs[IN1_INPUT+i].active;
		volts[i] = 0.9 * volts[i] + 0.1 * inputs[IN1_INPUT+i].normalize(0.0);
	};



};

struct VoltDisplayWidget : TransparentWidget {

  float  *value;
  bool *on;

  std::shared_ptr<Font> font;

  VoltDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };

  void draw(NVGcontext *vg) {
    // Background
//    NVGcolor backgroundColor = nvgRGB(0x44, 0x44, 0x44);
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

    char display_string[10];

    sprintf(display_string,"%6.2f",*value);

    Vec textPos = Vec(6.0f, 17.0f);

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~~~~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\\\\\\\\\", NULL);

	if(*on) {
	    textColor = nvgRGB(0xf0, 0x00, 0x00);
		nvgFillColor(vg, textColor);
		nvgText(vg, textPos.x, textPos.y, display_string, NULL);
	};
  }
};


struct VoltMeterWidget : ModuleWidget {
	VoltMeterWidget(VoltMeter *module);
	TextField ** label;
};

VoltMeterWidget::VoltMeterWidget(VoltMeter *module) : ModuleWidget(module) {

	box.size = Vec(15*8, 380);

//	label = new TextField*[4];

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/VoltMeter.svg")));
		addChild(panel);
	}


	const float delta_y = 70;

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 365)));


	for(int i=0; i<4; i++) {

		addInput(Port::create<MLPort>(Vec(12, 60+i*delta_y), Port::INPUT, module, VoltMeter::IN1_INPUT+i));


		VoltDisplayWidget *display = new VoltDisplayWidget();
		display->box.pos = Vec(10,90+i*delta_y);
		display->box.size = Vec(100, 20);
		display->value = &module->volts[i];
		display->on = &module->active[i];
		addChild(display);

//		label[i] = new TextField();
//		label[i]->box.pos = Vec(50,60+i*65);
//		label[i]->box.size = Vec(60,30);
//		addChild(label[i]);
	};
	

}

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, VoltMeter) {
   Model *modelVoltMeter = Model::create<VoltMeter, VoltMeterWidget>("ML modules", "VoltMeter", "Volt Meter", VISUAL_TAG, UTILITY_TAG);
   return modelVoltMeter;
}
