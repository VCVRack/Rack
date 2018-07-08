#pragma once

#include "module-widget.h"

namespace rack_plugin_DHE_Modules {

struct SwaveWidget : public ModuleWidget {
  explicit SwaveWidget(rack::Module *module);
};
}
