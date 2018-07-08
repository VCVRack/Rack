#pragma once

#include <rack.hpp>

using namespace rack;

RACK_PLUGIN_DECLARE(DHE_Modules);

#ifdef USE_VST2
#define plugin "DHE-Modules"
#endif // USE_VST2

namespace rack_plugin_DHE_Modules {
template<typename TModel, typename TWidget, typename... TTag>
static rack::Model *createModel(std::string moduleSlug, TTag... tags) {
  return rack::Model::create<TModel, TWidget>("DHE-Modules", moduleSlug, moduleSlug, tags...);
}
}
