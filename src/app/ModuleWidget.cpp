#include <thread>
#include <regex>

#include <osdialog.h>

#include <app/ModuleWidget.hpp>
#include <app/Scene.hpp>
#include <engine/Engine.hpp>
#include <plugin/Plugin.hpp>
#include <app/SvgPanel.hpp>
#include <ui/MenuSeparator.hpp>
#include <system.hpp>
#include <asset.hpp>
#include <helpers.hpp>
#include <context.hpp>
#include <settings.hpp>
#include <history.hpp>
#include <string.hpp>
#include <tag.hpp>


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


struct ModuleInfoItem : ui::MenuItem {
	plugin::Model* model;
	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;

		// plugin
		ModuleUrlItem* pluginItem = new ModuleUrlItem;
		pluginItem->text = "Plugin: " + model->plugin->name + " v" + model->plugin->version;
		if (model->plugin->pluginUrl != "") {
			pluginItem->url = model->plugin->pluginUrl;
		}
		else {
			pluginItem->disabled = true;
		}
		menu->addChild(pluginItem);

		// ui::MenuLabel* versionLabel = new ui::MenuLabel;
		// versionLabel->text = "v" + model->plugin->version;
		// menu->addChild(versionLabel);

		// author
		if (model->plugin->author != "") {
			ModuleUrlItem* authorItem = new ModuleUrlItem;
			authorItem->text = "Author: " + model->plugin->author;
			if (model->plugin->authorUrl != "") {
				authorItem->url = model->plugin->authorUrl;
			}
			else {
				authorItem->disabled = true;
			}
			menu->addChild(authorItem);
		}

		// license
		if (model->plugin->license != "") {
			ui::MenuLabel* licenseLabel = new ui::MenuLabel;
			licenseLabel->text = "License: " + model->plugin->license;
			menu->addChild(licenseLabel);
		}

		// tags
		if (!model->tags.empty()) {
			ui::MenuLabel* tagsLabel = new ui::MenuLabel;
			tagsLabel->text = "Tags:";
			menu->addChild(tagsLabel);
			for (int tagId : model->tags) {
				ui::MenuLabel* tagLabel = new ui::MenuLabel;
				tagLabel->text = "• " + tag::getTag(tagId);
				menu->addChild(tagLabel);
			}
		}

		menu->addChild(new ui::MenuSeparator);

		// library
		ModuleUrlItem* libraryItem = new ModuleUrlItem;
		libraryItem->text = "VCV Library entry";
		libraryItem->url = "https://library.vcvrack.com/" + model->plugin->slug + "/" + model->slug;
		menu->addChild(libraryItem);

		// manual
		std::string manualUrl = (model->manualUrl != "") ? model->manualUrl : model->plugin->manualUrl;
		if (manualUrl != "") {
			ModuleUrlItem* manualItem = new ModuleUrlItem;
			manualItem->text = "User manual";
			manualItem->url = manualUrl;
			menu->addChild(manualItem);
		}

		// donate
		if (model->plugin->donateUrl != "") {
			ModuleUrlItem* donateItem = new ModuleUrlItem;
			donateItem->text = "Donate";
			donateItem->url = model->plugin->donateUrl;
			menu->addChild(donateItem);
		}

		// changelog
		if (model->plugin->changelogUrl != "") {
			ModuleUrlItem* changelogItem = new ModuleUrlItem;
			changelogItem->text = "Changelog";
			changelogItem->url = model->plugin->changelogUrl;
			menu->addChild(changelogItem);
		}

		// source code
		if (model->plugin->sourceUrl != "") {
			ModuleUrlItem* sourceItem = new ModuleUrlItem;
			sourceItem->text = "Source code";
			sourceItem->url = model->plugin->sourceUrl;
			menu->addChild(sourceItem);
		}

		// plugin folder
		if (model->plugin->path != "") {
			ModuleFolderItem* pathItem = new ModuleFolderItem;
			pathItem->text = "Open plugin folder";
			pathItem->path = model->plugin->path;
			menu->addChild(pathItem);
		}

		return menu;
	}
};


struct ModuleDisconnectItem : ui::MenuItem {
	WeakPtr<ModuleWidget> moduleWidget;
	void onAction(const event::Action& e) override {
		if (!moduleWidget)
			return;
		moduleWidget->disconnectAction();
	}
};


struct ModuleResetItem : ui::MenuItem {
	WeakPtr<ModuleWidget> moduleWidget;
	void onAction(const event::Action& e) override {
		if (!moduleWidget)
			return;
		moduleWidget->resetAction();
	}
};


struct ModuleRandomizeItem : ui::MenuItem {
	WeakPtr<ModuleWidget> moduleWidget;
	void onAction(const event::Action& e) override {
		if (!moduleWidget)
			return;
		moduleWidget->randomizeAction();
	}
};


struct ModuleCopyItem : ui::MenuItem {
	WeakPtr<ModuleWidget> moduleWidget;
	void onAction(const event::Action& e) override {
		if (!moduleWidget)
			return;
		moduleWidget->copyClipboard();
	}
};


struct ModulePasteItem : ui::MenuItem {
	WeakPtr<ModuleWidget> moduleWidget;
	void onAction(const event::Action& e) override {
		if (!moduleWidget)
			return;
		moduleWidget->pasteClipboardAction();
	}
};


struct ModuleSaveItem : ui::MenuItem {
	WeakPtr<ModuleWidget> moduleWidget;
	void onAction(const event::Action& e) override {
		if (!moduleWidget)
			return;
		moduleWidget->saveDialog();
	}
};


struct ModuleSaveTemplateItem : ui::MenuItem {
	WeakPtr<ModuleWidget> moduleWidget;
	void onAction(const event::Action& e) override {
		if (!moduleWidget)
			return;
		moduleWidget->saveTemplate();
	}
};


struct ModuleLoadItem : ui::MenuItem {
	WeakPtr<ModuleWidget> moduleWidget;
	void onAction(const event::Action& e) override {
		if (!moduleWidget)
			return;
		moduleWidget->loadDialog();
	}
};


struct ModulePresetPathItem : ui::MenuItem {
	WeakPtr<ModuleWidget> moduleWidget;
	std::string presetPath;
	void onAction(const event::Action& e) override {
		if (!moduleWidget)
			return;
		try {
			moduleWidget->loadAction(presetPath);
		}
		catch (Exception& e) {
			osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, e.what());
		}
	}
};


struct ModulePresetItem : ui::MenuItem {
	WeakPtr<ModuleWidget> moduleWidget;
	ui::Menu* createChildMenu() override {
		if (!moduleWidget)
			return NULL;
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

		ModuleSaveTemplateItem* saveTemplateItem = new ModuleSaveTemplateItem;
		saveTemplateItem->text = "Save template";
		saveTemplateItem->moduleWidget = moduleWidget;
		menu->addChild(saveTemplateItem);

		// Create ModulePresetPathItems for each patch in a directory.
		auto createPresetItems = [&](std::string presetDir) {
			bool hasPresets = false;
			// Note: This is not cached, so opening this menu each time might have a bit of latency.
			if (system::isDirectory(presetDir)) {
				for (const std::string& presetPath : system::getEntries(presetDir)) {
					if (system::getExtension(presetPath) != ".vcvm")
						continue;
					hasPresets = true;

					std::string presetName = system::getStem(presetPath);
					// Remove "1_", "42_", "001_", etc at the beginning of preset filenames
					std::regex r("^\\d*_");
					presetName = std::regex_replace(presetName, r, "");

					ModulePresetPathItem* presetItem = new ModulePresetPathItem;
					presetItem->text = presetName;
					presetItem->presetPath = presetPath;
					presetItem->moduleWidget = moduleWidget;
					menu->addChild(presetItem);
				}
			}
			if (!hasPresets) {
				menu->addChild(createMenuLabel("(None)"));
			}
		};

		// Scan `<user dir>/presets/<plugin slug>/<module slug>` for presets.
		menu->addChild(new ui::MenuSeparator);
		menu->addChild(createMenuLabel("User presets"));
		createPresetItems(moduleWidget->model->getUserPresetDir());

		// Scan `<plugin dir>/presets/<module slug>` for presets.
		menu->addChild(new ui::MenuSeparator);
		menu->addChild(createMenuLabel("Factory presets"));
		createPresetItems(moduleWidget->model->getFactoryPresetDir());

		return menu;
	}
};


struct ModuleCloneItem : ui::MenuItem {
	WeakPtr<ModuleWidget> moduleWidget;
	void onAction(const event::Action& e) override {
		if (!moduleWidget)
			return;
		moduleWidget->cloneAction();
	}
};


struct ModuleBypassItem : ui::MenuItem {
	WeakPtr<ModuleWidget> moduleWidget;
	void onAction(const event::Action& e) override {
		if (!moduleWidget)
			return;
		moduleWidget->bypassAction();
	}
};


struct ModuleDeleteItem : ui::MenuItem {
	WeakPtr<ModuleWidget> moduleWidget;
	void onAction(const event::Action& e) override {
		if (!moduleWidget)
			return;
		moduleWidget->removeAction();
	}
};


struct ModuleWidget::Internal {
	/** The position the user clicked on the module to start dragging in the RackWidget.
	*/
	math::Vec dragPos;
	/** The position in the RackWidget when dragging began.
	Used for history::ModuleMove.
	Set by RackWidget::updateModuleOldPositions() when *any* module begins dragging, since force-dragging can move other modules around.
	*/
	math::Vec oldPos;

	widget::Widget* panel = NULL;
};


ModuleWidget::ModuleWidget() {
	internal = new Internal;
	box.size = math::Vec(0, RACK_GRID_HEIGHT);
}

ModuleWidget::~ModuleWidget() {
	clearChildren();
	setModule(NULL);
	delete internal;
}

void ModuleWidget::draw(const DrawArgs& args) {
	nvgScissor(args.vg, RECT_ARGS(args.clipBox));

	if (module && module->bypassed()) {
		nvgGlobalAlpha(args.vg, 0.33);
	}

	Widget::draw(args);

	// Power meter
	if (module && settings::cpuMeter) {
		nvgBeginPath(args.vg);
		nvgRect(args.vg,
		        0, box.size.y - 35,
		        65, 35);
		nvgFillColor(args.vg, nvgRGBAf(0, 0, 0, 0.75));
		nvgFill(args.vg);

		float percent = module->cpuTime() * APP->engine->getSampleRate() * 100;
		float microseconds = module->cpuTime() * 1e6f;
		std::string cpuText = string::f("%.1f%%\n%.2f μs", percent, microseconds);
		bndLabel(args.vg, 2.0, box.size.y - 34.0, INFINITY, INFINITY, -1, cpuText.c_str());

		float p = math::clamp(module->cpuTime() / APP->engine->getSampleTime(), 0.f, 1.f);
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
	// Don't consume left button if `lockModules` is enabled.
	if (settings::lockModules)
		Widget::onButton(e);
	else
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
		if (e.keyName == "i" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			resetAction();
			e.consume(this);
		}
		if (e.keyName == "r" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			randomizeAction();
			e.consume(this);
		}
		if (e.keyName == "c" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			copyClipboard();
			e.consume(this);
		}
		if (e.keyName == "v" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			pasteClipboardAction();
			e.consume(this);
		}
		if (e.keyName == "d" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			cloneAction();
			e.consume(this);
		}
		if (e.keyName == "u" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			disconnectAction();
			e.consume(this);
		}
		if (e.keyName == "e" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			bypassAction();
			e.consume(this);
		}
	}

	if (e.action == RACK_HELD) {
		if ((e.key == GLFW_KEY_DELETE || e.key == GLFW_KEY_BACKSPACE) && (e.mods & RACK_MOD_MASK) == 0) {
			removeAction();
			e.consume(NULL);
		}
	}
}

void ModuleWidget::onDragStart(const event::DragStart& e) {
	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		internal->dragPos = APP->scene->rack->mousePos.minus(box.pos);
		APP->scene->rack->updateModuleOldPositions();
	}
}

void ModuleWidget::onDragEnd(const event::DragEnd& e) {
	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		history::ComplexAction* h = APP->scene->rack->getModuleDragAction();
		if (!h)
			return;
		APP->history->push(h);
	}
}

void ModuleWidget::onDragMove(const event::DragMove& e) {
	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		if (!settings::lockModules) {
			math::Vec pos = APP->scene->rack->mousePos.minus(internal->dragPos);
			if ((APP->window->getMods() & RACK_MOD_MASK) == RACK_MOD_CTRL)
				APP->scene->rack->setModulePosForce(this, pos);
			else
				APP->scene->rack->setModulePosNearest(this, pos);
		}
	}
}

void ModuleWidget::setModule(engine::Module* module) {
	if (this->module) {
		APP->engine->removeModule(this->module);
		delete this->module;
		this->module = NULL;
	}
	this->module = module;
}

void ModuleWidget::setPanel(widget::Widget* panel) {
	// Remove existing panel
	if (internal->panel) {
		removeChild(internal->panel);
		delete internal->panel;
		internal->panel = NULL;
	}

	if (panel) {
		addChildBottom(panel);
		internal->panel = panel;
		box.size.x = std::round(panel->box.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
	}
}

void ModuleWidget::setPanel(std::shared_ptr<Svg> svg) {
	// Create SvgPanel
	SvgPanel* panel = new SvgPanel;
	panel->setBackground(svg);
	setPanel(panel);
}

void ModuleWidget::addParam(ParamWidget* param) {
	addChild(param);
}

void ModuleWidget::addInput(PortWidget* input) {
	// Check that the port is an input
	assert(input->type == engine::Port::INPUT);
	// Check that the port doesn't have a duplicate ID
	PortWidget* input2 = getInput(input->portId);
	assert(!input2);
	// Add port
	addChild(input);
}

void ModuleWidget::addOutput(PortWidget* output) {
	// Check that the port is an output
	assert(output->type == engine::Port::OUTPUT);
	// Check that the port doesn't have a duplicate ID
	PortWidget* output2 = getOutput(output->portId);
	assert(!output2);
	// Add port
	addChild(output);
}

template <class T, typename F>
T* getFirstDescendantOfTypeWithCondition(widget::Widget* w, F f) {
	T* t = dynamic_cast<T*>(w);
	if (t && f(t))
		return t;

	for (widget::Widget* child : w->children) {
		T* foundT = getFirstDescendantOfTypeWithCondition<T>(child, f);
		if (foundT)
			return foundT;
	}
	return NULL;
}

ParamWidget* ModuleWidget::getParam(int paramId) {
	return getFirstDescendantOfTypeWithCondition<ParamWidget>(this, [&](ParamWidget* pw) -> bool {
		return pw->paramId == paramId;
	});
}

PortWidget* ModuleWidget::getInput(int portId) {
	return getFirstDescendantOfTypeWithCondition<PortWidget>(this, [&](PortWidget* pw) -> bool {
		return pw->type == engine::Port::INPUT && pw->portId == portId;
	});
}

PortWidget* ModuleWidget::getOutput(int portId) {
	return getFirstDescendantOfTypeWithCondition<PortWidget>(this, [&](PortWidget* pw) -> bool {
		return pw->type == engine::Port::OUTPUT && pw->portId == portId;
	});
}

json_t* ModuleWidget::toJson() {
	json_t* moduleJ = APP->engine->moduleToJson(module);
	// When serializing ModuleWidget, don't include the ID. This ID is only meaningful when serializing the entire rack.
	json_object_del(moduleJ, "id");
	return moduleJ;
}

void ModuleWidget::fromJson(json_t* rootJ) {
	APP->engine->moduleFromJson(module, rootJ);
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

void ModuleWidget::load(std::string filename) {
	FILE* file = std::fopen(filename.c_str(), "r");
	if (!file)
		throw Exception(string::f("Could not load patch file %s", filename.c_str()));
	DEFER({
		std::fclose(file);
	});

	INFO("Loading preset %s", filename.c_str());

	json_error_t error;
	json_t* moduleJ = json_loadf(file, 0, &error);
	if (!moduleJ)
		throw Exception(string::f("File is not a valid patch file. JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text));
	DEFER({
		json_decref(moduleJ);
	});

	fromJson(moduleJ);
}

void ModuleWidget::loadAction(std::string filename) {
	// history::ModuleChange
	history::ModuleChange* h = new history::ModuleChange;
	h->name = "load module preset";
	h->moduleId = module->id;
	h->oldModuleJ = toJson();

	try {
		load(filename);
	}
	catch (Exception& e) {
		delete h;
		throw;
	}

	h->newModuleJ = toJson();
	APP->history->push(h);
}

void ModuleWidget::loadTemplate() {
	std::string templatePath = system::join(model->getUserPresetDir(), "template.vcvm");
	try {
		load(templatePath);
	}
	catch (Exception& e) {
		// Do nothing
	}
}

void ModuleWidget::loadDialog() {
	std::string presetDir = model->getUserPresetDir();
	system::createDirectories(presetDir);

	// Delete directories if empty
	DEFER({
		system::remove(presetDir);
		system::remove(system::getDirectory(presetDir));
	});

	osdialog_filters* filters = osdialog_filters_parse(PRESET_FILTERS);
	DEFER({osdialog_filters_free(filters);});

	char* pathC = osdialog_file(OSDIALOG_OPEN, presetDir.c_str(), NULL, filters);
	if (!pathC) {
		// No path selected
		return;
	}
	DEFER({free(pathC);});

	try {
		loadAction(pathC);
	}
	catch (Exception& e) {
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, e.what());
	}
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

void ModuleWidget::saveTemplate() {
	std::string presetDir = model->getUserPresetDir();
	system::createDirectories(presetDir);

	std::string templatePath = system::join(presetDir, "template.vcvm");
	save(templatePath);
}

void ModuleWidget::saveDialog() {
	std::string presetDir = model->getUserPresetDir();
	system::createDirectories(presetDir);

	// Delete directories if empty
	DEFER({
		// These fail silently if the directories are not empty
		system::remove(presetDir);
		system::remove(system::getDirectory(presetDir));
	});

	osdialog_filters* filters = osdialog_filters_parse(PRESET_FILTERS);
	DEFER({osdialog_filters_free(filters);});

	char* pathC = osdialog_file(OSDIALOG_SAVE, presetDir.c_str(), "Untitled.vcvm", filters);
	if (!pathC) {
		// No path selected
		return;
	}
	DEFER({free(pathC);});

	std::string path = pathC;
	if (system::getExtension(path) == "")
		path += ".vcvm";

	save(path);
}

template <class T, typename F>
void doOfType(widget::Widget* w, F f) {
	T* t = dynamic_cast<T*>(w);
	if (t)
		f(t);

	for (widget::Widget* child : w->children) {
		doOfType<T>(child, f);
	}
}

void ModuleWidget::disconnect() {
	doOfType<PortWidget>(this, [&](PortWidget* pw) {
		APP->scene->rack->clearCablesOnPort(pw);
	});
}

void ModuleWidget::resetAction() {
	assert(module);

	// history::ModuleChange
	history::ModuleChange* h = new history::ModuleChange;
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
	history::ModuleChange* h = new history::ModuleChange;
	h->name = "randomize module";
	h->moduleId = module->id;
	h->oldModuleJ = toJson();

	APP->engine->randomizeModule(module);

	h->newModuleJ = toJson();
	APP->history->push(h);
}

static void disconnectActions(ModuleWidget* mw, history::ComplexAction* complexAction) {
	// Add CableRemove action for all cables
	doOfType<PortWidget>(mw, [&](PortWidget* pw) {
		for (CableWidget* cw : APP->scene->rack->getCablesOnPort(pw)) {
			if (!cw->isComplete())
				continue;
			// Avoid creating duplicate actions for self-patched cables
			if (pw->type == engine::Port::INPUT && cw->outputPort->module == mw->module)
				continue;
			// history::CableRemove
			history::CableRemove* h = new history::CableRemove;
			h->setCable(cw);
			complexAction->push(h);
		}
	});
}

void ModuleWidget::disconnectAction() {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "disconnect cables";
	disconnectActions(this, complexAction);
	APP->history->push(complexAction);

	disconnect();
}

void ModuleWidget::cloneAction() {
	// history::ComplexAction
	history::ComplexAction* h = new history::ComplexAction;

	// Clone Module
	engine::Module* clonedModule = model->createModule();
	// JSON serialization is the obvious way to do this
	json_t* moduleJ = toJson();
	// This doesn't need a lock (via Engine::moduleFromJson()) because the Module is not added to the Engine yet.
	clonedModule->fromJson(moduleJ);
	json_decref(moduleJ);
	// Reset ID so the Engine automatically assigns a new one
	clonedModule->id = -1;
	APP->engine->addModule(clonedModule);

	// Clone ModuleWidget
	ModuleWidget* clonedModuleWidget = model->createModuleWidget(clonedModule);
	APP->scene->rack->addModuleAtMouse(clonedModuleWidget);

	// history::ModuleAdd
	history::ModuleAdd* hma = new history::ModuleAdd;
	hma->name = "clone modules";
	hma->setModule(clonedModuleWidget);
	h->push(hma);

	// Clone cables attached to input ports
	doOfType<PortWidget>(this, [&](PortWidget* pw) {
		if (pw->type != engine::Port::INPUT)
			return;
		std::list<CableWidget*> cables = APP->scene->rack->getCablesOnPort(pw);
		for (CableWidget* cw : cables) {
			// Create cable attached to cloned ModuleWidget's input
			engine::Cable* clonedCable = new engine::Cable;
			clonedCable->id = -1;
			clonedCable->inputModule = clonedModule;
			clonedCable->inputId = cw->cable->inputId;
			// If cable is self-patched, attach to cloned module instead
			if (cw->cable->outputModule == module)
				clonedCable->outputModule = clonedModule;
			else
				clonedCable->outputModule = cw->cable->outputModule;
			clonedCable->outputId = cw->cable->outputId;
			APP->engine->addCable(clonedCable);

			app::CableWidget* clonedCw = new app::CableWidget;
			clonedCw->setCable(clonedCable);
			clonedCw->color = cw->color;
			APP->scene->rack->addCable(clonedCw);

			// history::CableAdd
			history::CableAdd* hca = new history::CableAdd;
			hca->setCable(clonedCw);
			h->push(hca);
		}
	});

	APP->history->push(h);
}

void ModuleWidget::bypassAction() {
	assert(module);
	APP->engine->bypassModule(module, !module->bypassed());

	// history::ModuleBypass
	history::ModuleBypass* h = new history::ModuleBypass;
	h->moduleId = module->id;
	h->bypassed = module->bypassed();
	APP->history->push(h);
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

	ModuleInfoItem* infoItem = new ModuleInfoItem;
	infoItem->text = "Info";
	infoItem->rightText = RIGHT_ARROW;
	infoItem->model = model;
	menu->addChild(infoItem);

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
	bypassItem->text = "Bypass";
	bypassItem->rightText = RACK_MOD_CTRL_NAME "+E";
	if (module && module->bypassed())
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


math::Vec& ModuleWidget::dragPos() {
	return internal->dragPos;
}

math::Vec& ModuleWidget::oldPos() {
	return internal->oldPos;
}


engine::Module* ModuleWidget::releaseModule() {
	engine::Module* module = this->module;
	this->module = NULL;
	return module;
}


} // namespace app
} // namespace rack