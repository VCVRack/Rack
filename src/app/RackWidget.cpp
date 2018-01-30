#include "app.hpp"
#include "engine.hpp"
#include "plugin.hpp"
#include "gui.hpp"
#include "settings.hpp"
#include "asset.hpp"
#include <map>
#include <algorithm>
#include "../ext/osdialog/osdialog.h"


namespace rack {


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

	moduleContainer = new Widget();
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
	lastPath = "";

/*
	// Add all modules to rack
	Vec pos;
	for (Plugin *plugin : gPlugins) {
		for (Model *model : plugin->models) {
			ModuleWidget *moduleWidget = model->createModuleWidget();
			moduleContainer->addChild(moduleWidget);
			// Move module nearest to the mouse position
			Rect box;
			box.size = moduleWidget->box.size;
			box.pos = pos;
			requestModuleBoxNearest(moduleWidget, box);
			pos.x += box.size.x;
		}
		pos.y += RACK_GRID_HEIGHT;
		pos.x = 0;
	}
*/
}

void RackWidget::reset() {
	if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "Clear your patch and start over?")) {
		clear();
		loadPatch(assetLocal("template.vcv"));
	}
}

void RackWidget::openDialog() {
	std::string dir = lastPath.empty() ? assetLocal("") : extractDirectory(lastPath);
	char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, NULL);
	if (path) {
		loadPatch(path);
		lastPath = path;
		free(path);
	}
}

void RackWidget::saveDialog() {
	if (!lastPath.empty()) {
		savePatch(lastPath);
	}
	else {
		saveAsDialog();
	}
}

void RackWidget::saveAsDialog() {
	std::string dir = lastPath.empty() ? assetLocal("") : extractDirectory(lastPath);
	char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), "Untitled.vcv", NULL);

	if (path) {
		std::string pathStr = path;
		free(path);
		std::string extension = extractExtension(pathStr);
		if (extension.empty()) {
			pathStr += ".vcv";
		}

		savePatch(pathStr);
		lastPath = pathStr;
	}
}

void RackWidget::savePatch(std::string path) {
	info("Saving patch %s", path.c_str());
	json_t *rootJ = toJson();
	if (!rootJ)
		return;

	FILE *file = fopen(path.c_str(), "w");
	if (file) {
		json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
		fclose(file);
	}

	json_decref(rootJ);
}

void RackWidget::loadPatch(std::string path) {
	info("Loading patch %s", path.c_str());
	FILE *file = fopen(path.c_str(), "r");
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
		std::string message = stringf("JSON parsing error at %s %d:%d %s\n", error.source, error.line, error.column, error.text);
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
	}

	fclose(file);
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
		if (!version.empty() && gApplicationVersion != version)
			message += stringf("This patch was created with Rack %s. Saving it will convert it to a Rack %s patch.\n\n", version.c_str(), gApplicationVersion.c_str());
	}

	// Detect old patches with ModuleWidget::params/inputs/outputs indices.
	// (We now use Module::params/inputs/outputs indices.)
	int legacy = 0;
	if (startsWith(version, "0.3.") || startsWith(version, "0.4.") || startsWith(version, "0.5.") || version == "" || version == "dev") {
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
		// Set legacy property
		if (legacy)
			json_object_set_new(moduleJ, "legacy", json_integer(legacy));

		json_t *pluginSlugJ = json_object_get(moduleJ, "plugin");
		if (!pluginSlugJ) continue;
		json_t *modelSlugJ = json_object_get(moduleJ, "model");
		if (!modelSlugJ) continue;
		std::string pluginSlug = json_string_value(pluginSlugJ);
		std::string modelSlug = json_string_value(modelSlugJ);

		// Search for plugin
		Plugin *plugin = NULL;
		for (Plugin *p : gPlugins) {
			if (p->slug == pluginSlug) {
				plugin = p;
				break;
			}
		}

		if (!plugin) {
			message += stringf("Could not find plugin \"%s\" for module \"%s\"\n", pluginSlug.c_str(), modelSlug.c_str());
			continue;
		}

		// Search for model
		Model *model = NULL;
		for (Model *m : plugin->models) {
			if (m->slug == modelSlug) {
				model = m;
				break;
			}
		}

		if (!model) {
			message += stringf("Could not find module \"%s\" in plugin \"%s\"\n", modelSlug.c_str(), pluginSlug.c_str());
			continue;
		}

		// Create ModuleWidget
		ModuleWidget *moduleWidget = model->createModuleWidget();
		assert(moduleWidget);
		moduleWidget->fromJson(moduleJ);
		moduleContainer->addChild(moduleWidget);
		moduleWidgets[moduleId] = moduleWidget;
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
		osdialog_message(OSDIALOG_INFO, OSDIALOG_OK, message.c_str());
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
	// Create new module from model
	ModuleWidget *clonedModuleWidget = m->model->createModuleWidget();
	// JSON serialization is the most straightforward way to do this
	json_t *moduleJ = m->toJson();
	clonedModuleWidget->fromJson(moduleJ);
	json_decref(moduleJ);
	Rect clonedBox = clonedModuleWidget->box;
	clonedBox.pos = m->box.pos;
	requestModuleBoxNearest(clonedModuleWidget, clonedBox);
	addModule(clonedModuleWidget);
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
	for (int y = maxi(0, y0 - 8); y < y0 + 8; y++) {
		for (int x = maxi(0, x0 - 400); x < x0 + 400; x++) {
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
		savePatch(assetLocal("autosave.vcv"));
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
		MenuOverlay *overlay = new MenuOverlay();

		AddModuleWindow *window = new AddModuleWindow();
		// Set center position
		window->box.pos = gMousePos.minus(window->box.getCenter());
		window->modulePos = lastMousePos;

		overlay->addChild(window);
		gScene->setOverlay(overlay);
	}
	e.consumed = true;
	e.target = this;
}

void RackWidget::onZoom(EventZoom &e) {
	rails->box.size = Vec();
	Widget::onZoom(e);
}


} // namespace rack
