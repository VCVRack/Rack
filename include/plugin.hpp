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
void queryUpdates();
void syncUpdates();
void cancelDownload();
bool isLoggedIn();
Plugin *getPlugin(const std::string &pluginSlug);
Model *getModel(const std::string &pluginSlug, const std::string &modelSlug);
std::string normalizeTag(const std::string &tag);
/** Checks that the slug contains only alphanumeric characters, "-", and "_" */
bool isSlugValid(const std::string &slug);


struct Update {
	std::string pluginSlug;
	std::string version;
	std::string changelogUrl;
};


extern const std::set<std::string> allowedTags;
extern std::vector<Plugin*> plugins;

extern std::string loginStatus;
extern std::vector<Update> updates;
extern float downloadProgress;
extern std::string downloadName;


} // namespace plugin
} // namespace rack
