#include <patch.hpp>
#include <asset.hpp>
#include <system.hpp>
#include <engine/Engine.hpp>
#include <context.hpp>
#include <app/common.hpp>
#include <app/Scene.hpp>
#include <app/RackWidget.hpp>
#include <history.hpp>
#include <settings.hpp>
#include <algorithm>

#include <osdialog.h>


namespace rack {


static const char PATCH_FILTERS[] = "VCV Rack patch (.vcv):vcv";


PatchManager::PatchManager() {
	path = settings::patchPath;
}

PatchManager::~PatchManager() {
	settings::patchPath = path;
}

void PatchManager::init(std::string path) {
	if (!path.empty()) {
		// Load patch
		load(path);
		this->path = path;
		return;
	}

	if (!settings::devMode) {
		// To prevent launch crashes, if Rack crashes between now and 15 seconds from now, the "skipAutosaveOnLaunch" property will remain in settings.json, so that in the next launch, the broken autosave will not be loaded.
		bool oldSkipLoadOnLaunch = settings::skipLoadOnLaunch;
		settings::skipLoadOnLaunch = true;
		settings::save(asset::settingsPath);
		settings::skipLoadOnLaunch = false;
		if (oldSkipLoadOnLaunch && osdialog_message(OSDIALOG_INFO, OSDIALOG_YES_NO, "Rack has recovered from a crash, possibly caused by a faulty module in your patch. Clear your patch and start over?")) {
			this->path = "";
			return;
		}
	}

	// Load autosave
	if (load("")) {
		return;
	}

	reset();
}

void PatchManager::reset() {
	if (!settings::headless) {
		APP->history->clear();
		APP->scene->rack->clear();
		APP->scene->rackScroll->reset();
	}
	APP->engine->clear();

	path = "";
	if (load(asset::templatePath)) {
		return;
	}

	if (load(asset::system("template.vcv"))) {
		return;
	}
}

static bool promptClear(std::string text) {
	if (APP->history->isSaved())
		return true;
	if (APP->scene->rack->isEmpty())
		return true;
	return osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, text.c_str());
}

void PatchManager::resetDialog() {
	if (!promptClear("The current patch is unsaved. Clear it and start a new patch?"))
		return;
	reset();
}

void PatchManager::save(std::string path) {
	INFO("Saving patch %s", path.c_str());
	json_t* rootJ = toJson();
	if (!rootJ)
		return;
	DEFER({
		json_decref(rootJ);
	});

	// Write to temporary path and then rename it to the correct path
	std::string tmpPath = path + ".tmp";
	FILE* file = std::fopen(tmpPath.c_str(), "w");
	if (!file) {
		// Fail silently
		return;
	}

	json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
	std::fclose(file);
	system::moveFile(tmpPath, path);
}

void PatchManager::saveDialog() {
	if (!path.empty()) {
		save(path);
		APP->history->setSaved();
	}
	else {
		saveAsDialog();
	}
}

void PatchManager::saveAsDialog() {
	std::string dir;
	std::string filename;
	if (path.empty()) {
		dir = asset::user("patches");
		system::createDirectory(dir);
	}
	else {
		dir = string::directory(path);
		filename = string::filename(path);
	}

	osdialog_filters* filters = osdialog_filters_parse(PATCH_FILTERS);
	DEFER({
		osdialog_filters_free(filters);
	});

	char* pathC = osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), filters);
	if (!pathC) {
		// Fail silently
		return;
	}
	DEFER({
		std::free(pathC);
	});

	// Append .vcv extension if no extension was given.
	std::string pathStr = pathC;
	if (string::filenameExtension(string::filename(pathStr)) == "") {
		pathStr += ".vcv";
	}

	save(pathStr);
	path = pathStr;
	APP->history->setSaved();
}

void PatchManager::saveTemplateDialog() {
	// Even if <user>/template.vcv doesn't exist, this message is still valid because it overrides the <system>/template.vcv patch.
	if (!osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "Overwrite template patch?"))
		return;

	save(asset::templatePath);
}

bool PatchManager::load(std::string path) {
	std::string actualPath = (path != "") ? path : asset::autosavePath;

	INFO("Loading patch %s", actualPath.c_str());
	FILE* file = std::fopen(actualPath.c_str(), "r");
	if (!file) {
		// Exit silently
		return false;
	}
	DEFER({
		std::fclose(file);
	});

	json_error_t error;
	json_t* rootJ = json_loadf(file, 0, &error);
	if (!rootJ) {
		std::string message = string::f("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
		return false;
	}
	DEFER({
		json_decref(rootJ);
	});

	if (!settings::headless) {
		APP->history->clear();
		APP->scene->rack->clear();
		APP->scene->rackScroll->reset();
	}
	APP->engine->clear();
	fromJson(rootJ);

	// Update recent patches
	if (path != "") {
		auto& recent = settings::recentPatchPaths;
		// Remove path from recent patches (if exists)
		recent.erase(std::remove(recent.begin(), recent.end(), path), recent.end());
		// Add path to top of recent patches
		recent.push_front(path);
		// Limit recent patches size
		recent.resize(std::min((int) recent.size(), 10));
	}

	return true;
}

void PatchManager::loadDialog() {
	if (!promptClear("The current patch is unsaved. Clear it and open a new patch?"))
		return;

	std::string dir;
	if (path.empty()) {
		dir = asset::user("patches");
		system::createDirectory(dir);
	}
	else {
		dir = string::directory(path);
	}

	osdialog_filters* filters = osdialog_filters_parse(PATCH_FILTERS);
	DEFER({
		osdialog_filters_free(filters);
	});

	char* pathC = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, filters);
	if (!pathC) {
		// Fail silently
		return;
	}
	DEFER({
		std::free(pathC);
	});

	load(pathC);
	path = pathC;
	APP->history->setSaved();
}

void PatchManager::loadPathDialog(std::string path) {
	if (!promptClear("The current patch is unsaved. Clear it and open the new patch?"))
		return;

	load(path);
	this->path = path;
	APP->history->setSaved();
}

void PatchManager::revertDialog() {
	if (path.empty())
		return;
	if (!promptClear("Revert patch to the last saved state?"))
		return;

	load(path);
	APP->history->setSaved();
}

void PatchManager::disconnectDialog() {
	APP->scene->rack->clearCablesAction();
}

json_t* PatchManager::toJson() {
	// root
	json_t* rootJ = json_object();

	// version
	json_t* versionJ = json_string(APP_VERSION.c_str());
	json_object_set_new(rootJ, "version", versionJ);

	json_t* engineJ = APP->engine->toJson();
	if (!settings::headless) {
		APP->scene->rack->mergeJson(engineJ);
	}

	// Merge with rootJ
	json_object_update(rootJ, engineJ);
	json_decref(engineJ);

	return rootJ;
}

void PatchManager::fromJson(json_t* rootJ) {
	legacy = 0;

	// version
	std::string version;
	json_t* versionJ = json_object_get(rootJ, "version");
	if (versionJ)
		version = json_string_value(versionJ);
	if (version != APP_VERSION) {
		INFO("Patch was made with Rack v%s, current Rack version is v%s", version.c_str(), APP_VERSION.c_str());
	}

	// Detect old patches with ModuleWidget::params/inputs/outputs indices.
	if (string::startsWith(version, "0.3.") || string::startsWith(version, "0.4.") || string::startsWith(version, "0.5.") || version == "" || version == "dev") {
		// Use ModuleWidget::params/inputs/outputs indices instead of Module.
		legacy = 1;
	}
	else if (string::startsWith(version, "0.6.")) {
		legacy = 2;
	}
	if (legacy) {
		INFO("Loading patch using legacy mode %d", legacy);
	}

	APP->engine->fromJson(rootJ);
	if (!settings::headless) {
		APP->scene->rack->fromJson(rootJ);
	}
	// At this point, ModuleWidgets and CableWidgets should own all Modules and Cables.
	// TODO Assert this

	// Display a message if we have something to say
	if (!warningLog.empty()) {
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, warningLog.c_str());
	}
	warningLog = "";
}

bool PatchManager::isLegacy(int level) {
	return legacy && legacy <= level;
}

void PatchManager::log(std::string msg) {
	warningLog += msg;
	warningLog += "\n";
}


} // namespace rack
