#include <algorithm>

#include <osdialog.h>

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

#include <fstream>


namespace rack {


static const char PATCH_FILTERS[] = "VCV Rack patch (.vcv):vcv";


PatchManager::PatchManager() {
}


PatchManager::~PatchManager() {
	cleanAutosave();
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
	// Save patch.json
	saveAutosave();
	// Clean up autosave directory (e.g. removed modules)
	cleanAutosave();

	// Take screenshot (disabled because there is currently no way to quickly view them on any OS or website.)
	// APP->window->screenshot(system::join(asset::autosavePath, "screenshot.png"));

	uint64_t startTime = system::getNanoseconds();
	// Set compression level to 1 so that a 500MB/s SSD is almost bottlenecked
	system::archiveFolder(path, asset::autosavePath, 1);
	uint64_t endTime = system::getNanoseconds();
	INFO("Archived patch in %lf seconds", (endTime - startTime) / 1e9);
}


void PatchManager::saveDialog() {
	if (path == "") {
		saveAsDialog();
		return;
	}

	try {
		save(path);
	}
	catch (Exception& e) {
		WARN("Could not save patch: %s", e.what());
		return;
	}

	APP->history->setSaved();
}


void PatchManager::saveAsDialog() {
	std::string dir;
	std::string filename;
	if (this->path == "") {
		dir = asset::user("patches");
		system::createDirectories(dir);
	}
	else {
		dir = system::getDirectory(this->path);
		filename = system::getFilename(this->path);
	}

	osdialog_filters* filters = osdialog_filters_parse(PATCH_FILTERS);
	DEFER({osdialog_filters_free(filters);});

	char* pathC = osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), filters);
	if (!pathC) {
		// Cancel silently
		return;
	}
	DEFER({std::free(pathC);});

	// Append .vcv extension if no extension was given.
	std::string path = pathC;
	if (system::getExtension(path) == "") {
		path += ".vcv";
	}

	try {
		save(path);
	}
	catch (Exception& e) {
		WARN("Could not save patch: %s", e.what());
		return;
	}

	this->path = path;
	APP->history->setSaved();
	pushRecentPath(path);
}


void PatchManager::saveTemplateDialog() {
	// Even if <user>/template.vcv doesn't exist, this message is still valid because it overrides the <system>/template.vcv patch.
	if (!osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "Overwrite template patch?"))
		return;

	try {
		save(asset::templatePath);
	}
	catch (Exception& e) {
		WARN("Could not save template patch: %s", e.what());
		return;
	}
}


void PatchManager::saveAutosave() {
	std::string patchPath = system::join(asset::autosavePath, "patch.json");
	INFO("Saving autosave %s", patchPath.c_str());
	json_t* rootJ = toJson();
	if (!rootJ)
		return;
	DEFER({json_decref(rootJ);});

	// Write to temporary path and then rename it to the correct path
	system::createDirectories(asset::autosavePath);
	std::string tmpPath = patchPath + ".tmp";
	FILE* file = std::fopen(tmpPath.c_str(), "w");
	if (!file) {
		// Fail silently
		return;
	}

	json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
	std::fclose(file);
	system::remove(patchPath);
	system::rename(tmpPath, patchPath);
}


void PatchManager::cleanAutosave() {
	// Remove files and folders in the `autosave/modules` folder that doesn't match a module in the rack.
	std::string modulesDir = system::join(asset::autosavePath, "modules");
	if (system::isDirectory(modulesDir)) {
		for (const std::string& entry : system::getEntries(modulesDir)) {
			try {
				int64_t moduleId = std::stol(system::getFilename(entry));
				// Ignore modules that exist in the rack
				if (APP->engine->getModule(moduleId))
					continue;
			}
			catch (std::invalid_argument& e) {
			}
			// Remove the entry.
			system::removeRecursively(entry);
		}
	}
}


static bool isPatchLegacyPre2(std::string path) {
	FILE* f = std::fopen(path.c_str(), "rb");
	if (!f)
		return false;
	DEFER({std::fclose(f);});
	// All Zstandard frames start with this magic number.
	char zstdMagic[] = "\x28\xb5\x2f\xfd";
	char buf[4] = {};
	std::fread(buf, 1, sizeof(buf), f);
	// If the patch file doesn't begin with the magic number, it's a legacy patch.
	return std::memcmp(buf, zstdMagic, sizeof(buf)) != 0;
}


void PatchManager::load(std::string path) {
	INFO("Loading patch %s", path.c_str());

	system::removeRecursively(asset::autosavePath);
	system::createDirectories(asset::autosavePath);

	if (isPatchLegacyPre2(path)) {
		// Copy the .vcv file directly to "patch.json".
		system::copy(path, system::join(asset::autosavePath, "patch.json"));
	}
	else {
		// Extract the .vcv file as a .tar.zst archive.
		uint64_t startTime = system::getNanoseconds();
		system::unarchiveToFolder(path, asset::autosavePath);
		uint64_t endTime = system::getNanoseconds();
		INFO("Unarchived patch in %lf seconds", (endTime - startTime) / 1e9);
	}

	loadAutosave();
}


void PatchManager::loadTemplate() {
	this->path = "";
	APP->history->setSaved();

	try {
		load(asset::templatePath);
		return;
	}
	catch (Exception& e) {
		INFO("Could not load user template patch, attempting system template patch: %s", e.what());
	}

	try {
		load(asset::system("template.vcv"));
		return;
	}
	catch (Exception& e) {
		WARN("Could not load system template patch, clearing rack: %s", e.what());
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
	std::string patchPath = system::join(asset::autosavePath, "patch.json");
	INFO("Loading autosave %s", patchPath.c_str());
	FILE* file = std::fopen(patchPath.c_str(), "r");
	if (!file) {
		// Exit silently
		// TODO Load template without causing infinite recursion
		return;
	}
	DEFER({std::fclose(file);});

	json_error_t error;
	json_t* rootJ = json_loadf(file, 0, &error);
	if (!rootJ) {
		std::string message = string::f("Failed to load patch. JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
		return;
	}
	DEFER({json_decref(rootJ);});

	fromJson(rootJ);
}


void PatchManager::loadAction(std::string path) {
	try {
		load(path);
	}
	catch (Exception& e) {
		WARN("Could not load patch: %s", e.what());
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
		dir = system::getDirectory(this->path);
	}

	osdialog_filters* filters = osdialog_filters_parse(PATCH_FILTERS);
	DEFER({osdialog_filters_free(filters);});

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

	try {
		load(path);
	}
	catch (Exception& e) {
		WARN("Could not load patch: %s", e.what());
		return;
	}

	APP->history->setSaved();
}


void PatchManager::pushRecentPath(std::string path) {
	auto& recent = settings::recentPatchPaths;
	// Remove path from recent patches (if exists)
	recent.remove(path);
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
	clear();
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
