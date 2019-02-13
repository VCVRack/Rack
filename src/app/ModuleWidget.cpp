#include "app/ModuleWidget.hpp"
#include "engine/Engine.hpp"
#include "system.hpp"
#include "asset.hpp"
#include "app/Scene.hpp"
#include "app/SvgPanel.hpp"
#include "helpers.hpp"
#include "app.hpp"
#include "settings.hpp"
#include "history.hpp"

#include "osdialog.h"


namespace rack {
namespace app {


static const char PRESET_FILTERS[] = "VCV Rack module preset (.vcvm):vcvm";


struct ModuleDisconnectItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	ModuleDisconnectItem() {
		text = "Disconnect cables";
		rightText = WINDOW_MOD_CTRL_NAME "+U";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->disconnectAction();
	}
};

struct ModuleResetItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	ModuleResetItem() {
		text = "Initialize";
		rightText = WINDOW_MOD_CTRL_NAME "+I";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->resetAction();
	}
};

struct ModuleRandomizeItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	ModuleRandomizeItem() {
		text = "Randomize";
		rightText = WINDOW_MOD_CTRL_NAME "+R";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->randomizeAction();
	}
};

struct ModuleCopyItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	ModuleCopyItem() {
		text = "Copy preset";
		rightText = WINDOW_MOD_CTRL_NAME "+C";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->copyClipboard();
	}
};

struct ModulePasteItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	ModulePasteItem() {
		text = "Paste preset";
		rightText = WINDOW_MOD_CTRL_NAME "+V";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->pasteClipboardAction();
	}
};

struct ModuleSaveItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	ModuleSaveItem() {
		text = "Save preset as";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->saveDialog();
	}
};

struct ModuleLoadItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	ModuleLoadItem() {
		text = "Load preset";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->loadDialog();
	}
};

struct ModuleCloneItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	ModuleCloneItem() {
		text = "Duplicate";
		rightText = WINDOW_MOD_CTRL_NAME "+D";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->cloneAction();
	}
};

struct ModuleBypassItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	ModuleBypassItem() {
		text = "Disable";
	}
	void setModule(ModuleWidget *moduleWidget) {
		this->moduleWidget = moduleWidget;
		rightText = WINDOW_MOD_CTRL_NAME "+E";
		if (moduleWidget->module && moduleWidget->module->bypass)
			rightText = CHECKMARK_STRING " " + rightText;
	}
	void onAction(const event::Action &e) override {
		moduleWidget->bypassAction();
	}
};

struct ModuleDeleteItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	ModuleDeleteItem() {
		text = "Delete";
		rightText = "Backspace/Delete";
	}
	void onAction(const event::Action &e) override {
		moduleWidget->removeAction();
	}
};


ModuleWidget::~ModuleWidget() {
	setModule(NULL);
}

void ModuleWidget::draw(const widget::DrawContext &ctx) {
	if (module && module->bypass) {
		nvgGlobalAlpha(ctx.vg, 0.25);
	}
	// nvgScissor(ctx.vg, 0, 0, box.size.x, box.size.y);
	widget::Widget::draw(ctx);

	// Power meter
	if (module && settings.cpuMeter) {
		nvgBeginPath(ctx.vg);
		nvgRect(ctx.vg,
			0, box.size.y - 20,
			105, 20);
		nvgFillColor(ctx.vg, nvgRGBAf(0, 0, 0, 0.75));
		nvgFill(ctx.vg);

		std::string cpuText = string::f("%.2f μs %.1f%%", module->cpuTime * 1e6f, module->cpuTime * APP->engine->getSampleRate() * 100);
		bndLabel(ctx.vg, 2.0, box.size.y - 20.0, INFINITY, INFINITY, -1, cpuText.c_str());

		float p = math::clamp(module->cpuTime / APP->engine->getSampleTime(), 0.f, 1.f);
		nvgBeginPath(ctx.vg);
		nvgRect(ctx.vg,
			0, (1.f - p) * box.size.y,
			5, p * box.size.y);
		nvgFillColor(ctx.vg, nvgRGBAf(1, 0, 0, 1.0));
		nvgFill(ctx.vg);
	}

	// if (module) {
	// 	nvgBeginPath(ctx.vg);
	// 	nvgRect(ctx.vg, 0, 0, 20, 20);
	// 	nvgFillColor(ctx.vg, nvgRGBAf(0, 0, 0, 0.75));
	// 	nvgFill(ctx.vg);

	// 	std::string debugText = string::f("%d", module->id);
	// 	bndLabel(ctx.vg, 0, 0, INFINITY, INFINITY, -1, debugText.c_str());
	// }

	// nvgResetScissor(ctx.vg);
}

void ModuleWidget::drawShadow(const widget::DrawContext &ctx) {
	nvgBeginPath(ctx.vg);
	float r = 20; // Blur radius
	float c = 20; // Corner radius
	math::Vec b = math::Vec(-10, 30); // Offset from each corner
	nvgRect(ctx.vg, b.x - r, b.y - r, box.size.x - 2*b.x + 2*r, box.size.y - 2*b.y + 2*r);
	NVGcolor shadowColor = nvgRGBAf(0, 0, 0, 0.2);
	NVGcolor transparentColor = nvgRGBAf(0, 0, 0, 0);
	nvgFillPaint(ctx.vg, nvgBoxGradient(ctx.vg, b.x, b.y, box.size.x - 2*b.x, box.size.y - 2*b.y, c, r, shadowColor, transparentColor));
	nvgFill(ctx.vg);
}

void ModuleWidget::onHover(const event::Hover &e) {
	widget::OpaqueWidget::onHover(e);

	if (!APP->event->selectedWidget) {
		// Instead of checking key-down events, delete the module even if key-repeat hasn't fired yet and the cursor is hovering over the widget.
		if ((glfwGetKey(APP->window->win, GLFW_KEY_DELETE) == GLFW_PRESS
			|| glfwGetKey(APP->window->win, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
			&& (APP->window->getMods() & WINDOW_MOD_MASK) == 0) {
			removeAction();
			e.consume(NULL);
			return;
		}
	}
}

void ModuleWidget::onButton(const event::Button &e) {
	widget::OpaqueWidget::onButton(e);

	if (e.getConsumed() == this) {
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
			createContextMenu();
		}
	}
}

void ModuleWidget::onHoverKey(const event::HoverKey &e) {
	if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		switch (e.key) {
			case GLFW_KEY_I: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					resetAction();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_R: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					randomizeAction();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_C: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					copyClipboard();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_V: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					pasteClipboardAction();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_D: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					cloneAction();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_U: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					disconnectAction();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_E: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					bypassAction();
					e.consume(this);
				}
			} break;
		}
	}

	if (!e.getConsumed())
		widget::OpaqueWidget::onHoverKey(e);
}

void ModuleWidget::onDragStart(const event::DragStart &e) {
	oldPos = box.pos;
	dragPos = APP->scene->rackWidget->mousePos.minus(box.pos);
	e.consume(this);
}

void ModuleWidget::onDragEnd(const event::DragEnd &e) {
	if (!box.pos.isEqual(oldPos)) {
		// history::ModuleMove
		history::ModuleMove *h = new history::ModuleMove;
		h->moduleId = module->id;
		h->oldPos = oldPos;
		h->newPos = box.pos;
		APP->history->push(h);
	}
}

void ModuleWidget::onDragMove(const event::DragMove &e) {
	if (!settings.lockModules) {
		math::Rect newBox = box;
		newBox.pos = APP->scene->rackWidget->mousePos.minus(dragPos);
		APP->scene->rackWidget->requestModuleBoxNearest(this, newBox);
	}
}

void ModuleWidget::setModule(engine::Module *module) {
	if (this->module) {
		delete this->module;
	}
	this->module = module;
}

void ModuleWidget::setPanel(std::shared_ptr<Svg> svg) {
	// Remove old panel
	if (panel) {
		removeChild(panel);
		delete panel;
		panel = NULL;
	}

	{
		SvgPanel *panel = new SvgPanel;
		panel->setBackground(svg);
		addChild(panel);
		box.size = panel->box.size;
		this->panel = panel;
	}
}

void ModuleWidget::addParam(ParamWidget *param) {
	params.push_back(param);
	addChild(param);
}

void ModuleWidget::addOutput(PortWidget *output) {
	assert(output->type == PortWidget::OUTPUT);
	outputs.push_back(output);
	addChild(output);
}

void ModuleWidget::addInput(PortWidget *input) {
	assert(input->type == PortWidget::INPUT);
	inputs.push_back(input);
	addChild(input);
}

ParamWidget *ModuleWidget::getParam(int paramId) {
	if (0 <= paramId && paramId < (int) params.size())
		return params[paramId];
	return NULL;
}

PortWidget *ModuleWidget::getOutput(int outputId) {
	if (0 <= outputId && outputId < (int) outputs.size())
		return outputs[outputId];
	return NULL;
}

PortWidget *ModuleWidget::getInput(int inputId) {
	if (0 <= inputId && inputId < (int) inputs.size())
		return inputs[inputId];
	return NULL;
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

	// Merge with module JSON
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
			INFO("Patch created with %s v%s, currently using v%s.", pluginSlug.c_str(), version.c_str(), model->plugin->version.c_str());
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
	glfwSetClipboardString(APP->window->win, moduleJson);
}

void ModuleWidget::pasteClipboardAction() {
	const char *moduleJson = glfwGetClipboardString(APP->window->win);
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

	// history::ModuleChange
	history::ModuleChange *h = new history::ModuleChange;
	h->name = "paste module preset";
	h->moduleId = module->id;
	h->oldModuleJ = toJson();

	fromJson(moduleJ);

	h->newModuleJ = toJson();
	APP->history->push(h);
}

void ModuleWidget::loadAction(std::string filename) {
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

	// history::ModuleChange
	history::ModuleChange *h = new history::ModuleChange;
	h->name = "load module preset";
	h->moduleId = module->id;
	h->oldModuleJ = toJson();

	fromJson(moduleJ);

	h->newModuleJ = toJson();
	APP->history->push(h);
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

	osdialog_filters *filters = osdialog_filters_parse(PRESET_FILTERS);
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

	loadAction(path);
}

void ModuleWidget::saveDialog() {
	std::string dir = asset::user("presets");
	system::createDirectory(dir);

	osdialog_filters *filters = osdialog_filters_parse(PRESET_FILTERS);
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

void ModuleWidget::disconnect() {
	for (PortWidget *input : inputs) {
		APP->scene->rackWidget->clearCablesOnPort(input);
	}
	for (PortWidget *output : outputs) {
		APP->scene->rackWidget->clearCablesOnPort(output);
	}
}

void ModuleWidget::resetAction() {
	assert(module);

	// history::ModuleChange
	history::ModuleChange *h = new history::ModuleChange;
	h->name = "reset module";
	h->moduleId = module->id;
	h->oldModuleJ = toJson();

	APP->engine->resetModule(module);

	h->newModuleJ = toJson();
	APP->history->push(h);
}

void ModuleWidget::randomizeAction() {
	assert(module);

	// history::ModuleChange
	history::ModuleChange *h = new history::ModuleChange;
	h->name = "randomize module";
	h->moduleId = module->id;
	h->oldModuleJ = toJson();

	APP->engine->randomizeModule(module);

	h->newModuleJ = toJson();
	APP->history->push(h);
}

static void disconnectActions(ModuleWidget *mw, history::ComplexAction *complexAction) {
	// Add CableRemove action for all cables attached to outputs
	for (PortWidget* output : mw->outputs) {
		for (CableWidget *cw : APP->scene->rackWidget->getCablesOnPort(output)) {
			if (!cw->isComplete())
				continue;
			// history::CableRemove
			history::CableRemove *h = new history::CableRemove;
			h->setCable(cw);
			complexAction->push(h);
		}
	}
	// Add CableRemove action for all cables attached to inputs
	for (PortWidget* input : mw->inputs) {
		for (CableWidget *cw : APP->scene->rackWidget->getCablesOnPort(input)) {
			if (!cw->isComplete())
				continue;
			// Avoid creating duplicate actions for self-patched cables
			if (cw->outputPort->module == mw->module)
				continue;
			// history::CableRemove
			history::CableRemove *h = new history::CableRemove;
			h->setCable(cw);
			complexAction->push(h);
		}
	}
}

void ModuleWidget::disconnectAction() {
	history::ComplexAction *complexAction = new history::ComplexAction;
	complexAction->name = "disconnect cables";
	disconnectActions(this, complexAction);
	APP->history->push(complexAction);

	disconnect();
}

void ModuleWidget::cloneAction() {
	ModuleWidget *clonedModuleWidget = model->createModuleWidget();
	assert(clonedModuleWidget);
	// JSON serialization is the obvious way to do this
	json_t *moduleJ = toJson();
	clonedModuleWidget->fromJson(moduleJ);
	json_decref(moduleJ);

	APP->scene->rackWidget->addModuleAtMouse(clonedModuleWidget);

	// history::ModuleAdd
	history::ModuleAdd *h = new history::ModuleAdd;
	h->name = "clone modules";
	h->setModule(clonedModuleWidget);
	APP->history->push(h);
}

void ModuleWidget::bypassAction() {
	assert(module);
	// history::ModuleBypass
	history::ModuleBypass *h = new history::ModuleBypass;
	h->moduleId = module->id;
	h->bypass = !module->bypass;
	APP->history->push(h);
	h->redo();
}

void ModuleWidget::removeAction() {
	history::ComplexAction *complexAction = new history::ComplexAction;
	complexAction->name = "remove module";
	disconnectActions(this, complexAction);

	// history::ModuleRemove
	history::ModuleRemove *moduleRemove = new history::ModuleRemove;
	moduleRemove->setModule(this);
	complexAction->push(moduleRemove);

	APP->history->push(complexAction);

	// This disconnects cables, removes the module, and transfers ownership to caller
	APP->scene->rackWidget->removeModule(this);
	delete this;
}

void ModuleWidget::createContextMenu() {
	ui::Menu *menu = createMenu();
	assert(model);

	ui::MenuLabel *menuLabel = new ui::MenuLabel;
	menuLabel->text = model->plugin->name + " " + model->name + " v" + model->plugin->version;
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
	bypassItem->setModule(this);
	menu->addChild(bypassItem);

	ModuleDeleteItem *deleteItem = new ModuleDeleteItem;
	deleteItem->moduleWidget = this;
	menu->addChild(deleteItem);

	appendContextMenu(menu);
}


} // namespace app
} // namespace rack