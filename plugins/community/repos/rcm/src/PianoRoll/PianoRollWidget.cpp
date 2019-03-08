#include "rack.hpp"
#include "window.hpp"
#include "../rcm.h"
#include "../../include/PianoRoll/PatternWidget.hpp"
#include "../../include/PianoRoll/PianoRollWidget.hpp"
#include "../../include/PianoRoll/PianoRollModule.hpp"
#include "../../include/PianoRoll/MenuItems/CancelPasteItem.hpp"
#include "../../include/PianoRoll/MenuItems/ClearNotesItem.hpp"
#include "../../include/PianoRoll/MenuItems/ClockBufferItem.hpp"
#include "../../include/PianoRoll/MenuItems/CopyMeasureItem.hpp"
#include "../../include/PianoRoll/MenuItems/CopyPatternItem.hpp"
#include "../../include/PianoRoll/MenuItems/NotesToShowItem.hpp"
#include "../../include/PianoRoll/MenuItems/PasteMeasureItem.hpp"
#include "../../include/PianoRoll/MenuItems/PastePatternItem.hpp"

using namespace rack;

namespace rack_plugin_rcm {

PianoRollWidget::PianoRollWidget(PianoRollModule *module) : BaseWidget(module) {
  colourHotZone = Rect(Vec(506, 10), Vec(85, 13));
	backgroundHue = 0.5f;
	backgroundSaturation = 1.f;
	backgroundLuminosity = 0.25f;

  this->module = (PianoRollModule*)module;
  setPanel(SVG::load(assetPlugin(plugin, "res/PianoRoll.svg")));

  addInput(Port::create<PJ301MPort>(Vec(50.114, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::CLOCK_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(85.642, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::RESET_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(121.170, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::PATTERN_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(156.697, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::RUN_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(192.224, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::RECORD_INPUT));

  addInput(Port::create<PJ301MPort>(Vec(421.394, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::VOCT_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(456.921, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::GATE_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(492.448, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::RETRIGGER_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(527.976, 380.f-91-23.6), Port::INPUT, module, PianoRollModule::VELOCITY_INPUT));

  addOutput(Port::create<PJ301MPort>(Vec(50.114, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::CLOCK_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(85.642, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RESET_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(121.170, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::PATTERN_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(156.697, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RUN_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(192.224, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RECORD_OUTPUT));

  addOutput(Port::create<PJ301MPort>(Vec(421.394, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::VOCT_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(456.921, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::GATE_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(492.448, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::RETRIGGER_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(527.976, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::VELOCITY_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(563.503, 380.f-25.9-23.6), Port::OUTPUT, module, PianoRollModule::END_OF_PATTERN_OUTPUT));

  rollAreaWidget = new RollAreaWidget(&module->patternData, &module->transport, &module->auditioner);
  rollAreaWidget->box = getRollArea();
  addChild(rollAreaWidget);

  PatternWidget* patternWidget = Widget::create<PatternWidget>(Vec(505.257, 380.f-224.259-125.586));
  patternWidget->module = module;
  patternWidget->widget = this;
  addChild(patternWidget);
}

void PianoRollWidget::appendContextMenu(Menu* menu) {

  menu->addChild(MenuLabel::create(""));
  menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Copy / Paste"));

  CopyPatternItem *copyPatternItem = new CopyPatternItem();
  copyPatternItem->widget = this;
  copyPatternItem->module = module;
  copyPatternItem->text = "Copy Pattern";
  menu->addChild(copyPatternItem);

  CopyMeasureItem *copyMeasureItem = new CopyMeasureItem();
  copyMeasureItem->widget = this;
  copyMeasureItem->module = module;
  copyMeasureItem->text = "Copy Measure";
  menu->addChild(copyMeasureItem);

  switch(state) {
    case COPYREADY:
      break;
    case PATTERNLOADED:
      {
        PastePatternItem *pastePatternItem = new PastePatternItem();
        pastePatternItem->widget = this;
        pastePatternItem->module = module;
        pastePatternItem->text = "Paste Pattern";
        menu->addChild(pastePatternItem);
      }
      break;
    case MEASURELOADED:
      {
        PasteMeasureItem *pasteMeasureItem = new PasteMeasureItem();
        pasteMeasureItem->widget = this;
        pasteMeasureItem->module = module;
        pasteMeasureItem->text = "Paste Measure";
        menu->addChild(pasteMeasureItem);
      }
      break;
    default:
      state = COPYREADY;
      break;
  }

  menu->addChild(MenuLabel::create(""));
    menu->addChild(new ClearNotesItem(this->module));

  menu->addChild(MenuLabel::create(""));
  menu->addChild(MenuLabel::create("Notes to Show"));
    menu->addChild(new NotesToShowItem(this, 12));
    menu->addChild(new NotesToShowItem(this, 18));
    menu->addChild(new NotesToShowItem(this, 24));
    menu->addChild(new NotesToShowItem(this, 36));
    menu->addChild(new NotesToShowItem(this, 48));
    menu->addChild(new NotesToShowItem(this, 60));
  menu->addChild(MenuLabel::create(""));
  menu->addChild(MenuLabel::create("Clock Delay (samples)"));
    menu->addChild(new ClockBufferItem(module, 0));
    menu->addChild(new ClockBufferItem(module, 1));
    menu->addChild(new ClockBufferItem(module, 2));
    menu->addChild(new ClockBufferItem(module, 3));
    menu->addChild(new ClockBufferItem(module, 4));
    menu->addChild(new ClockBufferItem(module, 5));
    menu->addChild(new ClockBufferItem(module, 10));
}

Rect PianoRollWidget::getRollArea() {
  Rect roll;
  roll.pos.x = 15.f;
  roll.pos.y = 380-365.f;
  roll.size.x = 480.f;
  roll.size.y = 220.f;
  return roll;
}


json_t *PianoRollWidget::toJson() {
  json_t *rootJ = BaseWidget::toJson();
  if (rootJ == NULL) {
      rootJ = json_object();
  }

  json_object_set_new(rootJ, "lowestDisplayNote", json_integer(this->rollAreaWidget->state.lowestDisplayNote));
  json_object_set_new(rootJ, "notesToShow", json_integer(this->rollAreaWidget->state.notesToShow));
  json_object_set_new(rootJ, "currentMeasure", json_integer(this->rollAreaWidget->state.currentMeasure));

  return rootJ;
}

void PianoRollWidget::fromJson(json_t *rootJ) {
  BaseWidget::fromJson(rootJ);

  json_t *lowestDisplayNoteJ = json_object_get(rootJ, "lowestDisplayNote");
  if (lowestDisplayNoteJ) {
    rollAreaWidget->state.lowestDisplayNote = json_integer_value(lowestDisplayNoteJ);
  }

  json_t *notesToShowJ = json_object_get(rootJ, "notesToShow");
  if (notesToShowJ) {
    rollAreaWidget->state.notesToShow = json_integer_value(notesToShowJ);
  }

  json_t *currentMeasureJ = json_object_get(rootJ, "currentMeasure");
  if (currentMeasureJ) {
    rollAreaWidget->state.currentMeasure = json_integer_value(currentMeasureJ);
  }

  rollAreaWidget->state.dirty = true;
}

} // namespace rack_plugin_rcm
