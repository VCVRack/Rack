#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {

struct PasteMeasureItem : MenuItem {
  PianoRollWidget *widget = NULL;
  PianoRollModule *module = NULL;
  void onAction(EventAction &e) override {
    module->patternData.pasteMeasure(module->transport.currentPattern(), widget->rollAreaWidget->state.currentMeasure);
  }
};

} // namespace rack_plugin_rcm
