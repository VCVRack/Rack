#include "../include/ColourDragging.hpp"
#include "../include/BaseWidget.hpp"
#include "window.hpp"

namespace rack_plugin_rcm {

BaseWidget::BaseWidget(Module* module) : ModuleWidget(module) {}

void BaseWidget::onDragStart(EventDragStart& e) {
  if (currentDragType == NULL) {
    Vec pos = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.minus(box.pos);
    bool inColourDragZone = colourHotZone.contains(pos);

    if (inColourDragZone && windowIsShiftPressed()) {
      currentDragType = new ColourDragging(this);
    }
  }

  ModuleWidget::onDragStart(e);
}

void BaseWidget::onDragMove(EventDragMove& e) {
  if (currentDragType != NULL) {
    currentDragType->onDragMove(e);
  } else {
    ModuleWidget::onDragMove(e);
  }
}

void BaseWidget::onDragEnd(EventDragEnd& e) {
  if (currentDragType != NULL) {
    delete currentDragType;
    currentDragType = NULL;
  }

  ModuleWidget::onDragEnd(e);
}

json_t *BaseWidget::toJson() {
  json_t *rootJ = ModuleWidget::toJson();
  if (rootJ == NULL) {
      rootJ = json_object();
  }

  json_object_set_new(rootJ, "backgroundHue", json_real(this->backgroundHue));
  json_object_set_new(rootJ, "backgroundSaturation", json_real(this->backgroundSaturation));
  json_object_set_new(rootJ, "backgroundLuminosity", json_real(this->backgroundLuminosity));
  return rootJ;
}

void BaseWidget::fromJson(json_t *rootJ) {
  ModuleWidget::fromJson(rootJ);

  json_t *backgroundHueJ = json_object_get(rootJ, "backgroundHue");
  if (backgroundHueJ) {
    backgroundHue = json_real_value(backgroundHueJ);
  }

  json_t *backgroundSaturationJ = json_object_get(rootJ, "backgroundSaturation");
  if (backgroundSaturationJ) {
    backgroundSaturation = json_real_value(backgroundSaturationJ);
  }

  json_t *backgroundLuminosityJ = json_object_get(rootJ, "backgroundLuminosity");
  if (backgroundLuminosityJ) {
    backgroundLuminosity = json_real_value(backgroundLuminosityJ);
  }
}

void BaseWidget::draw(NVGcontext* ctx) {
  nvgBeginPath(ctx);
  nvgFillColor(ctx, nvgHSL(backgroundHue, backgroundSaturation, backgroundLuminosity));
  nvgRect(ctx, 0, 0, box.size.x, box.size.y);
  nvgFill(ctx);

  ModuleWidget::draw(ctx);
}

} // namespace rack_plugin_rcm

