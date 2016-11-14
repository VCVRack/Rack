#include "../5V.hpp"
#include <algorithm>


RackWidget::RackWidget() {
	moduleContainer = new TranslucentWidget();
	addChild(moduleContainer);

	wireContainer = new TransparentWidget();
	addChild(wireContainer);
}

void RackWidget::clear() {
	activeWire = NULL;
	wireContainer->clearChildren();
	moduleContainer->clearChildren();
}

json_t *RackWidget::toJson() {
	// root
	json_t *root = json_object();

	// rack
	json_t *rack = json_object();
	{
		json_t *size = json_pack("[f, f]", (double) box.size.x, (double) box.size.y);
		json_object_set_new(rack, "size", size);
	}
	json_object_set_new(root, "rack", rack);

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
	json_object_set_new(root, "modules", modulesJ);

	// wires
	json_t *wires = json_array();
	for (Widget *w : wireContainer->children) {
		WireWidget *wireWidget = dynamic_cast<WireWidget*>(w);
		assert(wireWidget);
		// wire
		json_t *wire = json_object();
		{
			int outputModuleId = moduleIds[wireWidget->outputPort->moduleWidget];
			int inputModuleId = moduleIds[wireWidget->inputPort->moduleWidget];
			json_object_set_new(wire, "outputModuleId", json_integer(outputModuleId));
			json_object_set_new(wire, "outputId", json_integer(wireWidget->outputPort->outputId));
			json_object_set_new(wire, "inputModuleId", json_integer(inputModuleId));
			json_object_set_new(wire, "inputId", json_integer(wireWidget->inputPort->inputId));
		}
		json_array_append_new(wires, wire);
	}
	json_object_set_new(root, "wires", wires);

	return root;
}

void RackWidget::fromJson(json_t *root) {
	// TODO There's virtually no validation in here. Bad input will result in a crash.
	// rack
	json_t *rack = json_object_get(root, "rack");
	assert(rack);
	{
		// size
		json_t *size = json_object_get(rack, "size");
		double width, height;
		json_unpack(size, "[F, F]", &width, &height);
		box.size = Vec(width, height);
	}

	// modules
	std::map<int, ModuleWidget*> moduleWidgets;
	json_t *modulesJ = json_object_get(root, "modules");
	size_t moduleId;
	json_t *moduleJ;
	json_array_foreach(modulesJ, moduleId, moduleJ) {
		// Get plugin
		json_t *pluginSlugJ = json_object_get(moduleJ, "plugin");
		const char *pluginSlug = json_string_value(pluginSlugJ);
		Plugin *plugin = NULL;
		for (Plugin *p : gPlugins) {
			if (p->slug == pluginSlug) {
				plugin = p;
				break;
			}
		}
		assert(plugin);

		// Get model
		json_t *modelSlug = json_object_get(moduleJ, "model");
		Model *model = NULL;
		for (Model *m : plugin->models) {
			if (m->slug == json_string_value(modelSlug)) {
				model = m;
				break;
			}
		}
		assert(model);

		// Create ModuleWidget
		ModuleWidget *moduleWidget = model->createModuleWidget();
		moduleWidget->fromJson(moduleJ);
		moduleContainer->addChild(moduleWidget);
		moduleWidgets[moduleId] = moduleWidget;
	}

	// wires
	json_t *wiresJ = json_object_get(root, "wires");
	size_t wireId;
	json_t *wireJ;
	json_array_foreach(wiresJ, wireId, wireJ) {
		int outputModuleId, outputId;
		int inputModuleId, inputId;
		json_unpack(wireJ, "{s:i, s:i, s:i, s:i}",
			"outputModuleId", &outputModuleId, "outputId", &outputId,
			"inputModuleId", &inputModuleId, "inputId", &inputId);
		// Get ports
		ModuleWidget *outputModuleWidget = moduleWidgets[outputModuleId];
		assert(outputModuleWidget);
		OutputPort *outputPort = outputModuleWidget->outputs[outputId];
		assert(outputPort);
		ModuleWidget *inputModuleWidget = moduleWidgets[inputModuleId];
		assert(inputModuleWidget);
		InputPort *inputPort = inputModuleWidget->inputs[inputId];
		assert(inputPort);
		// Create WireWidget
		WireWidget *wireWidget = new WireWidget();
		wireWidget->outputPort = outputPort;
		wireWidget->inputPort = inputPort;
		outputPort->connectedWire = wireWidget;
		inputPort->connectedWire = wireWidget;
		wireWidget->updateWire();
		// Add wire to rack
		gRackWidget->wireContainer->addChild(wireWidget);
	}
}

void RackWidget::repositionModule(ModuleWidget *module) {
	// Create possible positions
	int x0 = roundf(module->requestedPos.x / 15);
	int y0 = roundf(module->requestedPos.y / 380);
	std::vector<Vec> positions;
	for (int y = maxi(0, y0 - 2); y < y0 + 2; y++) {
		for (int x = maxi(0, x0 - 40); x < x0 + 40; x++) {
			positions.push_back(Vec(x*15, y*380));
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

	Widget::step();
}

void RackWidget::draw(NVGcontext *vg) {
	// Draw background
	nvgBeginPath(vg);
	nvgRect(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);
	NVGpaint paint;
	{
		int imageId = loadImage("res/wood.jpg");
		int imageWidth, imageHeight;
		nvgImageSize(vg, imageId, &imageWidth, &imageHeight);
		paint = nvgImagePattern(vg, box.pos.x, box.pos.y, imageWidth, imageHeight, 0.0, imageId, 1.0);
		nvgFillPaint(vg, paint);
		nvgFill(vg);
	}
	{
		int imageId = loadImage("res/rackrails.png");
		int imageWidth, imageHeight;
		nvgImageSize(vg, imageId, &imageWidth, &imageHeight);
		paint = nvgImagePattern(vg, box.pos.x, box.pos.y, imageWidth, imageHeight, 0.0, imageId, 1.0);
		nvgFillPaint(vg, paint);
		nvgFill(vg);
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
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		// Get relative position of the click
		Vec modulePos = gMousePos.minus(getAbsolutePos());

		MenuOverlay *overlay = new MenuOverlay();
		Menu *menu = new Menu();
		menu->box.pos = gMousePos;

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
		overlay->addChild(menu);
		gScene->addChild(overlay);
	}
}
