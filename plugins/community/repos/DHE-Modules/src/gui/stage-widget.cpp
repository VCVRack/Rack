#include <componentlibrary.hpp>

#include "modules/stage-module.h"
#include "stage-widget.h"

namespace rack_plugin_DHE_Modules {

struct StagePort : rack::SVGPort {
  StagePort() {
    background->svg = rack::SVG::load(rack::assetPlugin(plugin, "res/stage/port.svg"));
    background->wrap();
    box.size = background->box.size;
  }
};

struct StageKnobLarge : rack::RoundKnob {
  StageKnobLarge() {
    setSVG(rack::SVG::load(rack::assetPlugin(plugin, "res/stage/knob-large.svg")));
    shadow->opacity = 0.f;
  }
};

StageWidget::StageWidget(rack::Module *module) : ModuleWidget(module, 5, "res/stage/panel.svg") {
  auto widget_right_edge = width();

  auto left_x = width()/4.f + 0.333333f;
  auto center_x = widget_right_edge/2.f;
  auto right_x = widget_right_edge - left_x;

  auto top_row_y = 25.f;
  auto row_spacing = 18.5f;

  auto row = 0;
  install_knob<StageKnobLarge>(StageModule::LEVEL_KNOB, {center_x, top_row_y + row*row_spacing});

  row++;
  install_knob<StageKnobLarge>(StageModule::CURVE_KNOB, {center_x, top_row_y + row*row_spacing});

  row++;
  install_knob<StageKnobLarge>(StageModule::DURATION_KNOB, {center_x, top_row_y + row*row_spacing});

  top_row_y = 82.f;
  row_spacing = 15.f;

  row = 0;
  install_input<StagePort>(StageModule::DEFER_IN, {left_x, top_row_y + row*row_spacing});
  install_output<StagePort>(StageModule::ACTIVE_OUT, {right_x, top_row_y + row*row_spacing});

  row++;
  install_input<StagePort>(StageModule::TRIGGER_IN, {left_x, top_row_y + row*row_spacing});
  install_output<StagePort>(StageModule::EOC_OUT, {right_x, top_row_y + row*row_spacing});

  row++;
  install_input<StagePort>(StageModule::ENVELOPE_IN, {left_x, top_row_y + row*row_spacing});
  install_output<StagePort>(StageModule::ENVELOPE_OUT, {right_x, top_row_y + row*row_spacing});
}

} // namespace rack_plugin_DHE_Modules

using namespace rack_plugin_DHE_Modules;

RACK_PLUGIN_MODEL_INIT(DHE_Modules, Stage) {
   Model *modelStage = rack_plugin_DHE_Modules::createModel<rack_plugin_DHE_Modules::StageModule, rack_plugin_DHE_Modules::StageWidget, rack::ModelTag>("Stage", rack::ENVELOPE_GENERATOR_TAG);
   return modelStage;
}
