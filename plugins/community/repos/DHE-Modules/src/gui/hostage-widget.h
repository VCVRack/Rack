#pragma once

#include "module-widget.h"

namespace rack_plugin_DHE_Modules {

struct HostageWidget : public ModuleWidget {
  explicit HostageWidget(rack::Module *module);
};
}
