#include "rack.hpp"
#include "window.hpp"
#include "../rcm.h"
#include "../../include/SongRoll/SongRollWidget.hpp"
#include "../../include/SongRoll/SongRollModule.hpp"
#include "../../include/SongRoll/DragModes.hpp"
#include "../../include/SongRoll/RollArea.hpp"
#include "../../include/Consts.hpp"

using namespace rack;

namespace rack_plugin_rcm {

using namespace SongRoll;

static const int NUM_CHANNELS = 8;


SongRollWidget::SongRollWidget(SongRollModule *module) : BaseWidget(module) {
  this->module = (SongRollModule*)module;

  colourHotZone = Rect(Vec(506, 10), Vec(85, 13));
	backgroundHue = 0.33f;
	backgroundSaturation = 1.f;
	backgroundLuminosity = 0.25f;

  setPanel(SVG::load(assetPlugin(plugin, "res/SongRoll.svg")));

  // addInput(Port::create<PJ301MPort>(Vec(50.114, 380.f-91-23.6), Port::INPUT, module, SongRollModule::CLOCK_INPUT));
  // addInput(Port::create<PJ301MPort>(Vec(85.642, 380.f-91-23.6), Port::INPUT, module, SongRollModule::RESET_INPUT));
  // addInput(Port::create<PJ301MPort>(Vec(121.170, 380.f-91-23.6), Port::INPUT, module, SongRollModule::PATTERN_INPUT));
  // addInput(Port::create<PJ301MPort>(Vec(156.697, 380.f-91-23.6), Port::INPUT, module, SongRollModule::RUN_INPUT));
  // addInput(Port::create<PJ301MPort>(Vec(192.224, 380.f-91-23.6), Port::INPUT, module, SongRollModule::RECORD_INPUT));

  // addInput(Port::create<PJ301MPort>(Vec(421.394, 380.f-91-23.6), Port::INPUT, module, SongRollModule::VOCT_INPUT));
  // addInput(Port::create<PJ301MPort>(Vec(456.921, 380.f-91-23.6), Port::INPUT, module, SongRollModule::GATE_INPUT));
  // addInput(Port::create<PJ301MPort>(Vec(492.448, 380.f-91-23.6), Port::INPUT, module, SongRollModule::RETRIGGER_INPUT));
  // addInput(Port::create<PJ301MPort>(Vec(527.976, 380.f-91-23.6), Port::INPUT, module, SongRollModule::VELOCITY_INPUT));

  // addOutput(Port::create<PJ301MPort>(Vec(50.114, 380.f-25.9-23.6), Port::OUTPUT, module, SongRollModule::CLOCK_OUTPUT));
  // addOutput(Port::create<PJ301MPort>(Vec(85.642, 380.f-25.9-23.6), Port::OUTPUT, module, SongRollModule::RESET_OUTPUT));
  // addOutput(Port::create<PJ301MPort>(Vec(121.170, 380.f-25.9-23.6), Port::OUTPUT, module, SongRollModule::PATTERN_OUTPUT));
  // addOutput(Port::create<PJ301MPort>(Vec(156.697, 380.f-25.9-23.6), Port::OUTPUT, module, SongRollModule::RUN_OUTPUT));
  // addOutput(Port::create<PJ301MPort>(Vec(192.224, 380.f-25.9-23.6), Port::OUTPUT, module, SongRollModule::RECORD_OUTPUT));

  // addOutput(Port::create<PJ301MPort>(Vec(421.394, 380.f-25.9-23.6), Port::OUTPUT, module, SongRollModule::VOCT_OUTPUT));
  // addOutput(Port::create<PJ301MPo  json_t *lowestDisplayNoteJ = json_object_get(rootJ, "lowestDisplayNote");

  // patternWidget->widget = this;
  // addChild(patternWidget);

  auto *rollArea = new RollArea(getRollArea(), module->songRollData);
  addChild(rollArea);
}

void SongRollWidget::appendContextMenu(Menu* menu) {

}

Rect SongRollWidget::getRollArea() {
  return Rect(Vec(16, 380-218-145), Vec(477, 217));
}

void SongRollWidget::drawBackgroundColour(NVGcontext* ctx) {
    nvgBeginPath(ctx);
    nvgFillColor(ctx, nvgHSL(backgroundHue, backgroundSaturation, backgroundLuminosity));
    nvgRect(ctx, 0, 0, box.size.x, box.size.y);
    nvgFill(ctx);
}

static int stepcount = 0;
void SongRollWidget::drawPatternEditors(NVGcontext* ctx) {
  //stepcount += 1;

  Rect rollArea = getRollArea();
  static const float PATTERN_AREA_HEIGHT = 1;
  static const float leftMargin = 0;
  // Rect patternArea(Vec(rollArea.pos.x, rollArea.pos.y + rollArea.size.y - (rollArea.size.y * PATTERN_AREA_HEIGHT)), Vec(rollArea.size.x, rollArea.size.y * PATTERN_AREA_HEIGHT));
  Rect patternArea(Vec(rollArea.pos.x + leftMargin, rollArea.pos.y + rollArea.size.y - (rollArea.size.y * PATTERN_AREA_HEIGHT)), Vec(rollArea.size.x - leftMargin, rollArea.size.y * PATTERN_AREA_HEIGHT));

  nvgBeginPath(ctx);
  nvgFillColor(ctx, nvgRGBA(1, 1, 1, 1));
  nvgRect(ctx, rollArea.pos.x, rollArea.pos.y, rollArea.size.x, rollArea.size.y);
  nvgFill(ctx);


  float channelWidth = patternArea.size.x / NUM_CHANNELS;

  nvgSave(ctx);
  nvgScissor(ctx, patternArea.pos.x, patternArea.pos.y, patternArea.size.x, patternArea.size.y);

  for(int i = 1; i < NUM_CHANNELS; i++) {
    nvgBeginPath(ctx);
    nvgMoveTo(ctx, patternArea.pos.x + (channelWidth * i), patternArea.pos.y);
    nvgLineTo(ctx, patternArea.pos.x + (channelWidth * i), patternArea.pos.y + patternArea.size.y);
    nvgStrokeWidth(ctx, 1.f);
    nvgStrokeColor(ctx, NV_YELLOW_H);
    nvgStroke(ctx);
  }

  nvgRestore(ctx);
}

void SongRollWidget::draw(NVGcontext* ctx) {
  drawBackgroundColour(ctx);

  BaseWidget::draw(ctx);

  drawPatternEditors(ctx);
}

struct ClickZone {
  Rect r;

};

//void std::vector<

void SongRollWidget::onMouseDown(EventMouseDown& e) {
  Vec pos = gRackWidget->lastMousePos.minus(box.pos);

  Rect repeatsMinus(Vec(), Vec());
  Rect repeatsPlus(Vec(), Vec());
  Rect modeFree(Vec(), Vec());
  Rect modeRepeats(Vec(), Vec());
  Rect modeLimit(Vec(), Vec());


  BaseWidget::onMouseDown(e);
}

void SongRollWidget::onDragStart(EventDragStart& e) {
  Vec pos = gRackWidget->lastMousePos.minus(box.pos);

  BaseWidget::onDragStart(e);
}

void SongRollWidget::baseDragMove(EventDragMove& e) {
  BaseWidget::onDragMove(e);
}

void SongRollWidget::onDragMove(EventDragMove& e) {
  BaseWidget::onDragMove(e);
}

void SongRollWidget::onDragEnd(EventDragEnd& e) {
  BaseWidget::onDragEnd(e);
}

json_t *SongRollWidget::toJson() {
  json_t *rootJ = BaseWidget::toJson();
  if (rootJ == NULL) {
      rootJ = json_object();
  }

  return rootJ;
}

void SongRollWidget::fromJson(json_t *rootJ) {
  BaseWidget::fromJson(rootJ);

}

} // namespace rack_plugin_rcm
