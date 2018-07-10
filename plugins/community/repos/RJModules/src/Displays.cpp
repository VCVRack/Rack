#include "RJModules.hpp"

#include "dsp/digital.hpp"
#include "common.hpp"
#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>

/*
Thanks to Strum for the display widget!
*/

namespace rack_plugin_RJModules {

struct Displays: Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        CH1_INPUT,
        CH2_INPUT,
        CH3_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        CH1_OUTPUT,
        CH2_OUTPUT,
        CH3_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    float display1_val;
    float display2_val;
    float display3_val;

    Displays() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

struct NumberDisplayWidgeter : TransparentWidget {

  float *value;
  std::shared_ptr<Font> font;

  NumberDisplayWidgeter() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };

  void draw(NVGcontext *vg) override
  {
    // Background
    NVGcolor backgroundColor = nvgRGB(0xC0, 0xC0, 0xC0);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);

    // text
    nvgFontSize(vg, 32);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::stringstream to_display;
    to_display = format4display(*value);

    Vec textPos = Vec(16.0f, 33.0f);
    NVGcolor textColor = nvgRGB(0x00, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);

    nvgFontSize(vg, 16);
    textPos = Vec(1.0f, (*value<0?20.0f:30.0f));
    nvgText(vg, textPos.x, textPos.y, (*value<0?"-":"+"), NULL);
  }
};

void Displays::step() {

    display1_val = inputs[CH1_INPUT].value;
    outputs[CH1_OUTPUT].value = inputs[CH1_INPUT].value;

    display2_val = inputs[CH2_INPUT].value;
    outputs[CH2_OUTPUT].value = inputs[CH2_INPUT].value;

    display3_val = inputs[CH3_INPUT].value;
    outputs[CH3_OUTPUT].value = inputs[CH3_INPUT].value;

}

struct DisplaysWidget: ModuleWidget {
    DisplaysWidget(Displays *module);
};

DisplaysWidget::DisplaysWidget(Displays *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Displays.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));


    NumberDisplayWidgeter *display = new NumberDisplayWidgeter();
    display->box.pos = Vec(28, 60);
    display->box.size = Vec(100, 40);
    display->value = &module->display1_val;
    addChild(display);

    addInput(Port::create<PJ301MPort>(Vec(35, 123), Port::INPUT, module, Displays::CH1_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(95, 123), Port::OUTPUT, module, Displays::CH1_OUTPUT));

    NumberDisplayWidgeter *display2 = new NumberDisplayWidgeter();
    display2->box.pos = Vec(28, 160);
    display2->box.size = Vec(100, 40);
    display2->value = &module->display2_val;
    addChild(display2);

    addInput(Port::create<PJ301MPort>(Vec(35, 223), Port::INPUT, module, Displays::CH2_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(95, 223), Port::OUTPUT, module, Displays::CH2_OUTPUT));

    NumberDisplayWidgeter *display3 = new NumberDisplayWidgeter();
    display3->box.pos = Vec(28, 260);
    display3->box.size = Vec(100, 40);
    display3->value = &module->display3_val;
    addChild(display3);

    addInput(Port::create<PJ301MPort>(Vec(35, 323), Port::INPUT, module, Displays::CH3_INPUT));
    addOutput(Port::create<PJ301MPort>(Vec(95, 323), Port::OUTPUT, module, Displays::CH3_OUTPUT));

}

} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, Displays) {
   Model *modelDisplays = Model::create<Displays, DisplaysWidget>("RJModules", "Displays", "[UTIL] Displays", VISUAL_TAG);
   return modelDisplays;
}
