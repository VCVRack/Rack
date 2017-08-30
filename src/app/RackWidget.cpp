#include <map>
#include <algorithm>
#include "app.hpp"
#include "engine.hpp"
#include "plugin.hpp"
#include "gui.hpp"


namespace rack {

static Vec rackGridSize = Vec(15, 380);


struct WireContainer : TransparentWidget {
	void draw(NVGcontext *vg) {
		// Wire plugs
		for (Widget *child : children) {
			WireWidget *wire = dynamic_cast<WireWidget*>(child);
			assert(wire);
			wire->drawPlugs(vg);
		}

		Widget::draw(vg);
	}
};

RackWidget::RackWidget() {
	moduleContainer = new Widget();
	addChild(moduleContainer);

	wireContainer = new WireContainer();
	addChild(wireContainer);
}

RackWidget::~RackWidget() {
}

void RackWidget::clear() {
	activeWire = NULL;
	wireContainer->clearChildren();
	moduleContainer->clearChildren();
}

void RackWidget::savePatch(std::string filename) {
	printf("Saving patch %s\n", filename.c_str());
	FILE *file = fopen(filename.c_str(), "w");
	if (!file)
		return;

	json_t *rootJ = toJson();
	if (rootJ) {
		json_dumpf(rootJ, file, JSON_INDENT(2));
		json_decref(rootJ);
	}

	fclose(file);
}

void RackWidget::loadPatch(std::string filename) {
	printf("Loading patch %s\n", filename.c_str());
	FILE *file = fopen(filename.c_str(), "r");
	if (!file)
		return;

	json_error_t error;
	json_t *rootJ = json_loadf(file, 0, &error);
	if (rootJ) {
		clear();
		fromJson(rootJ);
		json_decref(rootJ);
	}
	else {
		printf("JSON parsing error at %s %d:%d %s\n", error.source, error.line, error.column, error.text);
	}

	fclose(file);
}

json_t *RackWidget::toJson() {
	// root
	json_t *rootJ = json_object();

	// version
	json_t *versionJ = json_string(gApplicationVersion.c_str());
	json_object_set_new(rootJ, "version", versionJ);

	// wireOpacity
	json_t *wireOpacityJ = json_real(dynamic_cast<RackScene*>(gScene)->toolbar->wireOpacitySlider->value);
	json_object_set_new(rootJ, "wireOpacity", wireOpacityJ);

	// wireTension
	json_t *wireTensionJ = json_real(dynamic_cast<RackScene*>(gScene)->toolbar->wireTensionSlider->value);
	json_object_set_new(rootJ, "wireTension", wireTensionJ);

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
	// version
	json_t *versionJ = json_object_get(rootJ, "version");
	if (versionJ) {
		const char *version = json_string_value(versionJ);
		if (gApplicationVersion != version)
			printf("JSON version mismatch, attempting to convert JSON version %s to %s\n", version, gApplicationVersion.c_str());
	}

	// wireOpacity
	json_t *wireOpacityJ = json_object_get(rootJ, "wireOpacity");
	if (wireOpacityJ)
		dynamic_cast<RackScene*>(gScene)->toolbar->wireOpacitySlider->value = json_number_value(wireOpacityJ);

	// wireTension
	json_t *wireTensionJ = json_object_get(rootJ, "wireTension");
	if (wireTensionJ)
		dynamic_cast<RackScene*>(gScene)->toolbar->wireTensionSlider->value = json_number_value(wireTensionJ);

	// modules
	std::map<int, ModuleWidget*> moduleWidgets;
	json_t *modulesJ = json_object_get(rootJ, "modules");
	if (!modulesJ) return;
	size_t moduleId;
	json_t *moduleJ;
	json_array_foreach(modulesJ, moduleId, moduleJ) {
		// Get plugin
		json_t *pluginSlugJ = json_object_get(moduleJ, "plugin");
		if (!pluginSlugJ) continue;
		const char *pluginSlug = json_string_value(pluginSlugJ);
		Plugin *plugin = NULL;
		for (Plugin *p : gPlugins) {
			if (p->slug == pluginSlug) {
				plugin = p;
				break;
			}
		}
		if (!plugin) continue;

		// Get model
		json_t *modelSlug = json_object_get(moduleJ, "model");
		Model *model = NULL;
		for (Model *m : plugin->models) {
			if (m->slug == json_string_value(modelSlug)) {
				model = m;
				break;
			}
		}
		if (!model) continue;

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
		outputPort->connectedWire = wireWidget;
		inputPort->connectedWire = wireWidget;
		wireWidget->updateWire();
		// Add wire to rack
		wireContainer->addChild(wireWidget);
	}
}

void RackWidget::repositionModule(ModuleWidget *module) {
	// Create possible positions
	int x0 = roundf(module->requestedPos.x / rackGridSize.x);
	int y0 = roundf(module->requestedPos.y / rackGridSize.y);
	std::vector<Vec> positions;
	for (int y = maxi(0, y0 - 2); y < y0 + 2; y++) {
		for (int x = maxi(0, x0 - 40); x < x0 + 40; x++) {
			positions.push_back(Vec(x * rackGridSize.x, y * rackGridSize.y));
		}
	}

	// Sort possible positions by distance to the requested position
	Vec requestedPos = module->requestedPos;
	std::sort(positions.begin(), positions.end(), [requestedPos](Vec a, Vec b) {
		return a.minus(requestedPos).norm() < b.minus(requestedPos).norm();
	});

	// Find a position that does not collide
	for (Vec pos : positions) {
		Rect newBox = Rect(pos, module->box.size);
		bool collides = false;
		for (Widget *child2 : moduleContainer->children) {
			if (module == child2) continue;
			if (newBox.intersects(child2->box)) {
				collides = true;
				break;
			}
		}
		if (collides) continue;

		module->box.pos = pos;
		break;
	}
}

void RackWidget::step() {
	// Resize to be a bit larger than the ScrollWidget viewport
	assert(parent);
	assert(parent->parent);
	Vec moduleSize = moduleContainer->getChildrenBoundingBox().getBottomRight();
	Vec viewportSize = parent->parent->box.size.minus(parent->box.pos);
	box.size = moduleSize.max(viewportSize).plus(Vec(500, 500));

	// Reposition modules
	for (Widget *child : moduleContainer->children) {
		ModuleWidget *module = dynamic_cast<ModuleWidget*>(child);
		assert(module);
		if (module->requested) {
			repositionModule(module);
			module->requested = false;
		}
	}

	// Autosave every 15 seconds
	if (gGuiFrame % (60*15) == 0) {
		savePatch("autosave.json");
	}

	Widget::step();
}

void RackWidget::draw(NVGcontext *vg) {
	nvgBeginPath(vg);
	nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);

	// Background color
	nvgFillColor(vg, nvgRGBf(0.2, 0.2, 0.2));
	nvgFill(vg);

	// Rails
	// TODO Put this in a framebuffer cache and tile
	const float railHeight = 15;
	nvgFillColor(vg, nvgRGBf(0.8, 0.8, 0.8));
	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, nvgRGBf(0.6, 0.6, 0.6));
	float holeRadius = 3.5;
	for (float railY = 0; railY < box.size.y; railY += rackGridSize.y) {
		// Top rail
		nvgBeginPath(vg);
		nvgRect(vg, 0, railY, box.size.x, railHeight);
		for (float railX = 0; railX < box.size.x; railX += rackGridSize.x) {
			nvgCircle(vg, railX + rackGridSize.x / 2, railY + railHeight / 2, holeRadius);
			nvgPathWinding(vg, NVG_HOLE);
		}
		nvgFill(vg);

		nvgBeginPath(vg);
		nvgMoveTo(vg, 0, railY + railHeight - 0.5);
		nvgLineTo(vg, box.size.x, railY + railHeight - 0.5);
		nvgStroke(vg);

		// Bottom rail
		nvgBeginPath(vg);
		nvgRect(vg, 0, railY + rackGridSize.y - railHeight, box.size.x, railHeight);
		for (float railX = 0; railX < box.size.x; railX += rackGridSize.x) {
			nvgCircle(vg, railX + rackGridSize.x / 2, railY + rackGridSize.y - railHeight + railHeight / 2, holeRadius);
			nvgPathWinding(vg, NVG_HOLE);
		}
		nvgFill(vg);

		nvgBeginPath(vg);
		nvgMoveTo(vg, 0, railY + rackGridSize.y - 0.5);
		nvgLineTo(vg, box.size.x, railY + rackGridSize.y - 0.5);
		nvgStroke(vg);
	}

	Widget::draw(vg);
}

struct AddModuleMenuItem : MenuItem {
	Model *model;
	Vec modulePos;
	void onAction() {
		ModuleWidget *moduleWidget = model->createModuleWidget();
		moduleWidget->requestedPos = modulePos.minus(moduleWidget->box.getCenter());
		moduleWidget->requested = true;
		gRackWidget->moduleContainer->addChild(moduleWidget);
	}
};

void RackWidget::onMouseDown(int button) {
	if (button == 1) {
		Vec modulePos = gMousePos.minus(getAbsolutePos());
		Menu *menu = gScene->createMenu();

		MenuLabel *menuLabel = new MenuLabel();
		menuLabel->text = "Add Module";
		menu->pushChild(menuLabel);
		for (Plugin *plugin : gPlugins) {
			for (Model *model : plugin->models) {
				AddModuleMenuItem *item = new AddModuleMenuItem();
				item->text = model->plugin->name + ": " + model->name;
				item->model = model;
				item->modulePos = modulePos;
				menu->pushChild(item);
			}
		}
	}
}


} // namespace rack
