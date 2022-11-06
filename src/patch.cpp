#include <algorithm>
#include <fstream>

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


namespace rack {
namespace patch {


static const char PATCH_FILTERS[] = "VCV Rack patch (.vcv):vcv";


Manager::Manager() {
	autosavePath = asset::user("autosave");

	// Use a different temporary autosave dir when safe mode is enabled, to avoid altering normal autosave.
	if (settings::safeMode) {
		autosavePath = asset::user("autosave-safe");
		clearAutosave();
	}

	templatePath = asset::user("template.vcv");
	factoryTemplatePath = asset::system("template.vcv");
}


Manager::~Manager() {
	// In safe mode, delete autosave dir.
	if (settings::safeMode) {
		clearAutosave();
		return;
	}

	// Dispatch onSave to all Modules so they save their patch storage, etc.
	APP->engine->prepareSave();
	// Save autosave if not headless
	if (!settings::headless) {
		APP->patch->saveAutosave();
	}
	cleanAutosave();
}


void Manager::launch(std::string pathArg) {
	// Don't load any patches if safe mode is enabled
	if (settings::safeMode)
		return;

	// Load the argument if exists
	if (pathArg != "") {
		loadAction(pathArg);
		return;
	}

	// Try loading the autosave patch
	if (hasAutosave()) {
		try {
			loadAutosave();
			// Keep path and save state as it was stored in patch.json
		}
		catch (Exception& e) {
			osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, e.what());
		}
		return;
	}

	// Try loading the template patch
	loadTemplate();
}


void Manager::clear() {
	path = "";
	if (APP->scene) {
		APP->scene->rack->clear();
		APP->scene->rackScroll->reset();
	}
	if (APP->history) {
		APP->history->clear();
	}
	APP->engine->clear();
}


static bool promptClear(std::string text) {
	if (APP->history->isSaved())
		return true;
	if (APP->scene->rack->hasModules())
		return true;
	return osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, text.c_str());
}


void Manager::save(std::string path) {
	INFO("Saving patch %s", path.c_str());
	// Dispatch SaveEvent to modules
	APP->engine->prepareSave();
	// Save patch.json
	saveAutosave();
	// Clean up autosave directory (e.g. removed modules)
	cleanAutosave();

	// Take screenshot (disabled because there is currently no way to quickly view them on any OS or website.)
	// APP->window->screenshot(system::join(autosavePath, "screenshot.png"));

	double startTime = system::getTime();
	// Set compression level to 1 so that a 500MB/s SSD is almost bottlenecked
	system::archiveDirectory(path, autosavePath, 1);
	double endTime = system::getTime();
	INFO("Archived patch in %lf seconds", (endTime - startTime));
}


void Manager::saveDialog() {
	if (path == "") {
		saveAsDialog();
		return;
	}

	// Note: If save() fails below, this should probably be reset. But we need it so toJson() doesn't set the "unsaved" property.
	APP->history->setSaved();

	try {
		save(path);
	}
	catch (Exception& e) {
		std::string message = string::f("Could not save patch: %s", e.what());
		osdialog_message(OSDIALOG_INFO, OSDIALOG_OK, message.c_str());
		return;
	}
}


void Manager::saveAsDialog(bool setPath) {
	std::string dir;
	std::string filename;
	if (this->path == "") {
		dir = asset::user("patches");
		system::createDirectories(dir);
		filename = "Untitled.vcv";
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

	// Automatically append .vcv extension
	std::string path = pathC;
	if (system::getExtension(path) != ".vcv") {
		path += ".vcv";
	}

	APP->history->setSaved();
	if (setPath) {
		this->path = path;
	}

	try {
		save(path);
	}
	catch (Exception& e) {
		std::string message = string::f("Could not save patch: %s", e.what());
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
		return;
	}

	pushRecentPath(path);
}


void Manager::saveTemplateDialog() {
	// Even if <user>/template.vcv doesn't exist, this message is still valid because it overrides the <system>/template.vcv patch.
	if (!osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "Overwrite template patch?"))
		return;

	try {
		save(templatePath);
	}
	catch (Exception& e) {
		std::string message = string::f("Could not save template patch: %s", e.what());
		osdialog_message(OSDIALOG_INFO, OSDIALOG_OK, message.c_str());
		return;
	}
}


void Manager::saveAutosave() {
	std::string patchPath = system::join(autosavePath, "patch.json");
	INFO("Saving autosave %s", patchPath.c_str());
	json_t* rootJ = toJson();
	if (!rootJ)
		return;
	DEFER({json_decref(rootJ);});

	// Write to temporary path and then rename it to the correct path
	system::createDirectories(autosavePath);
	std::string tmpPath = patchPath + ".tmp";
	FILE* file = std::fopen(tmpPath.c_str(), "w");
	if (!file) {
		// Fail silently
		return;
	}

	json_dumpf(rootJ, file, JSON_INDENT(2));
	std::fclose(file);
	system::remove(patchPath);
	system::rename(tmpPath, patchPath);
}


void Manager::clearAutosave() {
	system::removeRecursively(autosavePath);
}


void Manager::cleanAutosave() {
	// Remove files and directories in the `autosave/modules` directory that doesn't match a module in the rack.
	std::string modulesDir = system::join(autosavePath, "modules");
	if (system::isDirectory(modulesDir)) {
		for (const std::string& entry : system::getEntries(modulesDir)) {
			try {
				int64_t moduleId = std::stoll(system::getFilename(entry));
				// Ignore modules that exist in the rack
				if (APP->engine->getModule(moduleId))
					continue;
			}
			catch (std::invalid_argument& e) {}
			catch (std::out_of_range& e) {}
			// Remove the entry.
			system::removeRecursively(entry);
		}
	}
}


static bool isPatchLegacyV1(std::string path) {
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


void Manager::load(std::string path) {
	INFO("Loading patch %s", path.c_str());

	clear();
	clearAutosave();
	system::createDirectories(autosavePath);

	if (isPatchLegacyV1(path)) {
		// Copy the .vcv file directly to "patch.json".
		system::copy(path, system::join(autosavePath, "patch.json"));
	}
	else {
		// Extract the .vcv file as a .tar.zst archive.
		double startTime = system::getTime();
		system::unarchiveToDirectory(path, autosavePath);
		double endTime = system::getTime();
		INFO("Unarchived patch in %lf seconds", (endTime - startTime));
	}

	loadAutosave();
}


void Manager::loadTemplate() {
	try {
		load(templatePath);
	}
	catch (Exception& e) {
		// Try loading the system template patch
		try {
			load(factoryTemplatePath);
		}
		catch (Exception& e) {
			std::string message = string::f("Could not load system template patch, clearing rack: %s", e.what());
			osdialog_message(OSDIALOG_INFO, OSDIALOG_OK, message.c_str());

			clear();
			clearAutosave();
		}
	}

	// load() sets the patch's original patch, but we don't want to use that.
	this->path = "";
	APP->history->setSaved();
}


void Manager::loadTemplateDialog() {
	if (!promptClear("The current patch is unsaved. Clear it and start a new patch?")) {
		return;
	}
	loadTemplate();
}


bool Manager::hasAutosave() {
	std::string patchPath = system::join(autosavePath, "patch.json");
	FILE* file = std::fopen(patchPath.c_str(), "r");
	if (!file)
		return false;
	std::fclose(file);
	return true;
}


void Manager::loadAutosave() {
	std::string patchPath = system::join(autosavePath, "patch.json");
	INFO("Loading autosave %s", patchPath.c_str());
	FILE* file = std::fopen(patchPath.c_str(), "r");
	if (!file)
		throw Exception("Could not open autosave patch %s", patchPath.c_str());
	DEFER({std::fclose(file);});

	json_error_t error;
	json_t* rootJ = json_loadf(file, 0, &error);
	if (!rootJ)
		throw Exception("Failed to load patch. JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
	DEFER({json_decref(rootJ);});

	fromJson(rootJ);
}


void Manager::loadAction(std::string path) {
	try {
		load(path);
	}
	catch (Exception& e) {
		std::string message = string::f("Could not load patch: %s", e.what());
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
		return;
	}

	this->path = path;
	APP->history->setSaved();
	pushRecentPath(path);
}


void Manager::loadDialog() {
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


void Manager::loadPathDialog(std::string path) {
	if (!promptClear("The current patch is unsaved. Clear it and open the new patch?"))
		return;

	loadAction(path);
}


void Manager::revertDialog() {
	if (path == "")
		return;
	if (!promptClear("Revert patch to the last saved state?"))
		return;

	loadAction(path);
}


void Manager::pushRecentPath(std::string path) {
	auto& recent = settings::recentPatchPaths;
	// Remove path from recent patches (if exists)
	recent.remove(path);
	// Add path to top of recent patches
	recent.push_front(path);
	// Limit recent patches size
	recent.resize(std::min((int) recent.size(), 10));
}


void Manager::disconnectDialog() {
	APP->scene->rack->clearCablesAction();
}


json_t* Manager::toJson() {
	// root
	json_t* rootJ = json_object();

	// version
	json_t* versionJ = json_string(APP_VERSION.c_str());
	json_object_set_new(rootJ, "version", versionJ);

	// path
	if (path != "") {
		json_t* pathJ = json_string(path.c_str());
		json_object_set_new(rootJ, "path", pathJ);
	}

	// unsaved
	if (!APP->history->isSaved())
		json_object_set_new(rootJ, "unsaved", json_boolean(true));

	if (APP->scene) {
		// zoom
		float zoom = APP->scene->rackScroll->getZoom();
		json_object_set_new(rootJ, "zoom", json_real(zoom));

		// gridOffset
		math::Vec gridOffset = APP->scene->rackScroll->getGridOffset();
		json_t* gridOffsetJ = json_pack("[f, f]", gridOffset.x, gridOffset.y);
		json_object_set_new(rootJ, "gridOffset", gridOffsetJ);
	}

	// Merge with Engine JSON
	json_t* engineJ = APP->engine->toJson();
	json_object_update(rootJ, engineJ);
	json_decref(engineJ);

	// Merge with RackWidget JSON
	if (APP->scene) {
		APP->scene->rack->mergeJson(rootJ);
	}

	return rootJ;
}


void Manager::fromJson(json_t* rootJ) {
	clear();
	warningLog = "";

	// version
	std::string version;
	json_t* versionJ = json_object_get(rootJ, "version");
	if (versionJ)
		version = json_string_value(versionJ);
	if (version != APP_VERSION) {
		INFO("Patch was made with Rack %s, current Rack version is %s", version.c_str(), APP_VERSION.c_str());
	}

	// path
	json_t* pathJ = json_object_get(rootJ, "path");
	if (pathJ)
		path = json_string_value(pathJ);
	else
		path = "";

	// unsaved
	json_t* unsavedJ = json_object_get(rootJ, "unsaved");
	if (!unsavedJ)
		APP->history->setSaved();

	if (APP->scene) {
		// zoom
		json_t* zoomJ = json_object_get(rootJ, "zoom");
		if (zoomJ)
			APP->scene->rackScroll->setZoom(json_number_value(zoomJ));

		// gridOffset
		json_t* gridOffsetJ = json_object_get(rootJ, "gridOffset");
		if (gridOffsetJ) {
			double x, y;
			json_unpack(gridOffsetJ, "[F, F]", &x, &y);
			APP->scene->rackScroll->setGridOffset(math::Vec(x, y));
		}
	}

	// Pass JSON to Engine and RackWidget
	try {
		APP->engine->fromJson(rootJ);
		if (APP->scene) {
			APP->scene->rack->fromJson(rootJ);
		}
	}
	catch (Exception& e) {
		warningLog += "\n";
		warningLog += e.what();
	}
	// At this point, ModuleWidgets and CableWidgets should own all Modules and Cables.
	// TODO Assert this

	// Display a message if we have something to say.
	if (warningLog != "") {
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, warningLog.c_str());
	}
	warningLog = "";
}


void Manager::log(std::string msg) {
	warningLog += msg;
	warningLog += "\n";
}


} // namespace patch
} // namespace rack
