#include "app/ModuleWidget.hpp"
#include "app/Scene.hpp"
#include "app/SvgPanel.hpp"
#include "engine/Engine.hpp"
#include "plugin/Plugin.hpp"
#include "system.hpp"
#include "asset.hpp"
#include "helpers.hpp"
#include "app.hpp"
#include "settings.hpp"
#include "history.hpp"

#include "osdialog.h"
#include <thread>


namespace rack {
namespace app {


static const char PRESET_FILTERS[] = "VCV Rack module preset (.vcvm):vcvm";


struct ModuleUrlItem : ui::MenuItem {
	std::string url;
	void onAction(const event::Action &e) override {
		std::thread t(system::openBrowser, url);
		t.detach();
	}
};

struct ModulePluginItem : ui::MenuItem {
	plugin::Plugin *plugin;
	ui::Menu *createChildMenu() override {
		ui::Menu *menu = new ui::Menu;

		ui::MenuLabel *versionLabel = new ui::MenuLabel;
		versionLabel->text = "v" + plugin->version;
		menu->addChild(versionLabel);

		if (!plugin->author.empty()) {
			if (!plugin->authorUrl.empty()) {
				ModuleUrlItem *authorItem = new ModuleUrlItem;
				authorItem->text = plugin->author;
				authorItem->url = plugin->authorUrl;
				menu->addChild(authorItem);
			}
			else {
				ui::MenuLabel *authorLabel = new ui::MenuLabel;
				authorLabel->text = plugin->author;
				menu->addChild(authorLabel);
			}
		}

		if (!plugin->pluginUrl.empty()) {
			ModuleUrlItem *websiteItem = new ModuleUrlItem;
			websiteItem->text = "Website";
			websiteItem->url = plugin->pluginUrl;
			menu->addChild(websiteItem);
		}

		if (!plugin->manualUrl.empty()) {
			ModuleUrlItem *manualItem = new ModuleUrlItem;
			manualItem->text = "Manual";
			manualItem->url = plugin->manualUrl;
			menu->addChild(manualItem);
		}

		if (!plugin->sourceUrl.empty()) {
			ModuleUrlItem *sourceItem = new ModuleUrlItem;
			sourceItem->text = "Source code";
			sourceItem->url = plugin->sourceUrl;
			menu->addChild(sourceItem);
		}

		if (!plugin->donateUrl.empty()) {
			ModuleUrlItem *donateItem = new ModuleUrlItem;
			donateItem->text = "Donate";
			donateItem->url = plugin->donateUrl;
			menu->addChild(donateItem);
		}

		// TODO open folder location with file explorer instead of browser
		if (!plugin->path.empty()) {
			ModuleUrlItem *pathItem = new ModuleUrlItem;
			pathItem->text = "Open folder";
			pathItem->url = plugin->path;
			menu->addChild(pathItem);
		}

		return menu;
	}
};

struct ModuleDisconnectItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(const event::Action &e) override {
		moduleWidget->disconnectAction();
	}
};

struct ModuleResetItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(const event::Action &e) override {
		moduleWidget->resetAction();
	}
};

struct ModuleRandomizeItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(const event::Action &e) override {
		moduleWidget->randomizeAction();
	}
};

struct ModuleCopyItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(const event::Action &e) override {
		moduleWidget->copyClipboard();
	}
};

struct ModulePasteItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(const event::Action &e) override {
		moduleWidget->pasteClipboardAction();
	}
};

struct ModuleSaveItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(const event::Action &e) override {
		moduleWidget->saveDialog();
	}
};

struct ModuleLoadItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(const event::Action &e) override {
		moduleWidget->loadDialog();
	}
};

struct ModulePresetItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	std::string presetPath;
	void onAction(const event::Action &e) override {
		moduleWidget->loadAction(presetPath);
	}
};

struct ModuleListPresetsItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	ui::Menu *createChildMenu() override {
		ui::Menu *menu = new ui::Menu;

		for (const std::string &presetPath : moduleWidget->model->presetPaths) {
			ModulePresetItem *presetItem = new ModulePresetItem;
			std::string presetName = string::basename(string::filename(presetPath));
			presetItem->text = presetName;
			presetItem->presetPath = presetPath;
			presetItem->moduleWidget = moduleWidget;
			menu->addChild(presetItem);
		}

		return menu;
	}
};

struct ModuleCloneItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(const event::Action &e) override {
		moduleWidget->cloneAction();
	}
};

struct ModuleBypassItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(const event::Action &e) override {
		moduleWidget->bypassAction();
	}
};

struct ModuleDeleteItem : ui::MenuItem {
	ModuleWidget *moduleWidget;
	void onAction(const event::Action &e) override {
		moduleWidget->removeAction();
	}
};


ModuleWidget::ModuleWidget() {
	box.size = math::Vec(0, RACK_GRID_HEIGHT);
}

ModuleWidget::~ModuleWidget() {
	setModule(NULL);
}

void ModuleWidget::draw(const DrawArgs &args) {
	nvgScissor(args.vg, RECT_ARGS(args.clipBox));

	if (module && module->bypass) {
		nvgGlobalAlpha(args.vg, 0.25);
	}

	Widget::draw(args);

	// Power meter
	if (module && settings.cpuMeter) {
		nvgBeginPath(args.vg);
		nvgRect(args.vg,
			0, box.size.y - 20,
			105, 20);
		nvgFillColor(args.vg, nvgRGBAf(0, 0, 0, 0.75));
		nvgFill(args.vg);

		std::string cpuText = string::f("%.2f μs %.1f%%", module->cpuTime * 1e6f, module->cpuTime * APP->engine->getSampleRate() * 100);
		bndLabel(args.vg, 2.0, box.size.y - 20.0, INFINITY, INFINITY, -1, cpuText.c_str());

		float p = math::clamp(module->cpuTime / APP->engine->getSampleTime(), 0.f, 1.f);
		nvgBeginPath(args.vg);
		nvgRect(args.vg,
			0, (1.f - p) * box.size.y,
			5, p * box.size.y);
		nvgFillColor(args.vg, nvgRGBAf(1, 0, 0, 1.0));
		nvgFill(args.vg);
	}

	// if (module) {
	// 	nvgBeginPath(args.vg);
	// 	nvgRect(args.vg, 0, 0, 20, 20);
	// 	nvgFillColor(args.vg, nvgRGBAf(0, 0, 0, 0.75));
	// 	nvgFill(args.vg);

	// 	std::string debugText = string::f("%d", module->id);
	// 	bndLabel(args.vg, 0, 0, INFINITY, INFINITY, -1, debugText.c_str());
	// }

	nvgResetScissor(args.vg);
}

void ModuleWidget::drawShadow(const DrawArgs &args) {
	nvgBeginPath(args.vg);
	float r = 20; // Blur radius
	float c = 20; // Corner radius
	math::Vec b = math::Vec(-10, 30); // Offset from each corner
	nvgRect(args.vg, b.x - r, b.y - r, box.size.x - 2*b.x + 2*r, box.size.y - 2*b.y + 2*r);
	NVGcolor shadowColor = nvgRGBAf(0, 0, 0, 0.2);
	NVGcolor transparentColor = nvgRGBAf(0, 0, 0, 0);
	nvgFillPaint(args.vg, nvgBoxGradient(args.vg, b.x, b.y, box.size.x - 2*b.x, box.size.y - 2*b.y, c, r, shadowColor, transparentColor));
	nvgFill(args.vg);
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

	SvgPanel *svgPanel = new SvgPanel;
	svgPanel->setBackground(svg);
	addChild(svgPanel);
	box.size.x = svgPanel->box.size.x;
	panel = svgPanel;
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

	ui::MenuLabel *modelLabel = new ui::MenuLabel;
	modelLabel->text = model->name;
	menu->addChild(modelLabel);

	ModulePluginItem *pluginItem = new ModulePluginItem;
	pluginItem->text = model->plugin->name;
	pluginItem->rightText = RIGHT_ARROW;
	pluginItem->plugin = model->plugin;
	menu->addChild(pluginItem);

	ModuleResetItem *resetItem = new ModuleResetItem;
	resetItem->text = "Initialize";
	resetItem->rightText = WINDOW_MOD_CTRL_NAME "+I";
	resetItem->moduleWidget = this;
	menu->addChild(resetItem);

	ModuleRandomizeItem *randomizeItem = new ModuleRandomizeItem;
	randomizeItem->text = "Randomize";
	randomizeItem->rightText = WINDOW_MOD_CTRL_NAME "+R";
	randomizeItem->moduleWidget = this;
	menu->addChild(randomizeItem);

	ModuleDisconnectItem *disconnectItem = new ModuleDisconnectItem;
	disconnectItem->text = "Disconnect cables";
	disconnectItem->rightText = WINDOW_MOD_CTRL_NAME "+U";
	disconnectItem->moduleWidget = this;
	menu->addChild(disconnectItem);

	ModuleCloneItem *cloneItem = new ModuleCloneItem;
	cloneItem->text = "Duplicate";
	cloneItem->rightText = WINDOW_MOD_CTRL_NAME "+D";
	cloneItem->moduleWidget = this;
	menu->addChild(cloneItem);

	if (!model->presetPaths.empty()) {
		ModuleListPresetsItem *presetsItem = new ModuleListPresetsItem;
		presetsItem->text = "Factory presets";
		presetsItem->rightText = RIGHT_ARROW;
		presetsItem->moduleWidget = this;
		menu->addChild(presetsItem);
	}

	ModuleCopyItem *copyItem = new ModuleCopyItem;
	copyItem->text = "Copy preset";
	copyItem->rightText = WINDOW_MOD_CTRL_NAME "+C";
	copyItem->moduleWidget = this;
	menu->addChild(copyItem);

	ModulePasteItem *pasteItem = new ModulePasteItem;
	pasteItem->text = "Paste preset";
	pasteItem->rightText = WINDOW_MOD_CTRL_NAME "+V";
	pasteItem->moduleWidget = this;
	menu->addChild(pasteItem);

	ModuleLoadItem *loadItem = new ModuleLoadItem;
	loadItem->text = "Open preset";
	loadItem->moduleWidget = this;
	menu->addChild(loadItem);

	ModuleSaveItem *saveItem = new ModuleSaveItem;
	saveItem->text = "Save preset as";
	saveItem->moduleWidget = this;
	menu->addChild(saveItem);

	ModuleBypassItem *bypassItem = new ModuleBypassItem;
	bypassItem->text = "Disable";
	bypassItem->rightText = WINDOW_MOD_CTRL_NAME "+E";
	if (module && module->bypass)
		bypassItem->rightText = CHECKMARK_STRING " " + bypassItem->rightText;
	bypassItem->moduleWidget = this;
	menu->addChild(bypassItem);

	ModuleDeleteItem *deleteItem = new ModuleDeleteItem;
	deleteItem->text = "Delete";
	deleteItem->rightText = "Backspace/Delete";
	deleteItem->moduleWidget = this;
	menu->addChild(deleteItem);

	appendContextMenu(menu);
}


} // namespace app
} // namespace rack