#include "app/ModuleWidget.hpp"
#include "engine/Engine.hpp"
#include "system.hpp"
#include "asset.hpp"
#include "app/Scene.hpp"
#include "app/SVGPanel.hpp"
#include "helpers.hpp"
#include "app.hpp"
#include "settings.hpp"

#include "osdialog.h"


namespace rack {


ModuleWidget::ModuleWidget(Module *module) {
	if (module) {
		app()->engine->addModule(module);
	}
	this->module = module;
}

ModuleWidget::~ModuleWidget() {
	// HACK
	// If we try to disconnect wires in the Module Browser (e.g. when Rack is closed while the Module Browser is open), app()->scene->rackWidget will be an invalid pointer.
	// So only attempt to disconnect if the module is not NULL.
	if (module)
		disconnect();
	// Remove and delete the Module instance
	if (module) {
		app()->engine->removeModule(module);
		delete module;
		module = NULL;
	}
}

void ModuleWidget::addInput(PortWidget *input) {
	assert(input->type == PortWidget::INPUT);
	inputs.push_back(input);
	addChild(input);
}

void ModuleWidget::addOutput(PortWidget *output) {
	assert(output->type == PortWidget::OUTPUT);
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

	{
		SVGPanel *panel = new SVGPanel;
		panel->setBackground(svg);
		addChild(panel);
		box.size = panel->box.size;
		this->panel = panel;
	}
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
	DEFER({
		json_decref(moduleJ);
	});
	char *moduleJson = json_dumps(moduleJ, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
	DEFER({
		free(moduleJson);
	});
	glfwSetClipboardString(app()->window->win, moduleJson);
}

void ModuleWidget::pasteClipboard() {
	const char *moduleJson = glfwGetClipboardString(app()->window->win);
	if (!moduleJson) {
		WARN("Could not get text from clipboard.");
		return;
	}

	json_error_t error;
	json_t *moduleJ = json_loads(moduleJson, 0, &error);
	if (!moduleJ) {
		WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		return;
	}
	DEFER({
		json_decref(moduleJ);
	});

	fromJson(moduleJ);
}

void ModuleWidget::load(std::string filename) {
	INFO("Loading preset %s", filename.c_str());

	FILE *file = fopen(filename.c_str(), "r");
	if (!file) {
		WARN("Could not load patch file %s", filename.c_str());
		return;
	}
	DEFER({
		fclose(file);
	});

	json_error_t error;
	json_t *moduleJ = json_loadf(file, 0, &error);
	if (!moduleJ) {
		std::string message = string::f("File is not a valid patch file. JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
		return;
	}
	DEFER({
		json_decref(moduleJ);
	});

	fromJson(moduleJ);
}

void ModuleWidget::save(std::string filename) {
	INFO("Saving preset %s", filename.c_str());

	json_t *moduleJ = toJson();
	assert(moduleJ);
	DEFER({
		json_decref(moduleJ);
	});

	FILE *file = fopen(filename.c_str(), "w");
	if (!file) {
		WARN("Could not write to patch file %s", filename.c_str());
	}
	DEFER({
		fclose(file);
	});

	json_dumpf(moduleJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
}

void ModuleWidget::loadDialog() {
	std::string dir = asset::user("presets");
	system::createDirectory(dir);

	osdialog_filters *filters = osdialog_filters_parse(PRESET_FILTERS.c_str());
	DEFER({
		osdialog_filters_free(filters);
	});

	char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, filters);
	if (!path) {
		// No path selected
		return;
	}
	DEFER({
		free(path);
	});

	load(path);
}

void ModuleWidget::saveDialog() {
	std::string dir = asset::user("presets");
	system::createDirectory(dir);

	osdialog_filters *filters = osdialog_filters_parse(PRESET_FILTERS.c_str());
	DEFER({
		osdialog_filters_free(filters);
	});

	char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), "Untitled.vcvm", filters);
	if (!path) {
		// No path selected
		return;
	}
	DEFER({
		free(path);
	});

	std::string pathStr = path;
	std::string extension = string::extension(pathStr);
	if (extension.empty()) {
		pathStr += ".vcvm";
	}

	save(pathStr);
}

void ModuleWidget::toggleBypass() {
	if (!module)
		return;
	module->bypass ^= true;
}

void ModuleWidget::disconnect() {
	for (PortWidget *input : inputs) {
		app()->scene->rackWidget->wireContainer->removeAllWires(input);
	}
	for (PortWidget *output : outputs) {
		app()->scene->rackWidget->wireContainer->removeAllWires(output);
	}
}

void ModuleWidget::reset() {
	if (module) {
		app()->engine->resetModule(module);
	}
}

void ModuleWidget::randomize() {
	if (module) {
		app()->engine->randomizeModule(module);
	}
}

void ModuleWidget::draw(NVGcontext *vg) {
	if (module && module->bypass) {
		nvgGlobalAlpha(vg, 0.5);
	}
	// nvgScissor(vg, 0, 0, box.size.x, box.size.y);
	Widget::draw(vg);

	// Power meter
	if (module && settings::powerMeter) {
		nvgBeginPath(vg);
		nvgRect(vg,
			0, box.size.y - 20,
			55, 20);
		nvgFillColor(vg, nvgRGBAf(0, 0, 0, 0.5));
		nvgFill(vg);

		std::string cpuText = string::f("%.0f mS", module->cpuTime * 1000.f);
		// TODO Use blendish text function
		nvgFontFaceId(vg, app()->window->uiFont->handle);
		nvgFontSize(vg, 12);
		nvgFillColor(vg, nvgRGBf(1, 1, 1));
		nvgText(vg, 10.0, box.size.y - 6.0, cpuText.c_str(), NULL);

		float p = math::clamp(module->cpuTime, 0.f, 1.f);
		nvgBeginPath(vg);
		nvgRect(vg,
			0, (1.f - p) * box.size.y,
			5, p * box.size.y);
		nvgFillColor(vg, nvgRGBAf(1, 0, 0, 1.0));
		nvgFill(vg);
	}

	// nvgResetScissor(vg);
}

void ModuleWidget::drawShadow(NVGcontext *vg) {
	nvgBeginPath(vg);
	float r = 20; // Blur radius
	float c = 20; // Corner radius
	math::Vec b = math::Vec(-10, 30); // Offset from each corner
	nvgRect(vg, b.x - r, b.y - r, box.size.x - 2*b.x + 2*r, box.size.y - 2*b.y + 2*r);
	NVGcolor shadowColor = nvgRGBAf(0, 0, 0, 0.2);
	NVGcolor transparentColor = nvgRGBAf(0, 0, 0, 0);
	nvgFillPaint(vg, nvgBoxGradient(vg, b.x, b.y, box.size.x - 2*b.x, box.size.y - 2*b.y, c, r, shadowColor, transparentColor));
	nvgFill(vg);
}

void ModuleWidget::onHover(const event::Hover &e) {
	OpaqueWidget::onHover(e);

	// Instead of checking key-down events, delete the module even if key-repeat hasn't fired yet and the cursor is hovering over the widget.
	if (glfwGetKey(app()->window->win, GLFW_KEY_DELETE) == GLFW_PRESS || glfwGetKey(app()->window->win, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
		if (!app()->window->isModPressed() && !app()->window->isShiftPressed()) {
			requestedDelete = true;
			return;
		}
	}
}

void ModuleWidget::onButton(const event::Button &e) {
	OpaqueWidget::onButton(e);
	if (e.getConsumed() == this) {
		if (e.button == 1) {
			createContextMenu();
		}
	}
}

void ModuleWidget::onHoverKey(const event::HoverKey &e) {
	if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		switch (e.key) {
			case GLFW_KEY_I: {
				if (app()->window->isModPressed() && !app()->window->isShiftPressed()) {
					reset();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_R: {
				if (app()->window->isModPressed() && !app()->window->isShiftPressed()) {
					randomize();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_C: {
				if (app()->window->isModPressed() && !app()->window->isShiftPressed()) {
					copyClipboard();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_V: {
				if (app()->window->isModPressed() && !app()->window->isShiftPressed()) {
					pasteClipboard();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_D: {
				if (app()->window->isModPressed() && !app()->window->isShiftPressed()) {
					app()->scene->rackWidget->cloneModule(this);
					e.consume(this);
				}
			} break;
			case GLFW_KEY_U: {
				if (app()->window->isModPressed() && !app()->window->isShiftPressed()) {
					disconnect();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_E: {
				if (app()->window->isModPressed() && !app()->window->isShiftPressed()) {
					toggleBypass();
					e.consume(this);
				}
			} break;
		}
	}

	if (!e.getConsumed())
		OpaqueWidget::onHoverKey(e);
}

void ModuleWidget::onDragStart(const event::DragStart &e) {
	dragPos = app()->scene->rackWidget->lastMousePos.minus(box.pos);
}

void ModuleWidget::onDragEnd(const event::DragEnd &e) {
}

void ModuleWidget::onDragMove(const event::DragMove &e) {
	if (!settings::lockModules) {
		math::Rect newBox = box;
		newBox.pos = app()->scene->rackWidget->lastMousePos.minus(dragPos);
		app()->scene->rackWidget->requestModuleBoxNearest(this, newBox);
	}
}


struct ModuleDisconnectItem : MenuItem {
	ModuleWidget *moduleWidget;
	ModuleDisconnectItem() {
		text = "Disconnect cables";
		rightText = WINDOW_MOD_KEY_NAME "+U";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->disconnect();
	}
};

struct ModuleResetItem : MenuItem {
	ModuleWidget *moduleWidget;
	ModuleResetItem() {
		text = "Initialize";
		rightText = WINDOW_MOD_KEY_NAME "+I";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->reset();
	}
};

struct ModuleRandomizeItem : MenuItem {
	ModuleWidget *moduleWidget;
	ModuleRandomizeItem() {
		text = "Randomize";
		rightText = WINDOW_MOD_KEY_NAME "+R";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->randomize();
	}
};

struct ModuleCopyItem : MenuItem {
	ModuleWidget *moduleWidget;
	ModuleCopyItem() {
		text = "Copy preset";
		rightText = WINDOW_MOD_KEY_NAME "+C";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->copyClipboard();
	}
};

struct ModulePasteItem : MenuItem {
	ModuleWidget *moduleWidget;
	ModulePasteItem() {
		text = "Paste preset";
		rightText = WINDOW_MOD_KEY_NAME "+V";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->pasteClipboard();
	}
};

struct ModuleSaveItem : MenuItem {
	ModuleWidget *moduleWidget;
	ModuleSaveItem() {
		text = "Save preset as";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->saveDialog();
	}
};

struct ModuleLoadItem : MenuItem {
	ModuleWidget *moduleWidget;
	ModuleLoadItem() {
		text = "Load preset";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->loadDialog();
	}
};

struct ModuleCloneItem : MenuItem {
	ModuleWidget *moduleWidget;
	ModuleCloneItem() {
		text = "Duplicate";
		rightText = WINDOW_MOD_KEY_NAME "+D";
	}
	void onAction(const event::Action &e) override {
		app()->scene->rackWidget->cloneModule(moduleWidget);
	}
};

struct ModuleBypassItem : MenuItem {
	ModuleWidget *moduleWidget;
	ModuleBypassItem() {
		text = "Bypass";
	}
	void step() override {
		rightText = WINDOW_MOD_KEY_NAME "+E";
		if (!moduleWidget->module)
			return;
		if (moduleWidget->module->bypass)
			rightText = CHECKMARK_STRING " " + rightText;
	}
	void onAction(const event::Action &e) override {
		moduleWidget->toggleBypass();
	}
};

struct ModuleDeleteItem : MenuItem {
	ModuleWidget *moduleWidget;
	ModuleDeleteItem() {
		text = "Delete";
		rightText = "Backspace/Delete";
	}
	void onAction(const event::Action &e) override {
		app()->scene->rackWidget->deleteModule(moduleWidget);
		delete moduleWidget;
	}
};

Menu *ModuleWidget::createContextMenu() {
	Menu *menu = createMenu();

	MenuLabel *menuLabel = new MenuLabel;
	menuLabel->text = model->plugin->name + " " + model->name + " " + model->plugin->version;
	menu->addChild(menuLabel);

	ModuleResetItem *resetItem = new ModuleResetItem;
	resetItem->moduleWidget = this;
	menu->addChild(resetItem);

	ModuleRandomizeItem *randomizeItem = new ModuleRandomizeItem;
	randomizeItem->moduleWidget = this;
	menu->addChild(randomizeItem);

	ModuleDisconnectItem *disconnectItem = new ModuleDisconnectItem;
	disconnectItem->moduleWidget = this;
	menu->addChild(disconnectItem);

	ModuleCloneItem *cloneItem = new ModuleCloneItem;
	cloneItem->moduleWidget = this;
	menu->addChild(cloneItem);

	ModuleCopyItem *copyItem = new ModuleCopyItem;
	copyItem->moduleWidget = this;
	menu->addChild(copyItem);

	ModulePasteItem *pasteItem = new ModulePasteItem;
	pasteItem->moduleWidget = this;
	menu->addChild(pasteItem);

	ModuleLoadItem *loadItem = new ModuleLoadItem;
	loadItem->moduleWidget = this;
	menu->addChild(loadItem);

	ModuleSaveItem *saveItem = new ModuleSaveItem;
	saveItem->moduleWidget = this;
	menu->addChild(saveItem);

	ModuleBypassItem *bypassItem = new ModuleBypassItem;
	bypassItem->moduleWidget = this;
	menu->addChild(bypassItem);

	ModuleDeleteItem *deleteItem = new ModuleDeleteItem;
	deleteItem->moduleWidget = this;
	menu->addChild(deleteItem);

	appendContextMenu(menu);

	return menu;
}


} // namespace rack