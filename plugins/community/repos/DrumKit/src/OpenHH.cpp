#include <stdint.h>

#include "../deps/SynthDevKit/src/CV.hpp"
#include "DrumKit.hpp"
#include "DrumModule.hpp"
#include "openhh.h"

namespace rack_plugin_DrumKit {

struct OpenHHModule : DrumModule {
  void setupSamples( ) override;
};

void OpenHHModule::setupSamples( ) {
  samples[0] = { (float *)openhh1, openhh1_len };
  samples[1] = { (float *)openhh2, openhh2_len };
  samples[2] = { (float *)openhh3, openhh3_len };
  samples[3] = { (float *)openhh4, openhh4_len };
  samples[4] = { (float *)openhh5, openhh5_len };
  samples[5] = { (float *)openhh6, openhh6_len };
  samples[6] = { (float *)openhh7, openhh7_len };
  samples[7] = { (float *)openhh8, openhh8_len };
  samples[8] = { (float *)openhh9, openhh9_len };
  samples[9] = { (float *)openhh10, openhh10_len };
  samples[10] = { (float *)openhh11, openhh11_len };
  samples[11] = { (float *)openhh12, openhh12_len };
  samples[12] = { (float *)openhh13, openhh13_len };
  samples[13] = { (float *)openhh14, openhh14_len };

  numSamples = 14;
}

struct OpenHHWidget : ModuleWidget {
  OpenHHWidget(OpenHHModule *module);
};

OpenHHWidget::OpenHHWidget(OpenHHModule *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel( );
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/OpenHH.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<PJ301MPort>(Vec(10, 45), Port::INPUT, module,
                                   OpenHHModule::CLOCK1_INPUT));
  addParam(ParamWidget::create<RoundBlackSnapKnob>(
      Vec(8, 92), module, OpenHHModule::DRUM1_PARAM, 1.0, 14.0, 7.0));

  addOutput(Port::create<PJ301MPort>(Vec(10, 149), Port::OUTPUT, module,
                                     OpenHHModule::AUDIO1_OUTPUT));

  addInput(Port::create<PJ301MPort>(Vec(10, 205), Port::INPUT, module,
                                   OpenHHModule::CLOCK2_INPUT));
  addParam(ParamWidget::create<RoundBlackSnapKnob>(
      Vec(8, 252), module, OpenHHModule::DRUM2_PARAM, 1.0, 14.0, 7.0));

  addOutput(Port::create<PJ301MPort>(Vec(10, 308), Port::OUTPUT, module,
                                     OpenHHModule::AUDIO2_OUTPUT));
}

} // namespace rack_plugin_DrumKit

using namespace rack_plugin_DrumKit;

RACK_PLUGIN_MODEL_INIT(DrumKit, OpenHH) {
   Model *modelOpenHH = Model::create<OpenHHModule, OpenHHWidget>("DrumKit", "Open HiHat", "Open HiHat", DRUM_TAG);
   return modelOpenHH;
}
