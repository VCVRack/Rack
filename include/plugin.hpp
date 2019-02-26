#pragma once
#include "common.hpp"
#include "plugin/Plugin.hpp"
#include "plugin/Model.hpp"
#include <vector>
#include <set>


namespace rack {


/** Plugin loader and plugin manager
*/
namespace plugin {


void init();
void destroy();
void logIn(const std::string &email, const std::string &password);
void logOut();
/** Returns whether a new plugin is available, and downloads it unless doing a dry run */
bool sync(bool dryRun);
void cancelDownload();
bool isLoggedIn();
Plugin *getPlugin(const std::string &pluginSlug);
Model *getModel(const std::string &pluginSlug, const std::string &modelSlug);
std::string normalizeTag(const std::string &tag);
/** Checks that the slug contains only alphanumeric characters, "-", and "_" */
bool isSlugValid(const std::string &slug);


extern const std::set<std::string> allowedTags;
extern std::vector<Plugin*> plugins;
extern bool isDownloading;
extern float downloadProgress;
extern std::string downloadName;
extern std::string loginStatus;


} // namespace plugin
} // namespace rack
