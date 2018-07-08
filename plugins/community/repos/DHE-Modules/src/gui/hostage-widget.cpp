#include <componentlibrary.hpp>

#include "modules/hostage-module.h"
#include "hostage-widget.h"

namespace rack_plugin_DHE_Modules {

struct HostagePort : rack::SVGPort {
  HostagePort() {
    background->svg = rack::SVG::load(rack::assetPlugin(plugin, "res/hostage/port.svg"));
    background->wrap();
    box.size = background->box.size;
  }
};

struct HostageKnobLarge : rack::RoundKnob {
  HostageKnobLarge() {
    setSVG(rack::SVG::load(rack::assetPlugin(plugin, "res/hostage/knob-large.svg")));
    shadow->opacity = 0.f;
  }
};

struct HostageSwitch2 : rack::SVGSwitch, rack::ToggleSwitch {
  HostageSwitch2() {
    addFrame(rack::SVG::load(rack::assetPlugin(plugin, "res/hostage/switch-2-low.svg")));
    addFrame(rack::SVG::load(rack::assetPlugin(plugin, "res/hostage/switch-2-high.svg")));
  }
};

struct HostageSwitch3 : rack::SVGSwitch, rack::ToggleSwitch {
  HostageSwitch3() {
    addFrame(rack::SVG::load(rack::assetPlugin(plugin, "res/hostage/switch-3-low.svg")));
    addFrame(rack::SVG::load(rack::assetPlugin(plugin, "res/hostage/switch-3-mid.svg")));
    addFrame(rack::SVG::load(rack::assetPlugin(plugin, "res/hostage/switch-3-high.svg")));
  }
};

HostageWidget::HostageWidget(rack::Module *module) : ModuleWidget(module, 5, "res/hostage/panel.svg") {
  auto widget_right_edge = width();

  auto left_x = width()/4.f + 0.333333f;
  auto center_x = widget_right_edge/2.f;
  auto right_x = widget_right_edge - left_x;

  auto top_row_y = 25.f;
  auto row_spacing = 18.5f;

  auto row = 0;
  install_switch<HostageSwitch2>(HostageModule::GATE_MODE_SWITCH, {center_x, top_row_y + row*row_spacing});

  row++;
  install_input<HostagePort>(HostageModule::DURATION_CV, {left_x, top_row_y + row*row_spacing});
  install_switch<HostageSwitch3>(HostageModule::DURATION_SWITCH, {right_x, top_row_y + row*row_spacing}, 2, 1);

  row++;
  install_knob<HostageKnobLarge>(HostageModule::DURATION_KNOB, {center_x, top_row_y + row*row_spacing});

  top_row_y = 82.f;
  row_spacing = 15.f;

  row = 0;
  install_input<HostagePort>(HostageModule::DEFER_IN, {left_x, top_row_y + row*row_spacing});
  install_output<HostagePort>(HostageModule::ACTIVE_OUT, {right_x, top_row_y + row*row_spacing});

  row++;
  install_input<HostagePort>(HostageModule::HOLD_GATE_IN, {left_x, top_row_y + row*row_spacing});
  install_output<HostagePort>(HostageModule::EOC_OUT, {right_x, top_row_y + row*row_spacing});

  row++;
  install_input<HostagePort>(HostageModule::ENVELOPE_IN, {left_x, top_row_y + row*row_spacing});
  install_output<HostagePort>(HostageModule::ENVELOPE_OUT, {right_x, top_row_y + row*row_spacing});
}

} // namespace rack_plugin_DHE_Modules

using namespace rack_plugin_DHE_Modules;

RACK_PLUGIN_MODEL_INIT(DHE_Modules, Hostage) {
   Model *modelHostage = rack_plugin_DHE_Modules::createModel<rack_plugin_DHE_Modules::HostageModule, rack_plugin_DHE_Modules::HostageWidget, rack::ModelTag>("Hostage", rack::ENVELOPE_GENERATOR_TAG);
   return modelHostage;
}
