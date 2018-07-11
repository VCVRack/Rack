#include <stdint.h>
#include "../deps/SynthDevKit/src/CV.hpp"
#include "DrumKit.hpp"
#include "DrumModule.hpp"
#include "kick.h"

namespace rack_plugin_DrumKit {

struct BD9Module : DrumModule {
  void setupSamples( ) override;
};

void BD9Module::setupSamples( ) {
  samples[0] = { (float *)kick1, kick1_len };
  samples[1] = { (float *)kick2, kick2_len };
  samples[2] = { (float *)kick3, kick3_len };
  samples[3] = { (float *)kick4, kick4_len };
  samples[4] = { (float *)kick5, kick5_len };
  samples[5] = { (float *)kick6, kick6_len };
  samples[6] = { (float *)kick7, kick7_len };
  samples[7] = { (float *)kick8, kick8_len };
  samples[8] = { (float *)kick9, kick9_len };
  samples[9] = { (float *)kick10, kick10_len };
  samples[10] = { (float *)kick11, kick11_len };
  samples[11] = { (float *)kick12, kick12_len };
  samples[12] = { (float *)kick13, kick13_len };
  samples[13] = { (float *)kick14, kick14_len };
  samples[14] = { (float *)kick15, kick15_len };
  samples[15] = { (float *)kick16, kick16_len };

  numSamples = 16;
}

struct BD9Widget : ModuleWidget {
    BD9Widget(BD9Module *module);
};

BD9Widget::BD9Widget(BD9Module *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel( );
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/BD9.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(
      Port::create<PJ301MPort>(Vec(10, 45), Port::INPUT, module, BD9Module::CLOCK1_INPUT));
  addParam(ParamWidget::create<RoundBlackSnapKnob>(
      Vec(8, 92), module, BD9Module::DRUM1_PARAM, 1.0, 16.0, 8.0));

  addOutput(
      Port::create<PJ301MPort>(Vec(10, 149), Port::OUTPUT, module, BD9Module::AUDIO1_OUTPUT));

  addInput(
      Port::create<PJ301MPort>(Vec(10, 205), Port::INPUT, module, BD9Module::CLOCK2_INPUT));
  addParam(ParamWidget::create<RoundBlackSnapKnob>(
      Vec(8, 252), module, BD9Module::DRUM2_PARAM, 1.0, 16.0, 8.0));

  addOutput(
      Port::create<PJ301MPort>(Vec(10, 308), Port::OUTPUT, module, BD9Module::AUDIO2_OUTPUT));
}

} // namespace rack_plugin_DrumKit

using namespace rack_plugin_DrumKit;

RACK_PLUGIN_MODEL_INIT(DrumKit, BD9) {
   Model *modelBD9 = Model::create<BD9Module, BD9Widget>("DrumKit", "Bass Drum 9", "Bass Drum 9", DRUM_TAG);
   return modelBD9;
}
