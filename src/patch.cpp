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
}

PatchManager::~PatchManager() {
}

void PatchManager::reset() {
	if (APP->history) {
		APP->history->clear();
	}
	if (APP->scene) {
		APP->scene->rackScroll->reset();
	}
}

void PatchManager::clear() {
	if (APP->scene) {
		APP->scene->rack->clear();
	}
	APP->engine->clear();
	reset();
}

static bool promptClear(std::string text) {
	if (APP->history->isSaved())
		return true;
	if (APP->scene->rack->isEmpty())
		return true;
	return osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, text.c_str());
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
	if (path == "") {
		saveAsDialog();
	}
	else {
		save(path);
		APP->history->setSaved();
	}
}

void PatchManager::saveAsDialog() {
	std::string dir;
	std::string filename;
	if (this->path == "") {
		dir = asset::user("patches");
		system::createDirectory(dir);
	}
	else {
		dir = string::directory(this->path);
		filename = string::filename(this->path);
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
	std::string path = pathC;
	if (string::filenameExtension(string::filename(path)) == "") {
		path += ".vcv";
	}

	save(path);
	this->path = path;
	APP->history->setSaved();
	pushRecentPath(path);
}

void PatchManager::saveTemplateDialog() {
	// Even if <user>/template.vcv doesn't exist, this message is still valid because it overrides the <system>/template.vcv patch.
	if (!osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "Overwrite template patch?"))
		return;

	save(asset::templatePath);
}

void PatchManager::saveAutosave() {
	save(asset::autosavePath);
}

bool PatchManager::load(std::string path) {
	INFO("Loading patch %s", path.c_str());
	FILE* file = std::fopen(path.c_str(), "r");
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
		std::string message = string::f("Failed to load patch. JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
		return false;
	}
	DEFER({
		json_decref(rootJ);
	});

	clear();
	fromJson(rootJ);
	return true;
}

void PatchManager::loadTemplate() {
	this->path = "";
	APP->history->setSaved();

	if (load(asset::templatePath)) {
		return;
	}

	if (load(asset::system("template.vcv"))) {
		return;
	}

	clear();
}

void PatchManager::loadTemplateDialog() {
	if (!promptClear("The current patch is unsaved. Clear it and start a new patch?")) {
		return;
	}
	loadTemplate();
}

void PatchManager::loadAutosave() {
	if (load(asset::autosavePath)) {
		return;
	}
	loadTemplate();
}

void PatchManager::loadAction(std::string path) {
	if (!load(path)) {
		return;
	}
	this->path = path;
	APP->history->setSaved();
	pushRecentPath(path);
}

void PatchManager::loadDialog() {
	if (!promptClear("The current patch is unsaved. Clear it and open a new patch?"))
		return;

	std::string dir;
	if (this->path == "") {
		dir = asset::user("patches");
		system::createDirectory(dir);
	}
	else {
		dir = string::directory(this->path);
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
	std::string path = pathC;
	std::free(pathC);

	loadAction(path);
}

void PatchManager::loadPathDialog(std::string path) {
	if (!promptClear("The current patch is unsaved. Clear it and open the new patch?"))
		return;

	loadAction(path);
}

void PatchManager::revertDialog() {
	if (path == "")
		return;
	if (!promptClear("Revert patch to the last saved state?"))
		return;

	load(path);
	APP->history->setSaved();
}

void PatchManager::pushRecentPath(std::string path) {
	auto& recent = settings::recentPatchPaths;
	// Remove path from recent patches (if exists)
	recent.erase(std::remove(recent.begin(), recent.end(), path), recent.end());
	// Add path to top of recent patches
	recent.push_front(path);
	// Limit recent patches size
	recent.resize(std::min((int) recent.size(), 10));
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

	// Merge with rootJ
	json_t* engineJ = APP->engine->toJson();
	json_object_update(rootJ, engineJ);
	json_decref(engineJ);

	if (APP->scene) {
		APP->scene->rack->mergeJson(rootJ);
	}

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
	if (APP->scene) {
		APP->scene->rack->fromJson(rootJ);
	}
	// At this point, ModuleWidgets and CableWidgets should own all Modules and Cables.
	// TODO Assert this

	// Display a message if we have something to say.
	if (warningLog != "") {
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
