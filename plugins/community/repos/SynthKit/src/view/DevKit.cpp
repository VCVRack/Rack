#include "../controller/DevKit.hpp"
#include "../../deps/rack-components/jacks.hpp"
#include "../../deps/rack-components/screws.hpp"
#include "display.hpp"

namespace rack_plugin_SynthKit {

struct DevKitWidget : ModuleWidget {
  DevKitWidget(DevKitModule *module);
};

DevKitWidget::DevKitWidget(DevKitModule *module) : ModuleWidget(module) {
  box.size = Vec(5 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/DevKit.svg")));
    addChild(panel);
  }

  addChild(Widget::create<JLHHexScrew>(Vec(1, 1)));
  addChild(Widget::create<JLHHexScrew>(Vec(61, 1)));
  addChild(Widget::create<JLHHexScrew>(Vec(1, 366)));
  addChild(Widget::create<JLHHexScrew>(Vec(61, 366)));

  addChild(ModuleLightWidget::create<MediumLight<RedLight>>(
      Vec(33, 234.4), module, DevKitModule::BLINK_LIGHT));

  addInput(Port::create<RCJackSmallLight>(Vec(25.23, 73), Port::INPUT, module,
                                          DevKitModule::DEV_INPUT));

  // min
  {
    FloatDisplay *f1 = new FloatDisplay();
    f1->value = &module->min;
    f1->box.pos = Vec(5.5, 86.75);
    f1->box.size = Vec(40, 20);
    addChild(f1);
  }

  // max
  {
    FloatDisplay *f2 = new FloatDisplay();
    f2->value = &module->max;
    f2->box.pos = Vec(6.5, 106.35);
    f2->box.size = Vec(40, 20);
    addChild(f2);
  }

  // cvcount
  {
    IntDisplay *i1 = new IntDisplay();
    i1->value = &module->cvcount;
    i1->box.pos = Vec(6, 139.4);
    i1->box.size = Vec(40, 20);
    addChild(i1);
  }

  // interval
  {
    IntDisplay *i2 = new IntDisplay();
    i2->value = &module->cvinterval;
    i2->box.pos = Vec(6, 158.8);
    i2->box.size = Vec(40, 20);
    addChild(i2);
  }
}

} // namespace rack_plugin_SynthKit

using namespace rack_plugin_SynthKit;

RACK_PLUGIN_MODEL_INIT(SynthKit, DevKit) {
   Model *modelDevKit = Model::create<DevKitModule, DevKitWidget>(
      "SynthKit", "DevKit", "DevKit", UTILITY_TAG);
   return modelDevKit;
}

