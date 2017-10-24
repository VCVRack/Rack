#include "app.hpp"
#include "engine.hpp"
#include "plugin.hpp"
#include "gui.hpp"
#include "settings.hpp"
#include "asset.hpp"
#include <map>
#include <algorithm>
#include <thread>
#include <set>
#include "../ext/osdialog/osdialog.h"


namespace rack {


RackWidget::RackWidget() {
	rails = new FramebufferWidget();
	RackRail *rail = new RackRail();
	rail->box.size = Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	rails->addChild(rail);
	rails->box.size = rail->box.size;

	moduleContainer = new Widget();
	addChild(moduleContainer);

	wireContainer = new WireContainer();
	addChild(wireContainer);
}

RackWidget::~RackWidget() {
	delete rails;
}

void RackWidget::clear() {
	wireContainer->activeWire = NULL;
	wireContainer->clearChildren();
	moduleContainer->clearChildren();
	lastPath = "";
}

void RackWidget::reset() {
	clear();
	loadPatch(assetLocal("template.vcv"));
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
	printf("Saving patch %s\n", path.c_str());
	FILE *file = fopen(path.c_str(), "w");
	if (!file)
		return;

	json_t *rootJ = toJson();
	if (rootJ) {
		json_dumpf(rootJ, file, JSON_INDENT(2));
		json_decref(rootJ);
	}

	fclose(file);
}

void RackWidget::loadPatch(std::string path) {
	printf("Loading patch %s\n", path.c_str());
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
		json_t *wire = json_object();
		{
			// Get the modules at each end of the wire
			ModuleWidget *outputModuleWidget = wireWidget->outputPort->getAncestorOfType<ModuleWidget>();
			assert(outputModuleWidget);
			int outputModuleId = moduleIds[outputModuleWidget];

			ModuleWidget *inputModuleWidget = wireWidget->inputPort->getAncestorOfType<ModuleWidget>();
			assert(inputModuleWidget);
			int inputModuleId = moduleIds[inputModuleWidget];

			// Get output/input ports
			auto outputIt = std::find(outputModuleWidget->outputs.begin(), outputModuleWidget->outputs.end(), wireWidget->outputPort);
			assert(outputIt != outputModuleWidget->outputs.end());
			int outputId = outputIt - outputModuleWidget->outputs.begin();

			auto inputIt = std::find(inputModuleWidget->inputs.begin(), inputModuleWidget->inputs.end(), wireWidget->inputPort);
			assert(inputIt != inputModuleWidget->inputs.end());
			int inputId = inputIt - inputModuleWidget->inputs.begin();

			json_object_set_new(wire, "outputModuleId", json_integer(outputModuleId));
			json_object_set_new(wire, "outputId", json_integer(outputId));
			json_object_set_new(wire, "inputModuleId", json_integer(inputModuleId));
			json_object_set_new(wire, "inputId", json_integer(inputId));
		}
		json_array_append_new(wires, wire);
	}
	json_object_set_new(rootJ, "wires", wires);

	return rootJ;
}

void RackWidget::fromJson(json_t *rootJ) {
	std::string message;

	// version
	json_t *versionJ = json_object_get(rootJ, "version");
	if (versionJ) {
		std::string version = json_string_value(versionJ);
		if (!version.empty() && gApplicationVersion != version)
			message += stringf("This patch was created with Rack %s. Saving it will convert it to a Rack %s patch.\n\n", version.c_str(), gApplicationVersion.empty() ? "dev" : gApplicationVersion.c_str());
	}

	// modules
	std::map<int, ModuleWidget*> moduleWidgets;
	json_t *modulesJ = json_object_get(rootJ, "modules");
	if (!modulesJ) return;
	size_t moduleId;
	json_t *moduleJ;
	json_array_foreach(modulesJ, moduleId, moduleJ) {
		json_t *manufacturerSlugJ = json_object_get(moduleJ, "manufacturer");
		if (!manufacturerSlugJ) {
			// Backward compatibility with Rack v0.4 and lower
			manufacturerSlugJ = json_object_get(moduleJ, "plugin");
			if (!manufacturerSlugJ) continue;
		}
		json_t *modelSlugJ = json_object_get(moduleJ, "model");
		if (!modelSlugJ) continue;
		std::string manufacturerSlug = json_string_value(manufacturerSlugJ);
		std::string modelSlug = json_string_value(modelSlugJ);

		// Search for model
		Model *model = NULL;
		for (Plugin *plugin : gPlugins) {
			for (Model *m : plugin->models) {
				if (m->manufacturerSlug == manufacturerSlug && m->slug == modelSlug) {
					model = m;
				}
			}
		}

		if (!model) {
			message += stringf("Could not find \"%s %s\" module\n", manufacturerSlug.c_str(), modelSlug.c_str());
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
		int outputModuleId, outputId;
		int inputModuleId, inputId;
		int err = json_unpack(wireJ, "{s:i, s:i, s:i, s:i}",
			"outputModuleId", &outputModuleId, "outputId", &outputId,
			"inputModuleId", &inputModuleId, "inputId", &inputId);
		if (err) continue;
		// Get ports
		ModuleWidget *outputModuleWidget = moduleWidgets[outputModuleId];
		if (!outputModuleWidget) continue;
		Port *outputPort = outputModuleWidget->outputs[outputId];
		if (!outputPort) continue;
		ModuleWidget *inputModuleWidget = moduleWidgets[inputModuleId];
		if (!inputModuleWidget) continue;
		Port *inputPort = inputModuleWidget->inputs[inputId];
		if (!inputPort) continue;
		// Create WireWidget
		WireWidget *wireWidget = new WireWidget();
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
}

void RackWidget::deleteModule(ModuleWidget *m) {
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
	for (int y = maxi(0, y0 - 4); y < y0 + 4; y++) {
		for (int x = maxi(0, x0 - 200); x < x0 + 200; x++) {
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
	rails->step();

	// Expand size to fit modules
	Vec moduleSize = moduleContainer->getChildrenBoundingBox().getBottomRight();
	// We assume that the size is reset by a parent before calling step(). Otherwise it will grow unbounded.
	box.size = box.size.max(moduleSize);

	// Autosave every 15 seconds
	if (gGuiFrame % (60*15) == 0) {
		savePatch(assetLocal("autosave.vcv"));
		settingsSave(assetLocal("settings.json"));
	}

	Widget::step();
}

void RackWidget::draw(NVGcontext *vg) {
	// Draw rails
	nvgBeginPath(vg);
	nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);
	NVGpaint paint = nvgImagePattern(vg, rails->box.pos.x, rails->box.pos.y, rails->box.size.x, rails->box.size.y, 0.0, rails->getImageHandle(), 1.0);
	nvgFillPaint(vg, paint);
	nvgFill(vg);

	Widget::draw(vg);
}

struct AddModuleMenuItem : MenuItem {
	Model *model;
	Vec modulePos;
	void onAction() override {
		ModuleWidget *moduleWidget = model->createModuleWidget();
		gRackWidget->moduleContainer->addChild(moduleWidget);
		// Move module nearest to the mouse position
		Rect box;
		box.size = moduleWidget->box.size;
		box.pos = modulePos.minus(box.getCenter());
		gRackWidget->requestModuleBoxNearest(moduleWidget, box);
	}
};

struct UrlItem : MenuItem {
	std::string url;
	void onAction() override {
		std::thread t(openBrowser, url);
		t.detach();
	}
};

struct AddManufacturerMenuItem : MenuItem {
	std::string manufacturerName;
	Vec modulePos;
	Menu *createChildMenu() override {
		// Collect models which have this manufacturer name
		std::set<Model*> models;
		for (Plugin *plugin : gPlugins) {
			for (Model *model : plugin->models) {
				if (model->manufacturerName == manufacturerName) {
					models.insert(model);
				}
			}
		}

		if (models.empty())
			return NULL;

		// Model items
		Menu *menu = new Menu();
		for (Model *model : models) {
			AddModuleMenuItem *item = new AddModuleMenuItem();
			item->text = model->name;
			// item->rightText = model->plugin->slug;
			// if (!model->plugin->version.empty())
			// 	item->rightText += " v" + model->plugin->version;
			item->model = model;
			item->modulePos = modulePos;
			menu->pushChild(item);
		}

		// Metadata items
		/*
		{
			MenuLabel *label = new MenuLabel();
			menu->pushChild(label);
		}
		{
			MenuLabel *label = new MenuLabel();
			label->text = plugin->name;
			menu->pushChild(label);
		}

		if (!plugin->homepageUrl.empty()) {
			UrlItem *item = new UrlItem();
			item->text = "Homepage";
			item->url = plugin->homepageUrl;
			menu->pushChild(item);
		}

		if (!plugin->manualUrl.empty()) {
			UrlItem *item = new UrlItem();
			item->text = "Manual";
			item->url = plugin->manualUrl;
			menu->pushChild(item);
		}

		if (!plugin->path.empty()) {
			UrlItem *item = new UrlItem();
			item->text = "Browse directory";
			item->url = plugin->path;
			menu->pushChild(item);
		}

		if (!plugin->version.empty()) {
			MenuLabel *item = new MenuLabel();
			item->text = "Version: v" + plugin->version;
			menu->pushChild(item);
		}
		*/

		return menu;
	}
};

struct SearchModuleField : TextField {
	void onTextChange() override {
		Menu *parentMenu = getAncestorOfType<Menu>();
		assert(parentMenu);

		for (Widget *w : parentMenu->children) {
			AddManufacturerMenuItem *a = dynamic_cast<AddManufacturerMenuItem*>(w);
			if (!a)
				continue;
			if (a->manufacturerName == text) {
				a->visible = true;
			}
			else {
				a->visible = false;
			}
		}
	}
};

void RackWidget::onMouseDownOpaque(int button) {
	if (button == 1) {
		Vec modulePos = gMousePos.minus(getAbsolutePos());
		Menu *menu = gScene->createMenu();

		menu->pushChild(construct<MenuLabel>(&MenuLabel::text, "Add module"));

		/*
		// TODO make functional
		TextField *searchField = construct<SearchModuleField>();
		searchField->box.size.x = 100.0;
		menu->pushChild(searchField);
		// Focus search field
		gFocusedWidget = searchField;
		*/

		// Collect manufacturer names
		std::set<std::string> manufacturerNames;
		for (Plugin *plugin : gPlugins) {
			for (Model *model : plugin->models) {
				manufacturerNames.insert(model->manufacturerName);
			}
		}
		// Add menu item for each manufacturer name
		for (std::string manufacturerName : manufacturerNames) {
			AddManufacturerMenuItem *item = new AddManufacturerMenuItem();
			item->text = manufacturerName;
			item->manufacturerName = manufacturerName;
			item->modulePos = modulePos;
			menu->pushChild(item);
		}
	}
}


} // namespace rack
