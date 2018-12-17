#include "app/ModuleWidget.hpp"
#include "engine/Engine.hpp"
#include "logger.hpp"
#include "system.hpp"
#include "AssetManager.hpp"
#include "app/Scene.hpp"
#include "helpers.hpp"
#include "context.hpp"

#include "osdialog.h"


namespace rack {


ModuleWidget::ModuleWidget(Module *module) {
	if (module) {
		context()->engine->addModule(module);
	}
	this->module = module;
}

ModuleWidget::~ModuleWidget() {
	// Make sure WireWidget destructors are called *before* removing `module` from the rack.
	disconnect();
	// Remove and delete the Module instance
	if (module) {
		context()->engine->removeModule(module);
		delete module;
		module = NULL;
	}
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
		delete panel;
		panel = NULL;
	}

	panel = new SVGPanel;
	panel->setBackground(svg);
	addChild(panel);

	box.size = panel->box.size;
}


json_t *ModuleWidget::toJson() {
	json_t *rootJ = json_object();

	// plugin
	json_object_set_new(rootJ, "plugin", json_string(model->plugin->slug.c_str()));
	// version of plugin
	if (!model->plugin->version.empty())
		json_object_set_new(rootJ, "version", json_string(model->plugin->version.c_str()));
	// model
	json_object_set_new(rootJ, "model", json_string(model->slug.c_str()));

	// Other properties
	if (module) {
		json_t *moduleJ = module->toJson();
		// Merge with rootJ
		json_object_update(rootJ, moduleJ);
		json_decref(moduleJ);
	}
	return rootJ;
}

void ModuleWidget::fromJson(json_t *rootJ) {
	// Check if plugin and model are incorrect
	json_t *pluginJ = json_object_get(rootJ, "plugin");
	std::string pluginSlug;
	if (pluginJ) {
		pluginSlug = json_string_value(pluginJ);
		if (pluginSlug != model->plugin->slug) {
			WARN("Plugin %s does not match ModuleWidget's plugin %s.", pluginSlug.c_str(), model->plugin->slug.c_str());
			return;
		}
	}

	json_t *modelJ = json_object_get(rootJ, "model");
	std::string modelSlug;
	if (modelJ) {
		modelSlug = json_string_value(modelJ);
		if (modelSlug != model->slug) {
			WARN("Model %s does not match ModuleWidget's model %s.", modelSlug.c_str(), model->slug.c_str());
			return;
		}
	}

	// Check plugin version
	json_t *versionJ = json_object_get(rootJ, "version");
	if (versionJ) {
		std::string version = json_string_value(versionJ);
		if (version != model->plugin->version) {
			INFO("Patch created with %s version %s, using version %s.", pluginSlug.c_str(), version.c_str(), model->plugin->version.c_str());
		}
	}

	if (module) {
		module->fromJson(rootJ);
	}
}

void ModuleWidget::copyClipboard() {
	json_t *moduleJ = toJson();
	char *moduleJson = json_dumps(moduleJ, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
	glfwSetClipboardString(gWindow, moduleJson);
	free(moduleJson);
	json_decref(moduleJ);
}

void ModuleWidget::pasteClipboard() {
	const char *moduleJson = glfwGetClipboardString(gWindow);
	if (!moduleJson) {
		WARN("Could not get text from clipboard.");
		return;
	}

	json_error_t error;
	json_t *moduleJ = json_loads(moduleJson, 0, &error);
	if (moduleJ) {
		fromJson(moduleJ);
		json_decref(moduleJ);
	}
	else {
		WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
	}
}

void ModuleWidget::load(std::string filename) {
	INFO("Loading preset %s", filename.c_str());
	FILE *file = fopen(filename.c_str(), "r");
	if (!file) {
		// Exit silently
		return;
	}

	json_error_t error;
	json_t *moduleJ = json_loadf(file, 0, &error);
	if (moduleJ) {
		fromJson(moduleJ);
		json_decref(moduleJ);
	}
	else {
		std::string message = string::f("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
	}

	fclose(file);
}

void ModuleWidget::save(std::string filename) {
	INFO("Saving preset %s", filename.c_str());
	json_t *moduleJ = toJson();
	if (!moduleJ)
		return;

	FILE *file = fopen(filename.c_str(), "w");
	if (file) {
		json_dumpf(moduleJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
		fclose(file);
	}

	json_decref(moduleJ);
}

void ModuleWidget::loadDialog() {
	std::string dir = context()->asset->user("presets");
	system::createDirectory(dir);

	osdialog_filters *filters = osdialog_filters_parse(PRESET_FILTERS.c_str());
	char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, filters);
	if (path) {
		load(path);
		free(path);
	}
	osdialog_filters_free(filters);
}

void ModuleWidget::saveDialog() {
	std::string dir = context()->asset->user("presets");
	system::createDirectory(dir);

	osdialog_filters *filters = osdialog_filters_parse(PRESET_FILTERS.c_str());
	char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), "Untitled.vcvm", filters);

	if (path) {
		std::string pathStr = path;
		free(path);
		std::string extension = string::extension(pathStr);
		if (extension.empty()) {
			pathStr += ".vcvm";
		}

		save(pathStr);
	}
	osdialog_filters_free(filters);
}

void ModuleWidget::disconnect() {
	for (Port *input : inputs) {
		context()->scene->rackWidget->wireContainer->removeAllWires(input);
	}
	for (Port *output : outputs) {
		context()->scene->rackWidget->wireContainer->removeAllWires(output);
	}
}

void ModuleWidget::create() {
}

void ModuleWidget::_delete() {
}

void ModuleWidget::reset() {
	for (ParamWidget *param : params) {
		param->reset();
	}
	if (module) {
		context()->engine->resetModule(module);
	}
}

void ModuleWidget::randomize() {
	for (ParamWidget *param : params) {
		param->randomize();
	}
	if (module) {
		context()->engine->randomizeModule(module);
	}
}

void ModuleWidget::draw(NVGcontext *vg) {
	nvgScissor(vg, 0, 0, box.size.x, box.size.y);
	Widget::draw(vg);

	// Power meter
	if (module && context()->engine->powerMeter) {
		nvgBeginPath(vg);
		nvgRect(vg,
			0, box.size.y - 20,
			55, 20);
		nvgFillColor(vg, nvgRGBAf(0, 0, 0, 0.5));
		nvgFill(vg);

		std::string cpuText = string::f("%.0f mS", module->cpuTime * 1000.f);
		nvgFontFaceId(vg, gGuiFont->handle);
		nvgFontSize(vg, 12);
		nvgFillColor(vg, nvgRGBf(1, 1, 1));
		nvgText(vg, 10.0, box.size.y - 6.0, cpuText.c_str(), NULL);

		float p = clamp(module->cpuTime, 0.f, 1.f);
		nvgBeginPath(vg);
		nvgRect(vg,
			0, (1.f - p) * box.size.y,
			5, p * box.size.y);
		nvgFillColor(vg, nvgRGBAf(1, 0, 0, 1.0));
		nvgFill(vg);
	}

	nvgResetScissor(vg);
}

void ModuleWidget::drawShadow(NVGcontext *vg) {
	nvgBeginPath(vg);
	float r = 20; // Blur radius
	float c = 20; // Corner radius
	Vec b = Vec(-10, 30); // Offset from each corner
	nvgRect(vg, b.x - r, b.y - r, box.size.x - 2*b.x + 2*r, box.size.y - 2*b.y + 2*r);
	NVGcolor shadowColor = nvgRGBAf(0, 0, 0, 0.2);
	NVGcolor transparentColor = nvgRGBAf(0, 0, 0, 0);
	nvgFillPaint(vg, nvgBoxGradient(vg, b.x, b.y, box.size.x - 2*b.x, box.size.y - 2*b.y, c, r, shadowColor, transparentColor));
	nvgFill(vg);
}

void ModuleWidget::onHover(event::Hover &e) {
	OpaqueWidget::onHover(e);

	// Instead of checking key-down events, delete the module even if key-repeat hasn't fired yet and the cursor is hovering over the widget.
	if (glfwGetKey(gWindow, GLFW_KEY_DELETE) == GLFW_PRESS || glfwGetKey(gWindow, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
		if (!windowIsModPressed() && !windowIsShiftPressed()) {
			context()->scene->rackWidget->deleteModule(this);
			delete this;
			// e.target = this;
			return;
		}
	}
}

void ModuleWidget::onButton(event::Button &e) {
	OpaqueWidget::onButton(e);
	if (e.target == this) {
		if (e.button == 1) {
			createContextMenu();
		}
	}
}

void ModuleWidget::onHoverKey(event::HoverKey &e) {
	switch (e.key) {
		case GLFW_KEY_I: {
			if (windowIsModPressed() && !windowIsShiftPressed()) {
				reset();
				e.target = this;
				return;
			}
		} break;
		case GLFW_KEY_R: {
			if (windowIsModPressed() && !windowIsShiftPressed()) {
				randomize();
				e.target = this;
				return;
			}
		} break;
		case GLFW_KEY_C: {
			if (windowIsModPressed() && !windowIsShiftPressed()) {
				copyClipboard();
				e.target = this;
				return;
			}
		} break;
		case GLFW_KEY_V: {
			if (windowIsModPressed() && !windowIsShiftPressed()) {
				pasteClipboard();
				e.target = this;
				return;
			}
		} break;
		case GLFW_KEY_D: {
			if (windowIsModPressed() && !windowIsShiftPressed()) {
				context()->scene->rackWidget->cloneModule(this);
				e.target = this;
				return;
			}
		} break;
		case GLFW_KEY_U: {
			if (windowIsModPressed() && !windowIsShiftPressed()) {
				disconnect();
				e.target = this;
				return;
			}
		} break;
	}

	OpaqueWidget::onHoverKey(e);
}

void ModuleWidget::onDragStart(event::DragStart &e) {
	dragPos = context()->scene->rackWidget->lastMousePos.minus(box.pos);
}

void ModuleWidget::onDragEnd(event::DragEnd &e) {
}

void ModuleWidget::onDragMove(event::DragMove &e) {
	if (!context()->scene->rackWidget->lockModules) {
		Rect newBox = box;
		newBox.pos = context()->scene->rackWidget->lastMousePos.minus(dragPos);
		context()->scene->rackWidget->requestModuleBoxNearest(this, newBox);
	}
}


struct ModuleDisconnectItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(event::Action &e) override {
		moduleWidget->disconnect();
	}
};

struct ModuleResetItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(event::Action &e) override {
		moduleWidget->reset();
	}
};

struct ModuleRandomizeItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(event::Action &e) override {
		moduleWidget->randomize();
	}
};

struct ModuleCopyItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(event::Action &e) override {
		moduleWidget->copyClipboard();
	}
};

struct ModulePasteItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(event::Action &e) override {
		moduleWidget->pasteClipboard();
	}
};

struct ModuleSaveItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(event::Action &e) override {
		moduleWidget->saveDialog();
	}
};

struct ModuleLoadItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(event::Action &e) override {
		moduleWidget->loadDialog();
	}
};

struct ModuleCloneItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(event::Action &e) override {
		context()->scene->rackWidget->cloneModule(moduleWidget);
	}
};

struct ModuleDeleteItem : MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(event::Action &e) override {
		context()->scene->rackWidget->deleteModule(moduleWidget);
		delete moduleWidget;
	}
};

Menu *ModuleWidget::createContextMenu() {
	Menu *menu = createMenu();

	MenuLabel *menuLabel = new MenuLabel;
	menuLabel->text = model->author + " " + model->name + " " + model->plugin->version;
	menu->addChild(menuLabel);

	ModuleResetItem *resetItem = new ModuleResetItem;
	resetItem->text = "Initialize";
	resetItem->rightText = WINDOW_MOD_KEY_NAME "+I";
	resetItem->moduleWidget = this;
	menu->addChild(resetItem);

	ModuleRandomizeItem *randomizeItem = new ModuleRandomizeItem;
	randomizeItem->text = "Randomize";
	randomizeItem->rightText = WINDOW_MOD_KEY_NAME "+R";
	randomizeItem->moduleWidget = this;
	menu->addChild(randomizeItem);

	ModuleDisconnectItem *disconnectItem = new ModuleDisconnectItem;
	disconnectItem->text = "Disconnect cables";
	disconnectItem->rightText = WINDOW_MOD_KEY_NAME "+U";
	disconnectItem->moduleWidget = this;
	menu->addChild(disconnectItem);

	ModuleCloneItem *cloneItem = new ModuleCloneItem;
	cloneItem->text = "Duplicate";
	cloneItem->rightText = WINDOW_MOD_KEY_NAME "+D";
	cloneItem->moduleWidget = this;
	menu->addChild(cloneItem);

	ModuleCopyItem *copyItem = new ModuleCopyItem;
	copyItem->text = "Copy preset";
	copyItem->rightText = WINDOW_MOD_KEY_NAME "+C";
	copyItem->moduleWidget = this;
	menu->addChild(copyItem);

	ModulePasteItem *pasteItem = new ModulePasteItem;
	pasteItem->text = "Paste preset";
	pasteItem->rightText = WINDOW_MOD_KEY_NAME "+V";
	pasteItem->moduleWidget = this;
	menu->addChild(pasteItem);

	ModuleLoadItem *loadItem = new ModuleLoadItem;
	loadItem->text = "Load preset";
	loadItem->moduleWidget = this;
	menu->addChild(loadItem);

	ModuleSaveItem *saveItem = new ModuleSaveItem;
	saveItem->text = "Save preset";
	saveItem->moduleWidget = this;
	menu->addChild(saveItem);

	ModuleDeleteItem *deleteItem = new ModuleDeleteItem;
	deleteItem->text = "Delete";
	deleteItem->rightText = "Backspace/Delete";
	deleteItem->moduleWidget = this;
	menu->addChild(deleteItem);

	appendContextMenu(menu);

	return menu;
}


} // namespace rack