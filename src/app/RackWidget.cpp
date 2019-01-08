#include <map>
#include <algorithm>
#include "app/RackWidget.hpp"
#include "app/RackRail.hpp"
#include "app/Scene.hpp"
#include "app/ModuleBrowser.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include "asset.hpp"
#include "system.hpp"
#include "plugin.hpp"
#include "app.hpp"


namespace rack {


struct ModuleContainer : Widget {
	void draw(NVGcontext *vg) override {
		// Draw shadows behind each ModuleWidget first, so the shadow doesn't overlap the front of other ModuleWidgets.
		for (Widget *child : children) {
			if (!child->visible)
				continue;
			nvgSave(vg);
			nvgTranslate(vg, child->box.pos.x, child->box.pos.y);
			ModuleWidget *w = dynamic_cast<ModuleWidget*>(child);
			assert(w);
			w->drawShadow(vg);
			nvgRestore(vg);
		}

		Widget::draw(vg);
	}
};


RackWidget::RackWidget() {
	rails = new FramebufferWidget;
	rails->box.size = math::Vec();
	rails->oversample = 1.0;
	{
		RackRail *rail = new RackRail;
		rail->box.size = math::Vec();
		rails->addChild(rail);
	}
	addChild(rails);

	moduleContainer = new ModuleContainer;
	addChild(moduleContainer);

	wireContainer = new WireContainer;
	addChild(wireContainer);
}

RackWidget::~RackWidget() {
}

void RackWidget::clear() {
	wireContainer->activeWire = NULL;
	wireContainer->clearChildren();
	moduleContainer->clearChildren();

	app()->scene->scrollWidget->offset = math::Vec(0, 0);
}

void RackWidget::reset() {
	if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "Clear patch and start over?")) {
		clear();
		// Fails silently if file does not exist
		load(asset::user("template.vcv"));
		lastPath = "";
	}
}

void RackWidget::loadDialog() {
	std::string dir;
	if (lastPath.empty()) {
		dir = asset::user("patches");
		system::createDirectory(dir);
	}
	else {
		dir = string::directory(lastPath);
	}

	osdialog_filters *filters = osdialog_filters_parse(PATCH_FILTERS.c_str());
	DEFER({
		osdialog_filters_free(filters);
	});

	char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, filters);
	if (!path) {
		// Fail silently
		return;
	}
	DEFER({
		free(path);
	});

	load(path);
	lastPath = path;
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
		dir = asset::user("patches");
		system::createDirectory(dir);
	}
	else {
		dir = string::directory(lastPath);
		filename = string::filename(lastPath);
	}

	osdialog_filters *filters = osdialog_filters_parse(PATCH_FILTERS.c_str());
	DEFER({
		osdialog_filters_free(filters);
	});

	char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), filters);
	if (!path) {
		// Fail silently
		return;
	}
	DEFER({
		free(path);
	});


	std::string pathStr = path;
	if (string::extension(pathStr).empty()) {
		pathStr += ".vcv";
	}

	save(pathStr);
	lastPath = pathStr;
}

void RackWidget::saveTemplate() {
	if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "Overwrite template patch?")) {
		save(asset::user("template.vcv"));
	}
}

void RackWidget::save(std::string filename) {
	INFO("Saving patch %s", filename.c_str());
	json_t *rootJ = toJson();
	if (!rootJ)
		return;
	DEFER({
		json_decref(rootJ);
	});

	FILE *file = fopen(filename.c_str(), "w");
	if (!file) {
		// Fail silently
		return;
	}
	DEFER({
		fclose(file);
	});

	json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
}

void RackWidget::load(std::string filename) {
	INFO("Loading patch %s", filename.c_str());
	FILE *file = fopen(filename.c_str(), "r");
	if (!file) {
		// Exit silently
		return;
	}
	DEFER({
		fclose(file);
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

	clear();
	fromJson(rootJ);
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

	wireContainer->removeAllWires(NULL);
}

json_t *RackWidget::toJson() {
	// root
	json_t *rootJ = json_object();

	// version
	json_t *versionJ = json_string(APP_VERSION.c_str());
	json_object_set_new(rootJ, "version", versionJ);

	// modules
	json_t *modulesJ = json_array();
	for (Widget *w : moduleContainer->children) {
		ModuleWidget *moduleWidget = dynamic_cast<ModuleWidget*>(w);
		assert(moduleWidget);
		// module
		json_t *moduleJ = moduleWidget->toJson();
		{
			// id
			json_object_set_new(moduleJ, "id", json_integer(moduleWidget->module->id));
			// pos
			math::Vec pos = moduleWidget->box.pos.div(RACK_GRID_SIZE).round();
			json_t *posJ = json_pack("[i, i]", (int) pos.x, (int) pos.y);
			json_object_set_new(moduleJ, "pos", posJ);
		}
		json_array_append_new(modulesJ, moduleJ);
	}
	json_object_set_new(rootJ, "modules", modulesJ);

	// wires
	json_t *wiresJ = json_array();
	for (Widget *w : wireContainer->children) {
		WireWidget *wireWidget = dynamic_cast<WireWidget*>(w);
		assert(wireWidget);

		PortWidget *outputPort = wireWidget->outputPort;
		PortWidget *inputPort = wireWidget->inputPort;
		// Only serialize WireWidgets connected on both ends
		if (!(outputPort && inputPort))
			continue;

		Wire *wire = wireWidget->wire;
		assert(wire);
		// wire
		json_t *wireJ = wireWidget->toJson();

		assert(outputPort->module);
		assert(inputPort->module);

		json_object_set_new(wireJ, "id", json_integer(wire->id));
		json_object_set_new(wireJ, "outputModuleId", json_integer(outputPort->module->id));
		json_object_set_new(wireJ, "outputId", json_integer(outputPort->portId));
		json_object_set_new(wireJ, "inputModuleId", json_integer(inputPort->module->id));
		json_object_set_new(wireJ, "inputId", json_integer(inputPort->portId));

		json_array_append_new(wiresJ, wireJ);
	}
	json_object_set_new(rootJ, "wires", wiresJ);

	return rootJ;
}

void RackWidget::fromJson(json_t *rootJ) {
	std::string message;

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
	int legacy = 0;
	if (string::startsWith(version, "0.3.") || string::startsWith(version, "0.4.") || string::startsWith(version, "0.5.") || version == "" || version == "dev") {
		legacy = 1;
	}
	else if (string::startsWith(version, "0.6.")) {
		legacy = 2;
	}
	if (legacy) {
		INFO("Loading patch using legacy mode %d", legacy);
	}

	// modules
	json_t *modulesJ = json_object_get(rootJ, "modules");
	if (!modulesJ)
		return;
	std::map<int, ModuleWidget*> moduleWidgets;
	size_t moduleIndex;
	json_t *moduleJ;
	json_array_foreach(modulesJ, moduleIndex, moduleJ) {
		// Add "legacy" property if in legacy mode
		if (legacy) {
			json_object_set(moduleJ, "legacy", json_integer(legacy));
		}

		ModuleWidget *moduleWidget = moduleFromJson(moduleJ);

		if (moduleWidget) {
			// id
			json_t *idJ = json_object_get(moduleJ, "id");
			int id = 0;
			if (idJ)
				id = json_integer_value(idJ);
			// pos
			json_t *posJ = json_object_get(moduleJ, "pos");
			double x, y;
			json_unpack(posJ, "[F, F]", &x, &y);
			math::Vec pos = math::Vec(x, y);
			if (legacy && legacy <= 1) {
				// Before 0.6, positions were in pixel units
				moduleWidget->box.pos = pos;
			}
			else {
				moduleWidget->box.pos = pos.mult(RACK_GRID_SIZE);
			}

			if (legacy && legacy <= 2) {
				// Before 1.0, the module ID was the index in the "modules" array
				moduleWidgets[moduleIndex] = moduleWidget;
			}
			else {
				moduleWidgets[id] = moduleWidget;
			}
			addModule(moduleWidget);
		}
		else {
			json_t *pluginSlugJ = json_object_get(moduleJ, "plugin");
			json_t *modelSlugJ = json_object_get(moduleJ, "model");
			std::string pluginSlug = json_string_value(pluginSlugJ);
			std::string modelSlug = json_string_value(modelSlugJ);
			message += string::f("Could not find module \"%s\" of plugin \"%s\"\n", modelSlug.c_str(), pluginSlug.c_str());
		}
	}

	// wires
	json_t *wiresJ = json_object_get(rootJ, "wires");
	assert(wiresJ);
	size_t wireIndex;
	json_t *wireJ;
	json_array_foreach(wiresJ, wireIndex, wireJ) {
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
		PortWidget *outputPort = NULL;
		PortWidget *inputPort = NULL;
		if (legacy && legacy <= 1) {
			// Before 0.6, the index of the "ports" array was the index of the PortWidget in the `outputs` and `inputs` vector.
			outputPort = outputModuleWidget->outputs[outputId];
			inputPort = inputModuleWidget->inputs[inputId];
		}
		else {
			for (PortWidget *port : outputModuleWidget->outputs) {
				if (port->portId == outputId) {
					outputPort = port;
					break;
				}
			}
			for (PortWidget *port : inputModuleWidget->inputs) {
				if (port->portId == inputId) {
					inputPort = port;
					break;
				}
			}
		}
		if (!outputPort || !inputPort)
			continue;

		// Create WireWidget
		WireWidget *wireWidget = new WireWidget;
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
	Model *model = plugin::getModel(pluginSlug, modelSlug);
	if (!model)
		return NULL;

	// Create ModuleWidget
	ModuleWidget *moduleWidget = model->createModuleWidget();
	assert(moduleWidget);
	moduleWidget->fromJson(moduleJ);
	return moduleWidget;
}

void RackWidget::pastePresetClipboard() {
	const char *moduleJson = glfwGetClipboardString(app()->window->win);
	if (!moduleJson) {
		WARN("Could not get text from clipboard.");
		return;
	}

	json_error_t error;
	json_t *moduleJ = json_loads(moduleJson, 0, &error);
	if (moduleJ) {
		ModuleWidget *moduleWidget = moduleFromJson(moduleJ);
		json_decref(moduleJ);
		addModule(moduleWidget);
		// Set moduleWidget position
		math::Rect newBox = moduleWidget->box;
		newBox.pos = lastMousePos.minus(newBox.size.div(2));
		requestModuleBoxNearest(moduleWidget, newBox);
	}
	else {
		WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
	}
}

void RackWidget::addModule(ModuleWidget *m) {
	moduleContainer->addChild(m);
}

void RackWidget::addModuleAtMouse(ModuleWidget *m) {
	addModule(m);
	// Move module nearest to the mouse position
	m->box.pos = lastMousePos.minus(m->box.size.div(2));
	requestModuleBoxNearest(m, m->box);
}

void RackWidget::deleteModule(ModuleWidget *m) {
	moduleContainer->removeChild(m);
}

void RackWidget::cloneModule(ModuleWidget *m) {
	// JSON serialization is the obvious way to do this
	json_t *moduleJ = m->toJson();
	ModuleWidget *clonedModuleWidget = moduleFromJson(moduleJ);
	json_decref(moduleJ);
	addModule(clonedModuleWidget);
	math::Rect clonedBox = clonedModuleWidget->box;
	clonedBox.pos = m->box.pos;
	requestModuleBoxNearest(clonedModuleWidget, clonedBox);
}

bool RackWidget::requestModuleBox(ModuleWidget *m, math::Rect box) {
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

bool RackWidget::requestModuleBoxNearest(ModuleWidget *m, math::Rect box) {
	// Create possible positions
	int x0 = roundf(box.pos.x / RACK_GRID_WIDTH);
	int y0 = roundf(box.pos.y / RACK_GRID_HEIGHT);
	std::vector<math::Vec> positions;
	for (int y = std::max(0, y0 - 8); y < y0 + 8; y++) {
		for (int x = std::max(0, x0 - 400); x < x0 + 400; x++) {
			positions.push_back(math::Vec(x * RACK_GRID_WIDTH, y * RACK_GRID_HEIGHT));
		}
	}

	// Sort possible positions by distance to the requested position
	std::sort(positions.begin(), positions.end(), [box](math::Vec a, math::Vec b) {
		return a.minus(box.pos).norm() < b.minus(box.pos).norm();
	});

	// Find a position that does not collide
	for (math::Vec position : positions) {
		math::Rect newBox = box;
		newBox.pos = position;
		if (requestModuleBox(m, newBox))
			return true;
	}
	return false;
}

void RackWidget::step() {
	// Expand size to fit modules
	math::Vec moduleSize = moduleContainer->getChildrenBoundingBox().getBottomRight();
	// We assume that the size is reset by a parent before calling step(). Otherwise it will grow unbounded.
	box.size = box.size.max(moduleSize);

	// Adjust size and position of rails
	Widget *rail = rails->children.front();
	math::Rect bound = getViewport(math::Rect(math::Vec(), box.size));
	if (!rails->box.contains(bound)) {
		math::Vec cellMargin = math::Vec(20, 1);
		rails->box.pos = bound.pos.div(RACK_GRID_SIZE).floor().minus(cellMargin).mult(RACK_GRID_SIZE);
		rails->box.size = bound.size.plus(cellMargin.mult(RACK_GRID_SIZE).mult(2));
		rails->dirty = true;

		rail->box.size = rails->box.size;
	}

	// Autosave every 15 seconds
	int frame = app()->window->frame;
	if (frame > 0 && frame % (60 * 15) == 0) {
		save(asset::user("autosave.vcv"));
		settings::save(asset::user("settings.json"));
	}

	Widget::step();
}

void RackWidget::draw(NVGcontext *vg) {
	Widget::draw(vg);
}

void RackWidget::onHover(const event::Hover &e) {
	OpaqueWidget::onHover(e);
	lastMousePos = e.pos;
}

void RackWidget::onDragHover(const event::DragHover &e) {
	OpaqueWidget::onDragHover(e);
	lastMousePos = e.pos;
}

void RackWidget::onButton(const event::Button &e) {
	OpaqueWidget::onButton(e);
	if (e.getConsumed() == this) {
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
			app()->scene->moduleBrowser->visible = true;
		}
	}
}

void RackWidget::onZoom(const event::Zoom &e) {
	rails->box.size = math::Vec();
	OpaqueWidget::onZoom(e);
}


} // namespace rack
