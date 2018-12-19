#pragma once
#include "common.hpp"
#include "plugin/Plugin.hpp"
#include "plugin/Model.hpp"
#include <vector>
#include <list>


namespace rack {
namespace plugin {


void init(bool devMode);
void destroy();
void logIn(std::string email, std::string password);
void logOut();
/** Returns whether a new plugin is available, and downloads it unless doing a dry run */
bool sync(bool dryRun);
void cancelDownload();
bool isLoggedIn();
Plugin *getPlugin(std::string pluginSlug);
Model *getModel(std::string pluginSlug, std::string modelSlug);
std::string getAllowedTag(std::string tag);


extern std::list<Plugin*> plugins;
extern std::string token;
extern bool isDownloading;
extern float downloadProgress;
extern std::string downloadName;
extern std::string loginStatus;
extern const std::vector<std::string> allowedTags;


} // namespace plugin
} // namespace rack
