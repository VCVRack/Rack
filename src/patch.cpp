#include "patch.hpp"
#include "asset.hpp"
#include "system.hpp"
#include "app.hpp"
#include "app/Scene.hpp"
#include "app/RackWidget.hpp"
#include "osdialog.h"


namespace rack {


static const std::string PATCH_FILTERS = "VCV Rack patch (.vcv):vcv";


void PatchManager::reset() {
	app()->scene->rackWidget->clear();
	app()->scene->scrollWidget->offset = math::Vec(0, 0);
	// Fails silently if file does not exist
	load(asset::user("template.vcv"));
	legacy = 0;
	path = "";
}

void PatchManager::resetDialog() {
	if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "Clear patch and start over?")) {
		reset();
	}
}

void PatchManager::save(std::string path) {
	INFO("Saving patch %s", path.c_str());
	json_t *rootJ = toJson();
	if (!rootJ)
		return;
	DEFER({
		json_decref(rootJ);
	});

	FILE *file = std::fopen(path.c_str(), "w");
	if (!file) {
		// Fail silently
		return;
	}
	DEFER({
		std::fclose(file);
	});

	json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
}

void PatchManager::saveDialog() {
	if (!path.empty()) {
		save(path);
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

	osdialog_filters *filters = osdialog_filters_parse(PATCH_FILTERS.c_str());
	DEFER({
		osdialog_filters_free(filters);
	});

	char *pathC = osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), filters);
	if (!pathC) {
		// Fail silently
		return;
	}
	DEFER({
		free(pathC);
	});


	std::string pathStr = pathC;
	if (string::extension(pathStr).empty()) {
		pathStr += ".vcv";
	}

	save(pathStr);
	path = pathStr;
}

void PatchManager::saveTemplateDialog() {
	if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "Overwrite template patch?")) {
		save(asset::user("template.vcv"));
	}
}

void PatchManager::load(std::string path) {
	INFO("Loading patch %s", path.c_str());
	FILE *file = std::fopen(path.c_str(), "r");
	if (!file) {
		// Exit silently
		return;
	}
	DEFER({
		std::fclose(file);
	});

	json_error_t error;
	json_t *rootJ = json_loadf(file, 0, &error);
	if (!rootJ) {
		std::string message = string::f("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
		return;
	}
	DEFER({
		json_decref(rootJ);
	});

	app()->scene->rackWidget->clear();
	app()->scene->scrollWidget->offset = math::Vec(0, 0);
	fromJson(rootJ);
}

void PatchManager::loadDialog() {
	std::string dir;
	if (path.empty()) {
		dir = asset::user("patches");
		system::createDirectory(dir);
	}
	else {
		dir = string::directory(path);
	}

	osdialog_filters *filters = osdialog_filters_parse(PATCH_FILTERS.c_str());
	DEFER({
		osdialog_filters_free(filters);
	});

	char *pathC = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, filters);
	if (!pathC) {
		// Fail silently
		return;
	}
	DEFER({
		free(pathC);
	});

	load(pathC);
	path = pathC;
}

void PatchManager::revertDialog() {
	if (path.empty())
		return;
	if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "Revert patch to the last saved state?")) {
		load(path);
	}
}

void PatchManager::disconnectDialog() {
	if (!osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK_CANCEL, "Remove all patch cables?"))
		return;

	app()->scene->rackWidget->clear();
}

json_t *PatchManager::toJson() {
	// root
	json_t *rootJ = json_object();

	// version
	json_t *versionJ = json_string(APP_VERSION.c_str());
	json_object_set_new(rootJ, "version", versionJ);

	// Merge with RackWidget JSON
	json_t *rackJ = app()->scene->rackWidget->toJson();
	// Merge with rootJ
	json_object_update(rootJ, rackJ);
	json_decref(rackJ);

	return rootJ;
}

void PatchManager::fromJson(json_t *rootJ) {
	legacy = 0;

	// version
	std::string version;
	json_t *versionJ = json_object_get(rootJ, "version");
	if (versionJ)
		version = json_string_value(versionJ);
	if (version != APP_VERSION) {
		INFO("Patch made with Rack version %s, current Rack version is %s", version.c_str(), APP_VERSION.c_str());
	}

	// Detect old patches with ModuleWidget::params/inputs/outputs indices.
	// (We now use Module::params/inputs/outputs indices.)
	if (string::startsWith(version, "0.3.") || string::startsWith(version, "0.4.") || string::startsWith(version, "0.5.") || version == "" || version == "dev") {
		legacy = 1;
	}
	else if (string::startsWith(version, "0.6.")) {
		legacy = 2;
	}
	if (legacy) {
		INFO("Loading patch using legacy mode %d", legacy);
	}

	app()->scene->rackWidget->fromJson(rootJ);

	// Display a message if we have something to say
	if (!warningLog.empty()) {
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, warningLog.c_str());
	}
	warningLog = "";
}

bool PatchManager::isLegacy(int level) {
	return legacy && legacy <= level;
}


} // namespace rack
