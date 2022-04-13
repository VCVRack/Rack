#pragma once
#include <vector>

#include <common.hpp>
#include <plugin/Plugin.hpp>
#include <plugin/Model.hpp>


namespace rack {
/** Loads and manages Rack plugins */
namespace plugin {


PRIVATE void init();
PRIVATE void destroy();
PRIVATE void settingsMergeJson(json_t* rootJ);

/** Finds a loaded Plugin by slug. */
Plugin* getPlugin(const std::string& pluginSlug);
/** Finds a loaded Plugin by slug, or a fallback plugin if exists. */
Plugin* getPluginFallback(const std::string& pluginSlug);
/** Finds a loaded Model by plugin and model slug. */
Model* getModel(const std::string& pluginSlug, const std::string& modelSlug);
/** Finds a loaded Model by plugin and model slug, or a fallback model if exists. */
Model* getModelFallback(const std::string& pluginSlug, const std::string& modelSlug);

/** Creates a Model from a JSON module object.
Throws an Exception if the model is not found.
*/
Model* modelFromJson(json_t* moduleJ);
/** Checks that the slug contains only alphanumeric characters, "-", and "_" */
bool isSlugValid(const std::string& slug);
/** Returns a string containing only the valid slug characters. */
std::string normalizeSlug(const std::string& slug);


/** Path to plugins installation dir */
extern std::string pluginsPath;
extern std::vector<Plugin*> plugins;


} // namespace plugin
} // namespace rack
