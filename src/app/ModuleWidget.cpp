#include "app.hpp"
#include "engine.hpp"
#include "plugin.hpp"
#include "gui.hpp"


namespace rack {


ModuleWidget::~ModuleWidget() {
	// Make sure WireWidget destructors are called *before* removing `module` from the rack.
	disconnect();
	// Remove and delete the Module instance
	setModule(NULL);
}

void ModuleWidget::setModule(Module *module) {
	if (this->module) {
		engineRemoveModule(this->module);
		delete this->module;
	}
	if (module) {
		engineAddModule(module);
	}
	this->module = module;
}

void ModuleWidget::addInput(Port *input) {
	assert(input->type == Port::INPUT);
	inputs.push_back(input);
	addChild(input);
}

void ModuleWidget::addOutput(Port *output) {
	assert(output->type == Port::OUTPUT);
	outputs.push_back(output);
	addChild(output);
}

void ModuleWidget::addParam(ParamWidget *param) {
	params.push_back(param);
	addChild(param);
}

void ModuleWidget::setPanel(std::shared_ptr<SVG> svg) {
	// Remove old panel
	if (panel) {
		removeChild(panel);
		panel = NULL;
	}

	panel = new SVGPanel();
	panel->setBackground(svg);
	addChild(panel);

	box.size = panel->box.size;
}


json_t *ModuleWidget::toJson() {
	json_t *rootJ = json_object();

	// plugin
	json_object_set_new(rootJ, "plugin", json_string(model->plugin->slug.c_str()));
	// version (of plugin)
	if (!model->plugin->version.empty())
		json_object_set_new(rootJ, "version", json_string(model->plugin->version.c_str()));
	// model
	json_object_set_new(rootJ, "model", json_string(model->slug.c_str()));
	// pos
	Vec pos = box.pos.div(RACK_GRID_SIZE).round();
	json_t *posJ = json_pack("[i, i]", (int) pos.x, (int) pos.y);
	json_object_set_new(rootJ, "pos", posJ);
	// params
	json_t *paramsJ = json_array();
	for (ParamWidget *paramWidget : params) {
		json_t *paramJ = paramWidget->toJson();
		json_array_append_new(paramsJ, paramJ);
	}
	json_object_set_new(rootJ, "params", paramsJ);
	// data
	if (module) {
		json_t *dataJ = module->toJson();
		if (dataJ) {
			json_object_set_new(rootJ, "data", dataJ);
		}
	}

	return rootJ;
}

void ModuleWidget::fromJson(json_t *rootJ) {
	// legacy
	int legacy = 0;
	json_t *legacyJ = json_object_get(rootJ, "legacy");
	if (legacyJ)
		legacy = json_integer_value(legacyJ);

	// pos
	json_t *posJ = json_object_get(rootJ, "pos");
	double x, y;
	json_unpack(posJ, "[F, F]", &x, &y);
	Vec pos = Vec(x, y);
	if (legacy && legacy <= 1) {
		box.pos = pos;
	}
	else {
		box.pos = pos.mult(RACK_GRID_SIZE);
	}

	// params
	json_t *paramsJ = json_object_get(rootJ, "params");
	size_t i;
	json_t *paramJ;
	json_array_foreach(paramsJ, i, paramJ) {
		if (legacy && legacy <= 1) {
			// The index in the array we're iterating is the index of the ParamWidget in the params vector.
			if (i < params.size()) {
				// Create upgraded version of param JSON object
				json_t *newParamJ = json_object();
				json_object_set(newParamJ, "value", paramJ);
				params[i]->fromJson(newParamJ);
				json_decref(newParamJ);
			}
		}
		else {
			// Get paramId
			json_t *paramIdJ = json_object_get(paramJ, "paramId");
			if (!paramIdJ)
				continue;
			int paramId = json_integer_value(paramIdJ);
			// Find ParamWidget(s) with paramId
			for (ParamWidget *paramWidget : params) {
				if (paramWidget->paramId == paramId)
					paramWidget->fromJson(paramJ);
			}
		}
	}

	// data
	json_t *dataJ = json_object_get(rootJ, "data");
	if (dataJ && module) {
		module->fromJson(dataJ);
	}
}

void ModuleWidget::disconnect() {
	for (Port *input : inputs) {
		gRackWidget->wireContainer->removeAllWires(input);
	}
	for (Port *output : outputs) {
		gRackWidget->wireContainer->removeAllWires(output);
	}
}

void ModuleWidget::create() {
	if (module) {
		module->onCreate();
	}
}

void ModuleWidget::_delete() {
	if (module) {
		module->onDelete();
	}
}

void ModuleWidget::reset() {
	for (ParamWidget *param : params) {
		param->setValue(param->defaultValue);
	}
	if (module) {
		module->onReset();
	}
}

void ModuleWidget::randomize() {
	for (ParamWidget *param : params) {
		param->randomize();
	}
	if (module) {
		module->onRandomize();
	}
}

void ModuleWidget::draw(NVGcontext *vg) {
	nvgScissor(vg, 0, 0, box.size.x, box.size.y);

	Widget::draw(vg);

	// CPU usage text
	if (0) {
		float cpuTime = module ? module->cpuTime : 0.0;
		std::string text = stringf("%.1f%%", cpuTime * 100.0);

		nvgSave(vg);
		nvgBeginPath(vg);
		nvgRect(vg, 0.0, 0.0, box.size.x, BND_WIDGET_HEIGHT);
		nvgFillColor(vg, nvgRGBf(0.0, 0.0, 0.0));
		nvgFill(vg);

		nvgBeginPath(vg);
		cpuTime = clampf(cpuTime, 0.0, 1.0);
		nvgRect(vg, 0.0, 0.0, box.size.x * cpuTime, BND_WIDGET_HEIGHT);
		nvgFillColor(vg, nvgHSL(0.33 * cubic(1.0 - cpuTime), 1.0, 0.4));
		nvgFill(vg);

		bndMenuItem(vg, 0.0, 0.0, box.size.x, BND_WIDGET_HEIGHT, BND_DEFAULT, -1, text.c_str());
		nvgRestore(vg);
	}

	nvgResetScissor(vg);
}

void ModuleWidget::onMouseDown(EventMouseDown &e) {
	Widget::onMouseDown(e);
	if (e.consumed)
		return;

	if (e.button == 1) {
		createContextMenu();
	}
	e.consumed = true;
	e.target = this;
}

void ModuleWidget::onMouseMove(EventMouseMove &e) {
	OpaqueWidget::onMouseMove(e);

	// Don't delete the ModuleWidget if a TextField is focused
	if (!gFocusedWidget) {
		// Instead of checking key-down events, delete the module even if key-repeat hasn't fired yet and the cursor is hovering over the widget.
		if (glfwGetKey(gWindow, GLFW_KEY_DELETE) == GLFW_PRESS || glfwGetKey(gWindow, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
			if (!guiIsModPressed() && !guiIsShiftPressed()) {
				gRackWidget->deleteModule(this);
				this->finalizeEvents();
				delete this;
				e.consumed = true;
				return;
			}
		}
	}
}

void ModuleWidget::onHoverKey(EventHoverKey &e) {
	switch (e.key) {
		case GLFW_KEY_I:
			if (guiIsModPressed() && !guiIsShiftPressed()) {
				reset();
				e.consumed = true;
				return;
			}
			break;
		case GLFW_KEY_R:
			if (guiIsModPressed() && !guiIsShiftPressed()) {
				randomize();
				e.consumed = true;
				return;
			}
			break;
		case GLFW_KEY_D:
			if (guiIsModPressed() && !guiIsShiftPressed()) {
				gRackWidget->cloneModule(this);
				e.consumed = true;
				return;
			}
			break;
	}

	Widget::onHoverKey(e);
}

void ModuleWidget::onDragStart(EventDragStart &e) {
	dragPos = gRackWidget->lastMousePos.minus(box.pos);
}

void ModuleWidget::onDragEnd(EventDragEnd &e) {
}

void ModuleWidget::onDragMove(EventDragMove &e) {
	Rect newBox = box;
	newBox.pos = gRackWidget->lastMousePos.minus(dragPos);
	gRackWidget->requestModuleBoxNearest(this, newBox);
}


struct DisconnectMenuItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(EventAction &e) override {
		moduleWidget->disconnect();
	}
};

struct ResetMenuItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(EventAction &e) override {
		moduleWidget->reset();
	}
};

struct RandomizeMenuItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(EventAction &e) override {
		moduleWidget->randomize();
	}
};

struct CloneMenuItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(EventAction &e) override {
		gRackWidget->cloneModule(moduleWidget);
	}
};

struct DeleteMenuItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(EventAction &e) override {
		gRackWidget->deleteModule(moduleWidget);
		moduleWidget->finalizeEvents();
		delete moduleWidget;
	}
};

Menu *ModuleWidget::createContextMenu() {
	Menu *menu = gScene->createMenu();

	MenuLabel *menuLabel = new MenuLabel();
	menuLabel->text = model->manufacturer + " " + model->name;
	menu->addChild(menuLabel);

	ResetMenuItem *resetItem = new ResetMenuItem();
	resetItem->text = "Initialize";
	resetItem->rightText = GUI_MOD_KEY_NAME "+I";
	resetItem->moduleWidget = this;
	menu->addChild(resetItem);

	RandomizeMenuItem *randomizeItem = new RandomizeMenuItem();
	randomizeItem->text = "Randomize";
	randomizeItem->rightText = GUI_MOD_KEY_NAME "+R";
	randomizeItem->moduleWidget = this;
	menu->addChild(randomizeItem);

	DisconnectMenuItem *disconnectItem = new DisconnectMenuItem();
	disconnectItem->text = "Disconnect cables";
	disconnectItem->moduleWidget = this;
	menu->addChild(disconnectItem);

	CloneMenuItem *cloneItem = new CloneMenuItem();
	cloneItem->text = "Duplicate";
	cloneItem->rightText = GUI_MOD_KEY_NAME "+D";
	cloneItem->moduleWidget = this;
	menu->addChild(cloneItem);

	DeleteMenuItem *deleteItem = new DeleteMenuItem();
	deleteItem->text = "Delete";
	deleteItem->rightText = "Backspace/Delete";
	deleteItem->moduleWidget = this;
	menu->addChild(deleteItem);

	return menu;
}


} // namespace rack