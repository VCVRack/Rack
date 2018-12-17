#pragma once
#include "common.hpp"
#include "plugin/Plugin.hpp"
#include "plugin/Model.hpp"
#include <list>


namespace rack {


struct PluginManager {
	std::list<Plugin*> plugins;
	std::string token;

	bool isDownloading = false;
	float downloadProgress = 0.f;
	std::string downloadName;
	std::string loginStatus;

	PluginManager();
	~PluginManager();
	void logIn(std::string email, std::string password);
	void logOut();
	/** Returns whether a new plugin is available, and downloads it unless doing a dry run */
	bool sync(bool dryRun);
	void cancelDownload();
	bool isLoggedIn();
	Plugin *getPlugin(std::string pluginSlug);
	Model *getModel(std::string pluginSlug, std::string modelSlug);
};

extern PluginManager *gPluginManager;


} // namespace rack


////////////////////
// Implemented by plugin
////////////////////

/** Called once to initialize and return the Plugin instance.
You must implement this in your plugin
*/
extern "C"
void init(rack::Plugin *plugin);
