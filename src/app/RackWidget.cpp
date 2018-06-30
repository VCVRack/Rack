#include "app.hpp"
#include "engine.hpp"
#include "plugin.hpp"
#include "window.hpp"
#include "settings.hpp"
#include "asset.hpp"
#include <map>
#include <algorithm>
#include "osdialog.h"


namespace rack {


struct ModuleContainer : Widget {
	void draw(NVGcontext *vg) override {
		// Draw shadows behind each ModuleWidget first, so the shadow doesn't overlap the front.
		for (Widget *child : children) {
			if (!child->visible)
				continue;
			nvgSave(vg);
			nvgTranslate(vg, child->box.pos.x, child->box.pos.y);
			ModuleWidget *w = dynamic_cast<ModuleWidget*>(child);
			w->drawShadow(vg);
			nvgRestore(vg);
		}

		Widget::draw(vg);
	}
};


RackWidget::RackWidget() {
	rails = new FramebufferWidget();
	rails->box.size = Vec();
	rails->oversample = 1.0;
	{
		RackRail *rail = new RackRail();
		rail->box.size = Vec();
		rails->addChild(rail);
	}
	addChild(rails);

	moduleContainer = new ModuleContainer();
	addChild(moduleContainer);

	wireContainer = new WireContainer();
	addChild(wireContainer);
}

RackWidget::~RackWidget() {
}

void RackWidget::clear() {
	wireContainer->activeWire = NULL;
	wireContainer->clearChildren();
	moduleContainer->clearChildren();

	gRackScene->scrollWidget->offset = Vec(0, 0);
}

void RackWidget::reset() {
	if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "Clear patch and start over?")) {
		clear();
		// Fails silently if file does not exist
		load(assetLocal("template.vcv"));
		lastPath = "";
	}
}

void RackWidget::loadDialog() {
	std::string dir;
	if (lastPath.empty()) {
		dir = assetLocal("patches");
		systemCreateDirectory(dir);
	}
	else {
		dir = stringDirectory(lastPath);
	}
	osdialog_filters *filters = osdialog_filters_parse(PATCH_FILTERS.c_str());
	char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, filters);
	if (path) {
		load(path);
		lastPath = path;
		free(path);
	}
	osdialog_filters_free(filters);
}

void RackWidget::saveDialog() {
	if (!lastPath.empty()) {
		save(lastPath);
	}
	else {
		saveAsDialog();
	}
}

void RackWidget::saveAsDialog() {
	std::string dir;
	std::string filename;
	if (lastPath.empty()) {
		dir = assetLocal("patches");
		systemCreateDirectory(dir);
	}
	else {
		dir = stringDirectory(lastPath);
		filename = stringFilename(lastPath);
	}
	osdialog_filters *filters = osdialog_filters_parse(PATCH_FILTERS.c_str());
	char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), filters);

	if (path) {
		std::string pathStr = path;
		free(path);
		std::string extension = stringExtension(pathStr);
		if (extension.empty()) {
			pathStr += ".vcv";
		}

		save(pathStr);
		lastPath = pathStr;
	}
	osdialog_filters_free(filters);
}

void RackWidget::save(std::string filename) {
	info("Saving patch %s", filename.c_str());
	json_t *rootJ = toJson();
	if (!rootJ)
		return;

	FILE *file = fopen(filename.c_str(), "w");
	if (file) {
		json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
		fclose(file);
	}

	json_decref(rootJ);
}

void RackWidget::load(std::string filename) {
	info("Loading patch %s", filename.c_str());
	FILE *file = fopen(filename.c_str(), "r");
	if (!file) {
		// Exit silently
		return;
	}

	json_error_t error;
	json_t *rootJ = json_loadf(file, 0, &error);
	if (rootJ) {
		clear();
		fromJson(rootJ);
		json_decref(rootJ);
	}
	else {
		std::string message = stringf("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
	}

	fclose(file);
}

void RackWidget::revert() {
	if (lastPath.empty())
		return;
	if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "Revert patch to the last saved state?")) {
		load(lastPath);
	}
}

void RackWidget::disconnect() {
	if (!osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK_CANCEL, "Remove all patch cables?"))
		return;

	for (Widget *w : moduleContainer->children) {
		ModuleWidget *moduleWidget = dynamic_cast<ModuleWidget*>(w);
		assert(moduleWidget);
		moduleWidget->disconnect();
	}
}

json_t *RackWidget::toJson() {
	// root
	json_t *rootJ = json_object();

	// version
	json_t *versionJ = json_string(gApplicationVersion.c_str());
	json_object_set_new(rootJ, "version", versionJ);

	// modules
	json_t *modulesJ = json_array();
	std::map<ModuleWidget*, int> moduleIds;
	int moduleId = 0;
	for (Widget *w : moduleContainer->children) {
		ModuleWidget *moduleWidget = dynamic_cast<ModuleWidget*>(w);
		assert(moduleWidget);
		moduleIds[moduleWidget] = moduleId;
		moduleId++;
		// module
		json_t *moduleJ = moduleWidget->toJson();
		{
			// pos
			Vec pos = moduleWidget->box.pos.div(RACK_GRID_SIZE).round();
			json_t *posJ = json_pack("[i, i]", (int) pos.x, (int) pos.y);
			json_object_set_new(moduleJ, "pos", posJ);
		}
		json_array_append_new(modulesJ, moduleJ);
	}
	json_object_set_new(rootJ, "modules", modulesJ);

	// wires
	json_t *wires = json_array();
	for (Widget *w : wireContainer->children) {
		WireWidget *wireWidget = dynamic_cast<WireWidget*>(w);
		assert(wireWidget);
		// Only serialize WireWidgets connected on both ends
		if (!(wireWidget->outputPort && wireWidget->inputPort))
			continue;
		// wire
		json_t *wire = wireWidget->toJson();

		// Get the modules at each end of the wire
		ModuleWidget *outputModuleWidget = wireWidget->outputPort->getAncestorOfType<ModuleWidget>();
		assert(outputModuleWidget);
		int outputModuleId = moduleIds[outputModuleWidget];

		ModuleWidget *inputModuleWidget = wireWidget->inputPort->getAncestorOfType<ModuleWidget>();
		assert(inputModuleWidget);
		int inputModuleId = moduleIds[inputModuleWidget];

		// Get output/input ports
		int outputId = wireWidget->outputPort->portId;
		int inputId = wireWidget->inputPort->portId;

		json_object_set_new(wire, "outputModuleId", json_integer(outputModuleId));
		json_object_set_new(wire, "outputId", json_integer(outputId));
		json_object_set_new(wire, "inputModuleId", json_integer(inputModuleId));
		json_object_set_new(wire, "inputId", json_integer(inputId));

		json_array_append_new(wires, wire);
	}
	json_object_set_new(rootJ, "wires", wires);

	return rootJ;
}

void RackWidget::fromJson(json_t *rootJ) {
	std::string message;

	// version
	std::string version;
	json_t *versionJ = json_object_get(rootJ, "version");
	if (versionJ) {
		version = json_string_value(versionJ);
	}

	// Detect old patches with ModuleWidget::params/inputs/outputs indices.
	// (We now use Module::params/inputs/outputs indices.)
	int legacy = 0;
	if (stringStartsWith(version, "0.3.") || stringStartsWith(version, "0.4.") || stringStartsWith(version, "0.5.") || version == "" || version == "dev") {
		legacy = 1;
	}
	if (legacy) {
		info("Loading patch using legacy mode %d", legacy);
	}

	// modules
	std::map<int, ModuleWidget*> moduleWidgets;
	json_t *modulesJ = json_object_get(rootJ, "modules");
	if (!modulesJ) return;
	size_t moduleId;
	json_t *moduleJ;
	json_array_foreach(modulesJ, moduleId, moduleJ) {
		// Add "legacy" property if in legacy mode
		if (legacy) {
			json_object_set(moduleJ, "legacy", json_integer(legacy));
		}

		ModuleWidget *moduleWidget = moduleFromJson(moduleJ);

		if (moduleWidget) {
			// pos
			json_t *posJ = json_object_get(moduleJ, "pos");
			double x, y;
			json_unpack(posJ, "[F, F]", &x, &y);
			Vec pos = Vec(x, y);
			if (legacy && legacy <= 1) {
				moduleWidget->box.pos = pos;
			}
			else {
				moduleWidget->box.pos = pos.mult(RACK_GRID_SIZE);
			}

			moduleWidgets[moduleId] = moduleWidget;
		}
		else {
			json_t *pluginSlugJ = json_object_get(moduleJ, "plugin");
			json_t *modelSlugJ = json_object_get(moduleJ, "model");
			std::string pluginSlug = json_string_value(pluginSlugJ);
			std::string modelSlug = json_string_value(modelSlugJ);
			message += stringf("Could not find module \"%s\" of plugin \"%s\"\n", modelSlug.c_str(), pluginSlug.c_str());
		}
	}

	// wires
	json_t *wiresJ = json_object_get(rootJ, "wires");
	if (!wiresJ) return;
	size_t wireId;
	json_t *wireJ;
	json_array_foreach(wiresJ, wireId, wireJ) {
		int outputModuleId = json_integer_value(json_object_get(wireJ, "outputModuleId"));
		int outputId = json_integer_value(json_object_get(wireJ, "outputId"));
		int inputModuleId = json_integer_value(json_object_get(wireJ, "inputModuleId"));
		int inputId = json_integer_value(json_object_get(wireJ, "inputId"));

		// Get module widgets
		ModuleWidget *outputModuleWidget = moduleWidgets[outputModuleId];
		if (!outputModuleWidget) continue;
		ModuleWidget *inputModuleWidget = moduleWidgets[inputModuleId];
		if (!inputModuleWidget) continue;

		// Get port widgets
		Port *outputPort = NULL;
		Port *inputPort = NULL;
		if (legacy && legacy <= 1) {
			// Legacy 1 mode
			// The index of the "ports" array is the index of the Port in the `outputs` and `inputs` vector.
			outputPort = outputModuleWidget->outputs[outputId];
			inputPort = inputModuleWidget->inputs[inputId];
		}
		else {
			for (Port *port : outputModuleWidget->outputs) {
				if (port->portId == outputId) {
					outputPort = port;
					break;
				}
			}
			for (Port *port : inputModuleWidget->inputs) {
				if (port->portId == inputId) {
					inputPort = port;
					break;
				}
			}
		}
		if (!outputPort || !inputPort)
			continue;

		// Create WireWidget
		WireWidget *wireWidget = new WireWidget();
		wireWidget->fromJson(wireJ);
		wireWidget->outputPort = outputPort;
		wireWidget->inputPort = inputPort;
		wireWidget->updateWire();
		// Add wire to rack
		wireContainer->addChild(wireWidget);
	}

	// Display a message if we have something to say
	if (!message.empty()) {
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
	}
}

ModuleWidget *RackWidget::moduleFromJson(json_t *moduleJ) {
	// Get slugs
	json_t *pluginSlugJ = json_object_get(moduleJ, "plugin");
	if (!pluginSlugJ)
		return NULL;
	json_t *modelSlugJ = json_object_get(moduleJ, "model");
	if (!modelSlugJ)
		return NULL;
	std::string pluginSlug = json_string_value(pluginSlugJ);
	std::string modelSlug = json_string_value(modelSlugJ);

	// Get Model
	Model *model = pluginGetModel(pluginSlug, modelSlug);
	if (!model)
		return NULL;

	// Create ModuleWidget
	ModuleWidget *moduleWidget = model->createModuleWidget();
	assert(moduleWidget);
	moduleWidget->fromJson(moduleJ);
	moduleContainer->addChild(moduleWidget);
	return moduleWidget;
}

void RackWidget::pastePresetClipboard() {
	const char *moduleJson = glfwGetClipboardString(gWindow);
	if (!moduleJson) {
		warn("Could not get text from clipboard.");
		return;
	}

	json_error_t error;
	json_t *moduleJ = json_loads(moduleJson, 0, &error);
	if (moduleJ) {
		ModuleWidget *moduleWidget = moduleFromJson(moduleJ);
		// Set moduleWidget position
		Rect newBox = moduleWidget->box;
		newBox.pos = lastMousePos.minus(newBox.size.div(2));
		requestModuleBoxNearest(moduleWidget, newBox);

		json_decref(moduleJ);
	}
	else {
		warn("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
	}
}

void RackWidget::addModule(ModuleWidget *m) {
	moduleContainer->addChild(m);
	m->create();
}

void RackWidget::deleteModule(ModuleWidget *m) {
	m->_delete();
	moduleContainer->removeChild(m);
}

void RackWidget::cloneModule(ModuleWidget *m) {
	// JSON serialization is the most straightforward way to do this
	json_t *moduleJ = m->toJson();
	ModuleWidget *clonedModuleWidget = moduleFromJson(moduleJ);
	json_decref(moduleJ);
	Rect clonedBox = clonedModuleWidget->box;
	clonedBox.pos = m->box.pos;
	requestModuleBoxNearest(clonedModuleWidget, clonedBox);
}

bool RackWidget::requestModuleBox(ModuleWidget *m, Rect box) {
	if (box.pos.x < 0 || box.pos.y < 0)
		return false;

	for (Widget *child2 : moduleContainer->children) {
		if (m == child2) continue;
		if (box.intersects(child2->box)) {
			return false;
		}
	}
	m->box = box;
	return true;
}

bool RackWidget::requestModuleBoxNearest(ModuleWidget *m, Rect box) {
	// Create possible positions
	int x0 = roundf(box.pos.x / RACK_GRID_WIDTH);
	int y0 = roundf(box.pos.y / RACK_GRID_HEIGHT);
	std::vector<Vec> positions;
	for (int y = max(0, y0 - 8); y < y0 + 8; y++) {
		for (int x = max(0, x0 - 400); x < x0 + 400; x++) {
			positions.push_back(Vec(x * RACK_GRID_WIDTH, y * RACK_GRID_HEIGHT));
		}
	}

	// Sort possible positions by distance to the requested position
	std::sort(positions.begin(), positions.end(), [box](Vec a, Vec b) {
		return a.minus(box.pos).norm() < b.minus(box.pos).norm();
	});

	// Find a position that does not collide
	for (Vec position : positions) {
		Rect newBox = box;
		newBox.pos = position;
		if (requestModuleBox(m, newBox))
			return true;
	}
	return false;
}

void RackWidget::step() {
	// Expand size to fit modules
	Vec moduleSize = moduleContainer->getChildrenBoundingBox().getBottomRight();
	// We assume that the size is reset by a parent before calling step(). Otherwise it will grow unbounded.
	box.size = box.size.max(moduleSize);

	// Adjust size and position of rails
	Widget *rail = rails->children.front();
	Rect bound = getViewport(Rect(Vec(), box.size));
	if (!rails->box.contains(bound)) {
		Vec cellMargin = Vec(20, 1);
		rails->box.pos = bound.pos.div(RACK_GRID_SIZE).floor().minus(cellMargin).mult(RACK_GRID_SIZE);
		rails->box.size = bound.size.plus(cellMargin.mult(RACK_GRID_SIZE).mult(2));
		rails->dirty = true;

		rail->box.size = rails->box.size;
	}

	// Autosave every 15 seconds
	if (gGuiFrame % (60 * 15) == 0) {
		save(assetLocal("autosave.vcv"));
		settingsSave(assetLocal("settings.json"));
	}

	Widget::step();
}

void RackWidget::draw(NVGcontext *vg) {
	Widget::draw(vg);
}

void RackWidget::onMouseMove(EventMouseMove &e) {
	OpaqueWidget::onMouseMove(e);
	lastMousePos = e.pos;
}

void RackWidget::onMouseDown(EventMouseDown &e) {
	Widget::onMouseDown(e);
	if (e.consumed)
		return;

	if (e.button == 1) {
		appModuleBrowserCreate();
	}
	e.consumed = true;
	e.target = this;
}

void RackWidget::onZoom(EventZoom &e) {
	rails->box.size = Vec();
	Widget::onZoom(e);
}


} // namespace rack
