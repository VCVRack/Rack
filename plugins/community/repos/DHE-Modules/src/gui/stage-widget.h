#pragma once

#include "module-widget.h"

namespace rack_plugin_DHE_Modules {

struct StageWidget : public ModuleWidget {
  explicit StageWidget(rack::Module *module);
};
}
