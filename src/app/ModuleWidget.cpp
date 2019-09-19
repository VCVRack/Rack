#include <app/ModuleWidget.hpp>
#include <app/Scene.hpp>
#include <engine/Engine.hpp>
#include <plugin/Plugin.hpp>
#include <app/SvgPanel.hpp>
#include <system.hpp>
#include <asset.hpp>
#include <helpers.hpp>
#include <app.hpp>
#include <settings.hpp>
#include <history.hpp>

#include <osdialog.h>
#include <thread>


namespace rack {
namespace app {


static const char PRESET_FILTERS[] = "VCV Rack module preset (.vcvm):vcvm";


struct ModuleUrlItem : ui::MenuItem {
	std::string url;
	void onAction(const event::Action& e) override {
		std::thread t(system::openBrowser, url);
		t.detach();
	}
};


struct ModuleFolderItem : ui::MenuItem {
	std::string path;
	void onAction(const event::Action& e) override {
		std::thread t(system::openFolder, path);
		t.detach();
	}
};


struct ModulePluginItem : ui::MenuItem {
	plugin::Plugin* plugin;
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;

		ui::MenuLabel* pluginLabel = new ui::MenuLabel;
		pluginLabel->text = plugin->name;
		menu->addChild(pluginLabel);

		ui::MenuLabel* versionLabel = new ui::MenuLabel;
		versionLabel->text = "v" + plugin->version;
		menu->addChild(versionLabel);

		if (!plugin->author.empty()) {
			if (!plugin->authorUrl.empty()) {
				ModuleUrlItem* authorItem = new ModuleUrlItem;
				authorItem->text = plugin->author;
				authorItem->url = plugin->authorUrl;
				menu->addChild(authorItem);
			}
			else {
				ui::MenuLabel* authorLabel = new ui::MenuLabel;
				authorLabel->text = plugin->author;
				menu->addChild(authorLabel);
			}
		}

		if (!plugin->pluginUrl.empty()) {
			ModuleUrlItem* websiteItem = new ModuleUrlItem;
			websiteItem->text = "Website";
			websiteItem->url = plugin->pluginUrl;
			menu->addChild(websiteItem);
		}

		if (!plugin->manualUrl.empty()) {
			ModuleUrlItem* manualItem = new ModuleUrlItem;
			manualItem->text = "Manual";
			manualItem->url = plugin->manualUrl;
			menu->addChild(manualItem);
		}

		if (!plugin->sourceUrl.empty()) {
			ModuleUrlItem* sourceItem = new ModuleUrlItem;
			sourceItem->text = "Source code";
			sourceItem->url = plugin->sourceUrl;
			menu->addChild(sourceItem);
		}

		if (!plugin->donateUrl.empty()) {
			ModuleUrlItem* donateItem = new ModuleUrlItem;
			donateItem->text = "Donate";
			donateItem->url = plugin->donateUrl;
			menu->addChild(donateItem);
		}

		if (!plugin->path.empty()) {
			ModuleFolderItem* pathItem = new ModuleFolderItem;
			pathItem->text = "Open plugin folder";
			pathItem->path = plugin->path;
			menu->addChild(pathItem);
		}

		return menu;
	}
};


struct ModuleDisconnectItem : ui::MenuItem {
	ModuleWidget* moduleWidget;
	void onAction(const event::Action& e) override {
		moduleWidget->disconnectAction();
	}
};


struct ModuleResetItem : ui::MenuItem {
	ModuleWidget* moduleWidget;
	void onAction(const event::Action& e) override {
		moduleWidget->resetAction();
	}
};


struct ModuleRandomizeItem : ui::MenuItem {
	ModuleWidget* moduleWidget;
	void onAction(const event::Action& e) override {
		moduleWidget->randomizeAction();
	}
};


struct ModuleCopyItem : ui::MenuItem {
	ModuleWidget* moduleWidget;
	void onAction(const event::Action& e) override {
		moduleWidget->copyClipboard();
	}
};


struct ModulePasteItem : ui::MenuItem {
	ModuleWidget* moduleWidget;
	void onAction(const event::Action& e) override {
		moduleWidget->pasteClipboardAction();
	}
};


struct ModuleSaveItem : ui::MenuItem {
	ModuleWidget* moduleWidget;
	void onAction(const event::Action& e) override {
		moduleWidget->saveDialog();
	}
};


struct ModuleLoadItem : ui::MenuItem {
	ModuleWidget* moduleWidget;
	void onAction(const event::Action& e) override {
		moduleWidget->loadDialog();
	}
};


struct ModulePresetPathItem : ui::MenuItem {
	ModuleWidget* moduleWidget;
	std::string presetPath;
	void onAction(const event::Action& e) override {
		moduleWidget->loadAction(presetPath);
	}
};


struct ModulePresetItem : ui::MenuItem {
	ModuleWidget* moduleWidget;
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;

		ModuleCopyItem* copyItem = new ModuleCopyItem;
		copyItem->text = "Copy";
		copyItem->rightText = RACK_MOD_CTRL_NAME "+C";
		copyItem->moduleWidget = moduleWidget;
		menu->addChild(copyItem);

		ModulePasteItem* pasteItem = new ModulePasteItem;
		pasteItem->text = "Paste";
		pasteItem->rightText = RACK_MOD_CTRL_NAME "+V";
		pasteItem->moduleWidget = moduleWidget;
		menu->addChild(pasteItem);

		ModuleLoadItem* loadItem = new ModuleLoadItem;
		loadItem->text = "Open";
		loadItem->moduleWidget = moduleWidget;
		menu->addChild(loadItem);

		ModuleSaveItem* saveItem = new ModuleSaveItem;
		saveItem->text = "Save as";
		saveItem->moduleWidget = moduleWidget;
		menu->addChild(saveItem);

		if (!moduleWidget->model->presetPaths.empty()) {
			menu->addChild(new MenuEntry);
			menu->addChild(createMenuLabel("Factory presets"));

			for (const std::string& presetPath : moduleWidget->model->presetPaths) {
				ModulePresetPathItem* presetItem = new ModulePresetPathItem;
				std::string presetName = string::filenameBase(string::filename(presetPath));
				presetItem->text = presetName;
				presetItem->presetPath = presetPath;
				presetItem->moduleWidget = moduleWidget;
				menu->addChild(presetItem);
			}
		}

		return menu;
	}
};


struct ModuleCloneItem : ui::MenuItem {
	ModuleWidget* moduleWidget;
	void onAction(const event::Action& e) override {
		moduleWidget->cloneAction();
	}
};


struct ModuleBypassItem : ui::MenuItem {
	ModuleWidget* moduleWidget;
	void onAction(const event::Action& e) override {
		moduleWidget->bypassAction();
	}
};


struct ModuleDeleteItem : ui::MenuItem {
	ModuleWidget* moduleWidget;
	void onAction(const event::Action& e) override {
		moduleWidget->removeAction();
	}
};


ModuleWidget::ModuleWidget() {
	box.size = math::Vec(0, RACK_GRID_HEIGHT);
}

ModuleWidget::~ModuleWidget() {
	clearChildren();
	setModule(NULL);
}

void ModuleWidget::draw(const DrawArgs& args) {
	nvgScissor(args.vg, RECT_ARGS(args.clipBox));

	if (module && module->bypass) {
		nvgGlobalAlpha(args.vg, 0.33);
	}

	Widget::draw(args);

	// Power meter
	if (module && settings::cpuMeter && !module->bypass) {
		nvgBeginPath(args.vg);
		nvgRect(args.vg,
		        0, box.size.y - 35,
		        65, 35);
		nvgFillColor(args.vg, nvgRGBAf(0, 0, 0, 0.75));
		nvgFill(args.vg);

		float percent = module->cpuTime * APP->engine->getSampleRate() * 100;
		float microseconds = module->cpuTime * 1e6f;
		std::string cpuText = string::f("%.1f%%\n%.2f μs", percent, microseconds);
		bndLabel(args.vg, 2.0, box.size.y - 34.0, INFINITY, INFINITY, -1, cpuText.c_str());

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

void ModuleWidget::drawShadow(const DrawArgs& args) {
	nvgBeginPath(args.vg);
	float r = 20; // Blur radius
	float c = 20; // Corner radius
	math::Vec b = math::Vec(-10, 30); // Offset from each corner
	nvgRect(args.vg, b.x - r, b.y - r, box.size.x - 2 * b.x + 2 * r, box.size.y - 2 * b.y + 2 * r);
	NVGcolor shadowColor = nvgRGBAf(0, 0, 0, 0.2);
	NVGcolor transparentColor = nvgRGBAf(0, 0, 0, 0);
	nvgFillPaint(args.vg, nvgBoxGradient(args.vg, b.x, b.y, box.size.x - 2 * b.x, box.size.y - 2 * b.y, c, r, shadowColor, transparentColor));
	nvgFill(args.vg);
}

void ModuleWidget::onButton(const event::Button& e) {
	OpaqueWidget::onButton(e);
	if (e.isConsumed())
		return;

	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
		createContextMenu();
		e.consume(this);
	}
}

void ModuleWidget::onHoverKey(const event::HoverKey& e) {
	OpaqueWidget::onHoverKey(e);
	if (e.isConsumed())
		return;

	if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		switch (e.key) {
			case GLFW_KEY_I: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					resetAction();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_R: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					randomizeAction();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_C: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					copyClipboard();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_V: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					pasteClipboardAction();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_D: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					cloneAction();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_U: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					disconnectAction();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_E: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					bypassAction();
					e.consume(this);
				}
			} break;
		}
	}

	if (e.action == RACK_HELD) {
		switch (e.key) {
			case GLFW_KEY_DELETE:
			case GLFW_KEY_BACKSPACE: {
				if ((e.mods & RACK_MOD_MASK) == 0) {
					removeAction();
					e.consume(NULL);
				}
			} break;
		}
	}
}

void ModuleWidget::onDragStart(const event::DragStart& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	oldPos = box.pos;
	dragPos = APP->scene->rack->mousePos.minus(box.pos);
	APP->scene->rack->updateModuleDragPositions();
}

void ModuleWidget::onDragEnd(const event::DragEnd& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	history::ComplexAction* h = APP->scene->rack->getModuleDragAction();
	if (!h) {
		delete h;
		return;
	}
	APP->history->push(h);
}

void ModuleWidget::onDragMove(const event::DragMove& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	if (!settings::lockModules) {
		math::Vec pos = APP->scene->rack->mousePos.minus(dragPos);
		if ((APP->window->getMods() & RACK_MOD_MASK) == RACK_MOD_CTRL)
			APP->scene->rack->setModulePosForce(this, pos);
		else
			APP->scene->rack->setModulePosNearest(this, pos);
	}
}

void ModuleWidget::setModule(engine::Module* module) {
	if (this->module) {
		delete this->module;
	}
	this->module = module;
}

void ModuleWidget::setPanel(std::shared_ptr<Svg> svg) {
	// Remove existing panel
	if (panel) {
		removeChild(panel);
		delete panel;
		panel = NULL;
	}

	// Create SvgPanel
	SvgPanel* svgPanel = new SvgPanel;
	svgPanel->setBackground(svg);
	panel = svgPanel;
	addChildBottom(panel);

	// Set ModuleWidget size based on panel
	box.size.x = std::round(panel->box.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
}

void ModuleWidget::addParam(ParamWidget* param) {
	params.push_back(param);
	addChild(param);
}

void ModuleWidget::addOutput(PortWidget* output) {
	// Check that the port is an output
	assert(output->type == PortWidget::OUTPUT);
	// Check that the port doesn't have a duplicate ID
	for (PortWidget* output2 : outputs) {
		assert(output->portId != output2->portId);
	}
	// Add port
	outputs.push_back(output);
	addChild(output);
}

void ModuleWidget::addInput(PortWidget* input) {
	// Check that the port is an input
	assert(input->type == PortWidget::INPUT);
	// Check that the port doesn't have a duplicate ID
	for (PortWidget* input2 : inputs) {
		assert(input->portId != input2->portId);
	}
	// Add port
	inputs.push_back(input);
	addChild(input);
}

ParamWidget* ModuleWidget::getParam(int paramId) {
	for (ParamWidget* param : params) {
		if (param->paramQuantity && param->paramQuantity->paramId == paramId)
			return param;
	}
	return NULL;
}

PortWidget* ModuleWidget::getOutput(int outputId) {
	for (PortWidget* port : outputs) {
		if (port->portId == outputId)
			return port;
	}
	return NULL;
}

PortWidget* ModuleWidget::getInput(int inputId) {
	for (PortWidget* port : inputs) {
		if (port->portId == inputId)
			return port;
	}
	return NULL;
}

json_t* ModuleWidget::toJson() {
	if (!module)
		return NULL;

	json_t* rootJ = module->toJson();
	return rootJ;
}

void ModuleWidget::fromJson(json_t* rootJ) {
	if (!module)
		return;
	module->fromJson(rootJ);
}

void ModuleWidget::copyClipboard() {
	json_t* moduleJ = toJson();
	DEFER({
		json_decref(moduleJ);
	});
	char* moduleJson = json_dumps(moduleJ, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
	DEFER({
		free(moduleJson);
	});
	glfwSetClipboardString(APP->window->win, moduleJson);
}

void ModuleWidget::pasteClipboardAction() {
	const char* moduleJson = glfwGetClipboardString(APP->window->win);
	if (!moduleJson) {
		WARN("Could not get text from clipboard.");
		return;
	}

	json_error_t error;
	json_t* moduleJ = json_loads(moduleJson, 0, &error);
	if (!moduleJ) {
		WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		return;
	}
	DEFER({
		json_decref(moduleJ);
	});

	// history::ModuleChange
	history::ModuleChange* h = new history::ModuleChange;
	h->name = "paste module preset";
	h->moduleId = module->id;
	h->oldModuleJ = toJson();

	fromJson(moduleJ);

	h->newModuleJ = toJson();
	APP->history->push(h);
}

void ModuleWidget::loadAction(std::string filename) {
	INFO("Loading preset %s", filename.c_str());

	FILE* file = fopen(filename.c_str(), "r");
	if (!file) {
		WARN("Could not load patch file %s", filename.c_str());
		return;
	}
	DEFER({
		fclose(file);
	});

	json_error_t error;
	json_t* moduleJ = json_loadf(file, 0, &error);
	if (!moduleJ) {
		std::string message = string::f("File is not a valid patch file. JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
		return;
	}
	DEFER({
		json_decref(moduleJ);
	});

	// history::ModuleChange
	history::ModuleChange* h = new history::ModuleChange;
	h->name = "load module preset";
	h->moduleId = module->id;
	h->oldModuleJ = toJson();

	fromJson(moduleJ);

	h->newModuleJ = toJson();
	APP->history->push(h);
}

void ModuleWidget::save(std::string filename) {
	INFO("Saving preset %s", filename.c_str());

	json_t* moduleJ = toJson();
	assert(moduleJ);
	DEFER({
		json_decref(moduleJ);
	});

	FILE* file = fopen(filename.c_str(), "w");
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

	osdialog_filters* filters = osdialog_filters_parse(PRESET_FILTERS);
	DEFER({
		osdialog_filters_free(filters);
	});

	char* path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, filters);
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

	osdialog_filters* filters = osdialog_filters_parse(PRESET_FILTERS);
	DEFER({
		osdialog_filters_free(filters);
	});

	char* path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), "Untitled.vcvm", filters);
	if (!path) {
		// No path selected
		return;
	}
	DEFER({
		free(path);
	});

	std::string pathStr = path;
	std::string extension = string::filenameExtension(string::filename(pathStr));
	if (extension.empty()) {
		pathStr += ".vcvm";
	}

	save(pathStr);
}

void ModuleWidget::disconnect() {
	for (PortWidget* input : inputs) {
		APP->scene->rack->clearCablesOnPort(input);
	}
	for (PortWidget* output : outputs) {
		APP->scene->rack->clearCablesOnPort(output);
	}
}

void ModuleWidget::resetAction() {
	assert(module);

	// history::ModuleChange
	history::ModuleChange* h = new history::ModuleChange;
	h->name = "reset module";
	h->moduleId = module->id;
	h->oldModuleJ = toJson();

	for (ParamWidget* param : params) {
		param->reset();
	}
	APP->engine->resetModule(module);

	h->newModuleJ = toJson();
	APP->history->push(h);
}

void ModuleWidget::randomizeAction() {
	assert(module);

	// history::ModuleChange
	history::ModuleChange* h = new history::ModuleChange;
	h->name = "randomize module";
	h->moduleId = module->id;
	h->oldModuleJ = toJson();

	for (ParamWidget* param : params) {
		param->randomize();
	}
	APP->engine->randomizeModule(module);

	h->newModuleJ = toJson();
	APP->history->push(h);
}

static void disconnectActions(ModuleWidget* mw, history::ComplexAction* complexAction) {
	// Add CableRemove action for all cables attached to outputs
	for (PortWidget* output : mw->outputs) {
		for (CableWidget* cw : APP->scene->rack->getCablesOnPort(output)) {
			if (!cw->isComplete())
				continue;
			// history::CableRemove
			history::CableRemove* h = new history::CableRemove;
			h->setCable(cw);
			complexAction->push(h);
		}
	}
	// Add CableRemove action for all cables attached to inputs
	for (PortWidget* input : mw->inputs) {
		for (CableWidget* cw : APP->scene->rack->getCablesOnPort(input)) {
			if (!cw->isComplete())
				continue;
			// Avoid creating duplicate actions for self-patched cables
			if (cw->outputPort->module == mw->module)
				continue;
			// history::CableRemove
			history::CableRemove* h = new history::CableRemove;
			h->setCable(cw);
			complexAction->push(h);
		}
	}
}

void ModuleWidget::disconnectAction() {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "disconnect cables";
	disconnectActions(this, complexAction);
	APP->history->push(complexAction);

	disconnect();
}

void ModuleWidget::cloneAction() {
	ModuleWidget* clonedModuleWidget = model->createModuleWidget();
	assert(clonedModuleWidget);
	// JSON serialization is the obvious way to do this
	json_t* moduleJ = toJson();
	clonedModuleWidget->fromJson(moduleJ);
	json_decref(moduleJ);

	// Reset ID so the Engine automatically assigns a new one
	clonedModuleWidget->module->id = -1;

	APP->scene->rack->addModuleAtMouse(clonedModuleWidget);

	// history::ModuleAdd
	history::ModuleAdd* h = new history::ModuleAdd;
	h->name = "clone modules";
	h->setModule(clonedModuleWidget);
	APP->history->push(h);
}

void ModuleWidget::bypassAction() {
	assert(module);
	// history::ModuleBypass
	history::ModuleBypass* h = new history::ModuleBypass;
	h->moduleId = module->id;
	h->bypass = !module->bypass;
	APP->history->push(h);
	h->redo();
}

void ModuleWidget::removeAction() {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "remove module";
	disconnectActions(this, complexAction);

	// history::ModuleRemove
	history::ModuleRemove* moduleRemove = new history::ModuleRemove;
	moduleRemove->setModule(this);
	complexAction->push(moduleRemove);

	APP->history->push(complexAction);

	// This disconnects cables, removes the module, and transfers ownership to caller
	APP->scene->rack->removeModule(this);
	delete this;
}

void ModuleWidget::createContextMenu() {
	ui::Menu* menu = createMenu();
	assert(model);

	ui::MenuLabel* modelLabel = new ui::MenuLabel;
	modelLabel->text = model->plugin->brand + " " + model->name;
	menu->addChild(modelLabel);

	ModulePluginItem* pluginItem = new ModulePluginItem;
	pluginItem->text = "Plugin";
	pluginItem->rightText = RIGHT_ARROW;
	pluginItem->plugin = model->plugin;
	menu->addChild(pluginItem);

	ModulePresetItem* presetsItem = new ModulePresetItem;
	presetsItem->text = "Preset";
	presetsItem->rightText = RIGHT_ARROW;
	presetsItem->moduleWidget = this;
	menu->addChild(presetsItem);

	ModuleResetItem* resetItem = new ModuleResetItem;
	resetItem->text = "Initialize";
	resetItem->rightText = RACK_MOD_CTRL_NAME "+I";
	resetItem->moduleWidget = this;
	menu->addChild(resetItem);

	ModuleRandomizeItem* randomizeItem = new ModuleRandomizeItem;
	randomizeItem->text = "Randomize";
	randomizeItem->rightText = RACK_MOD_CTRL_NAME "+R";
	randomizeItem->moduleWidget = this;
	menu->addChild(randomizeItem);

	ModuleDisconnectItem* disconnectItem = new ModuleDisconnectItem;
	disconnectItem->text = "Disconnect cables";
	disconnectItem->rightText = RACK_MOD_CTRL_NAME "+U";
	disconnectItem->moduleWidget = this;
	menu->addChild(disconnectItem);

	ModuleCloneItem* cloneItem = new ModuleCloneItem;
	cloneItem->text = "Duplicate";
	cloneItem->rightText = RACK_MOD_CTRL_NAME "+D";
	cloneItem->moduleWidget = this;
	menu->addChild(cloneItem);

	ModuleBypassItem* bypassItem = new ModuleBypassItem;
	bypassItem->text = "Disable";
	bypassItem->rightText = RACK_MOD_CTRL_NAME "+E";
	if (module && module->bypass)
		bypassItem->rightText = CHECKMARK_STRING " " + bypassItem->rightText;
	bypassItem->moduleWidget = this;
	menu->addChild(bypassItem);

	ModuleDeleteItem* deleteItem = new ModuleDeleteItem;
	deleteItem->text = "Delete";
	deleteItem->rightText = "Backspace/Delete";
	deleteItem->moduleWidget = this;
	menu->addChild(deleteItem);

	appendContextMenu(menu);
}


} // namespace app
} // namespace rack