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

json_t *ModuleWidget::toJson() {
	json_t *rootJ = json_object();

	// plugin
	json_object_set_new(rootJ, "plugin", json_string(model->plugin->slug.c_str()));
	// model
	json_object_set_new(rootJ, "model", json_string(model->slug.c_str()));
	// pos
	json_t *posJ = json_pack("[f, f]", (double) box.pos.x, (double) box.pos.y);
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
	// pos
	json_t *posJ = json_object_get(rootJ, "pos");
	double x, y;
	json_unpack(posJ, "[F, F]", &x, &y);
	box.pos = Vec(x, y);

	// params
	json_t *paramsJ = json_object_get(rootJ, "params");
	size_t paramId;
	json_t *paramJ;
	json_array_foreach(paramsJ, paramId, paramJ) {
		if (paramId < params.size()) {
			params[paramId]->fromJson(paramJ);
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

void ModuleWidget::initialize() {
	for (ParamWidget *param : params) {
		param->setValue(param->defaultValue);
	}
	if (module) {
		module->initialize();
	}
}

void ModuleWidget::randomize() {
	for (ParamWidget *param : params) {
		param->randomize();
	}
	if (module) {
		module->randomize();
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

Widget *ModuleWidget::onMouseMove(Vec pos, Vec mouseRel) {
	if (!gFocusedWidget) {
		// Instead of checking key-down events, delete the module even if key-repeat hasn't fired yet and the cursor is hovering over the widget.
		if (glfwGetKey(gWindow, GLFW_KEY_DELETE) == GLFW_PRESS || glfwGetKey(gWindow, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
			if (!guiIsModPressed() && !guiIsShiftPressed()) {
				gRackWidget->deleteModule(this);
				this->finalizeEvents();
				delete this;
				// Kinda sketchy because events will be passed further down the tree
				return NULL;
			}
		}
	}
	return OpaqueWidget::onMouseMove(pos, mouseRel);
}

Widget *ModuleWidget::onHoverKey(Vec pos, int key) {
	switch (key) {
		case GLFW_KEY_I:
			if (guiIsModPressed() && !guiIsShiftPressed()) {
				initialize();
				return this;
			}
			break;
		case GLFW_KEY_R:
			if (guiIsModPressed() && !guiIsShiftPressed()) {
				randomize();
				return this;
			}
			break;
		case GLFW_KEY_D:
			if (guiIsModPressed() && !guiIsShiftPressed()) {
				gRackWidget->cloneModule(this);
				return this;
			}
			break;
	}

	return Widget::onHoverKey(pos, key);
}

void ModuleWidget::onDragStart() {
	dragPos = gMousePos.minus(getAbsolutePos());
}

void ModuleWidget::onDragMove(Vec mouseRel) {
	Rect newBox = box;
	newBox.pos = gMousePos.minus(parent->getAbsolutePos()).minus(dragPos);
	gRackWidget->requestModuleBoxNearest(this, newBox);
}

void ModuleWidget::onDragEnd() {
}

struct DisconnectMenuItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction() {
		moduleWidget->disconnect();
	}
};

struct InitializeMenuItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction() {
		moduleWidget->initialize();
	}
};

struct RandomizeMenuItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction() {
		moduleWidget->randomize();
	}
};

struct CloneMenuItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction() {
		gRackWidget->cloneModule(moduleWidget);
	}
};

struct DeleteMenuItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction() {
		gRackWidget->deleteModule(moduleWidget);
		moduleWidget->finalizeEvents();
		delete moduleWidget;
	}
};

Menu *ModuleWidget::createContextMenu() {
	Menu *menu = gScene->createMenu();

	MenuLabel *menuLabel = new MenuLabel();
	menuLabel->text = model->plugin->name + ": " + model->name;
	menu->pushChild(menuLabel);

	InitializeMenuItem *resetItem = new InitializeMenuItem();
	resetItem->text = "Initialize";
	resetItem->rightText = GUI_MOD_KEY_NAME "+I";
	resetItem->moduleWidget = this;
	menu->pushChild(resetItem);

	RandomizeMenuItem *randomizeItem = new RandomizeMenuItem();
	randomizeItem->text = "Randomize";
	randomizeItem->rightText = GUI_MOD_KEY_NAME "+R";
	randomizeItem->moduleWidget = this;
	menu->pushChild(randomizeItem);

	DisconnectMenuItem *disconnectItem = new DisconnectMenuItem();
	disconnectItem->text = "Disconnect cables";
	disconnectItem->moduleWidget = this;
	menu->pushChild(disconnectItem);

	CloneMenuItem *cloneItem = new CloneMenuItem();
	cloneItem->text = "Duplicate";
	cloneItem->rightText = GUI_MOD_KEY_NAME "+D";
	cloneItem->moduleWidget = this;
	menu->pushChild(cloneItem);

	DeleteMenuItem *deleteItem = new DeleteMenuItem();
	deleteItem->text = "Delete";
	deleteItem->rightText = "Backspace/Delete";
	deleteItem->moduleWidget = this;
	menu->pushChild(deleteItem);

	return menu;
}

void ModuleWidget::onMouseDownOpaque(int button) {
	if (button == 1) {
		createContextMenu();
	}
}


} // namespace rack