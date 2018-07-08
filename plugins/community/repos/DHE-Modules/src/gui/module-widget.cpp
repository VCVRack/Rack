#include <algorithm>
#include <random>

#include <app.hpp>
#include <componentlibrary.hpp>
#include <engine.hpp>

#include "module-widget.h"

namespace rack_plugin_DHE_Modules {

ModuleWidget::ModuleWidget(rack::Module *module, int widget_hp, const char *background)
    : rack::ModuleWidget(module) {
  box.size = rack::Vec{(float) widget_hp*rack::RACK_GRID_WIDTH, rack::RACK_GRID_HEIGHT};

  auto *panel = new rack::SVGPanel();
  panel->box.size = box.size;
  panel->setBackground(rack::SVG::load(rack::assetPlugin(plugin, background)));
  addChild(panel);

  install_screws();
}

void ModuleWidget::install_screws() {

  auto screw_diameter = rack::RACK_GRID_WIDTH*MM_PER_IN/SVG_DPI;
  auto screw_radius = screw_diameter/2.f;

  auto top = screw_radius;
  auto bottom = height() - top;

  auto max_screw_inset = screw_diameter*1.5f;
  auto left = std::min(width()/4.f, max_screw_inset);
  auto right = width() - left;

  auto screw_positions = std::vector<rack::Vec>{
      {left, top},
      {left, bottom},
      {right, top},
      {right, bottom}
  };

  std::shuffle(screw_positions.begin(), screw_positions.end(), std::mt19937(std::random_device()()));

  install_screw<rack::ScrewBlack>(screw_positions.back());

  screw_positions.pop_back();

  for (auto p : screw_positions) {
    install_screw<rack::ScrewSilver>(p);
  }
}

void ModuleWidget::moveTo(rack::Rect &box, rack::Vec center) {
  box.pos = center.minus(box.size.mult(0.5f));
}
}
