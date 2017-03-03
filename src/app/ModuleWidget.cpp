#include "app.hpp"
#include "engine.hpp"
#include "plugin.hpp"


namespace rack {

ModuleWidget::~ModuleWidget() {
	// Make sure WireWidget destructors are called *before* removing `module` from the rack.
	disconnectPorts();
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
	json_t *root = json_object();

	// plugin
	json_object_set_new(root, "plugin", json_string(model->plugin->slug.c_str()));
	// model
	json_object_set_new(root, "model", json_string(model->slug.c_str()));
	// pos
	json_t *pos = json_pack("[f, f]", (double) box.pos.x, (double) box.pos.y);
	json_object_set_new(root, "pos", pos);
	// params
	json_t *paramsJ = json_array();
	for (ParamWidget *paramWidget : params) {
		json_t *paramJ = paramWidget->toJson();
		json_array_append_new(paramsJ, paramJ);
	}
	json_object_set_new(root, "params", paramsJ);

	return root;
}

void ModuleWidget::fromJson(json_t *root) {
	// pos
	json_t *pos = json_object_get(root, "pos");
	double x, y;
	json_unpack(pos, "[F, F]", &x, &y);
	box.pos = Vec(x, y);

	// params
	json_t *paramsJ = json_object_get(root, "params");
	size_t paramId;
	json_t *paramJ;
	json_array_foreach(paramsJ, paramId, paramJ) {
		if (0 <= paramId && paramId < params.size()) {
			params[paramId]->fromJson(paramJ);
		}
	}
}

void ModuleWidget::disconnectPorts() {
	for (Port *input : inputs) {
		input->disconnect();
	}
	for (Port *output : outputs) {
		output->disconnect();
	}
}

void ModuleWidget::resetParams() {
	for (ParamWidget *param : params) {
		param->setValue(param->defaultValue);
	}
}

void ModuleWidget::cloneParams(ModuleWidget *source) {
	assert(params.size() == source->params.size());
	for (size_t i = 0; i < params.size(); i++) {
		params[i]->setValue(source->params[i]->value);
	}
}

void ModuleWidget::draw(NVGcontext *vg) {
	Widget::draw(vg);
	bndBevel(vg, 0.0, 0.0, box.size.x, box.size.y);

	// CPU usage text
	if (dynamic_cast<RackScene*>(gScene)->toolbar->cpuUsageButton->value != 0.0) {
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
}

void ModuleWidget::onDragStart() {
	dragPos = gMousePos.minus(getAbsolutePos());
}

void ModuleWidget::onDragMove(Vec mouseRel) {
	requestedPos = gMousePos.minus(parent->getAbsolutePos()).minus(dragPos);
	requested = true;
}

void ModuleWidget::onDragEnd() {
}

struct DisconnectPortsMenuItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction() {
		moduleWidget->disconnectPorts();
	}
};

struct ResetParamsMenuItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction() {
		moduleWidget->resetParams();
	}
};

struct CloneModuleMenuItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction() {
		// Create new module from model
		ModuleWidget *clonedModuleWidget = moduleWidget->model->createModuleWidget();
		clonedModuleWidget->requestedPos = moduleWidget->box.pos;
		clonedModuleWidget->requested = true;
		clonedModuleWidget->cloneParams(moduleWidget);
		gRackWidget->moduleContainer->addChild(clonedModuleWidget);
	}
};

struct DeleteModuleMenuItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction() {
		gRackWidget->moduleContainer->removeChild(moduleWidget);
		delete moduleWidget;
	}
};

void ModuleWidget::onMouseDown(int button) {
	if (button == 1) {
		MenuOverlay *overlay = new MenuOverlay();
		Menu *menu = new Menu();
		menu->box.pos = gMousePos;
		{
			MenuLabel *menuLabel = new MenuLabel();
			menuLabel->text = model->plugin->name + ": " + model->name;
			menu->pushChild(menuLabel);

			ResetParamsMenuItem *resetItem = new ResetParamsMenuItem();
			resetItem->text = "Initialize";
			resetItem->moduleWidget = this;
			menu->pushChild(resetItem);

			DisconnectPortsMenuItem *disconnectItem = new DisconnectPortsMenuItem();
			disconnectItem->text = "Disconnect cables";
			disconnectItem->moduleWidget = this;
			menu->pushChild(disconnectItem);

			CloneModuleMenuItem *cloneItem = new CloneModuleMenuItem();
			cloneItem->text = "Clone";
			cloneItem->moduleWidget = this;
			menu->pushChild(cloneItem);

			DeleteModuleMenuItem *deleteItem = new DeleteModuleMenuItem();
			deleteItem->text = "Delete";
			deleteItem->moduleWidget = this;
			menu->pushChild(deleteItem);
		}
		overlay->addChild(menu);
		gScene->setOverlay(overlay);
	}
}


} // namespace rack