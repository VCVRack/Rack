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


struct ModuleWidget::Internal {
	/** The module position clicked on to start dragging in the rack.
	*/
	math::Vec dragOffset;

	/** Global rack position the user clicked on.
	*/
	math::Vec dragRackPos;
	bool dragEnabled = true;

	/** The position in the RackWidget when dragging began.
	Used for history::ModuleMove.
	Set by RackWidget::updateModuleOldPositions() when *any* module begins dragging, since force-dragging can move other modules around.
	*/
	math::Vec oldPos;

	widget::Widget* panel = NULL;

	bool selected = false;
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

plugin::Model* ModuleWidget::getModel() {
	return model;
}

void ModuleWidget::setModel(plugin::Model* model) {
	assert(!this->model);
	this->model = model;
}

engine::Module* ModuleWidget::getModule() {
	return module;
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

template <class T, typename F>
void doIfTypeRecursive(widget::Widget* w, F f) {
	T* t = dynamic_cast<T*>(w);
	if (t)
		f(t);

	for (widget::Widget* child : w->children) {
		doIfTypeRecursive<T>(child, f);
	}
}

std::list<ParamWidget*> ModuleWidget::getParams() {
	std::list<ParamWidget*> pws;
	doIfTypeRecursive<ParamWidget>(this, [&](ParamWidget* pw) {
		pws.push_back(pw);
	});
	return pws;
}

std::list<PortWidget*> ModuleWidget::getPorts() {
	std::list<PortWidget*> pws;
	doIfTypeRecursive<PortWidget>(this, [&](PortWidget* pw) {
		pws.push_back(pw);
	});
	return pws;
}

std::list<PortWidget*> ModuleWidget::getInputs() {
	std::list<PortWidget*> pws;
	doIfTypeRecursive<PortWidget>(this, [&](PortWidget* pw) {
		if (pw->type == engine::Port::INPUT)
			pws.push_back(pw);
	});
	return pws;
}

std::list<PortWidget*> ModuleWidget::getOutputs() {
	std::list<PortWidget*> pws;
	doIfTypeRecursive<PortWidget>(this, [&](PortWidget* pw) {
		if (pw->type == engine::Port::OUTPUT)
			pws.push_back(pw);
	});
	return pws;
}

void ModuleWidget::draw(const DrawArgs& args) {
	nvgScissor(args.vg, RECT_ARGS(args.clipBox));

	if (module && module->isBypassed()) {
		nvgAlpha(args.vg, 0.33);
	}

	Widget::draw(args);

	// Meter
	if (module && settings::cpuMeter) {
		float sampleRate = APP->engine->getSampleRate();
		const float* meterBuffer = module->meterBuffer();
		int meterLength = module->meterLength();
		int meterIndex = module->meterIndex();

		float meterMax = 0.f;
		float meterAvg = 0.f;
		for (int i = 0; i < meterLength; i++) {
			float m = meterBuffer[i];
			meterAvg += m;
			meterMax = std::max(meterMax, m);
		}
		meterAvg /= meterLength;
		float percentMax = meterMax * sampleRate;
		float mult = (percentMax <= 0.1f) ? 10.f : 1.f;

		// // Text background
		// nvgBeginPath(args.vg);
		// nvgRect(args.vg, 0.0, box.size.y - infoHeight, box.size.x, infoHeight);
		// nvgFillColor(args.vg, nvgRGBAf(0, 0, 0, 0.75));
		// nvgFill(args.vg);

		// Draw time plot
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, box.size.x, box.size.y);
		for (int i = 0; i < meterLength; i++) {
			int index = (meterIndex - i + meterLength) % meterLength;
			float percent = math::clamp(meterBuffer[index] * mult * sampleRate, 0.f, 1.f);
			math::Vec p;
			p.x = (1.f - (float) i / (meterLength - 1)) * box.size.x;
			p.y = (1.f - percent) * (box.size.y);
			nvgLineTo(args.vg, p.x, p.y);
		}
		NVGcolor color;
		if (mult == 1.f)
			color = nvgRGBAf(0.5, 0, 0, 0.85);
		else if (mult == 10.f)
			color = nvgRGBAf(0.85, 0, 0, 0.85);
		nvgLineTo(args.vg, 0.0, box.size.y);
		nvgClosePath(args.vg);
		nvgFillColor(args.vg, color);
		nvgFill(args.vg);

		// Text
		float percent = meterAvg * sampleRate * 100.f;
		float microseconds = meterAvg * 1e6f;
		std::string meterText = string::f("%.0fx\n%.2f μs\n%.1f%%", mult, microseconds, percent);
		bndLabel(args.vg, 0.0, box.size.y - 60, INFINITY, INFINITY, -1, meterText.c_str());

		// Draw border
		nvgStrokeColor(args.vg, color);
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgStrokeWidth(args.vg, 2.0);
		nvgStroke(args.vg);
	}

	// if (module) {
	// 	nvgBeginPath(args.vg);
	// 	nvgRect(args.vg, 0, 0, 20, 20);
	// 	nvgFillColor(args.vg, nvgRGBAf(0, 0, 0, 0.75));
	// 	nvgFill(args.vg);

	// 	std::string debugText = string::f("%d", module->id);
	// 	bndLabel(args.vg, 0, 0, INFINITY, INFINITY, -1, debugText.c_str());
	// }

	// Selection
	if (internal->selected) {
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0.0, 0.0, VEC_ARGS(box.size));
		nvgFillColor(args.vg, nvgRGBAf(1, 0, 0, 0.25));
		nvgFill(args.vg);
		nvgStrokeWidth(args.vg, 2.0);
		nvgStrokeColor(args.vg, nvgRGBAf(1, 0, 0, 0.5));
		nvgStroke(args.vg);
	}

	nvgResetScissor(args.vg);
}

void ModuleWidget::drawShadow(const DrawArgs& args) {
	nvgBeginPath(args.vg);
	float r = 20; // Blur radius
	float c = 20; // Corner radius
	math::Rect shadowBox = box.zeroPos().grow(math::Vec(10, -30));
	math::Rect shadowOutsideBox = shadowBox.grow(math::Vec(r, r));
	nvgRect(args.vg, RECT_ARGS(shadowOutsideBox));
	NVGcolor shadowColor = nvgRGBAf(0, 0, 0, 0.2);
	NVGcolor transparentColor = nvgRGBAf(0, 0, 0, 0);
	nvgFillPaint(args.vg, nvgBoxGradient(args.vg, RECT_ARGS(shadowBox), c, r, shadowColor, transparentColor));
	nvgFill(args.vg);
}

void ModuleWidget::onHoverKey(const HoverKeyEvent& e) {
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

void ModuleWidget::onButton(const ButtonEvent& e) {
	OpaqueWidget::onButton(e);

	if (e.getTarget() == this) {
		// Set starting drag position
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			internal->dragOffset = e.pos;
		}
		// Toggle selection on Shift-click
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			internal->selected ^= true;
		}
	}

	if (!e.isConsumed()) {
		// Open context menu on right-click
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (internal->selected) {
				createSelectionContextMenu();
			}
			else {
				createContextMenu();
			}
			e.consume(this);
		}
	}
}

void ModuleWidget::onDragStart(const DragStartEvent& e) {
	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		// HACK Disable FramebufferWidget redrawing subpixels while dragging
		APP->window->fbDirtyOnSubpixelChange() = false;

		// Clear dragRack so dragging in not enabled until mouse is moved a bit.
		internal->dragRackPos = math::Vec(NAN, NAN);

		// Prepare initial position of modules for history.
		APP->scene->rack->updateModuleOldPositions();
	}
}

void ModuleWidget::onDragEnd(const DragEndEvent& e) {
	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		APP->window->fbDirtyOnSubpixelChange() = true;

		// The next time the module is dragged, it should always move immediately
		internal->dragEnabled = true;

		history::ComplexAction* h = APP->scene->rack->getModuleDragAction();
		if (!h->isEmpty())
			APP->history->push(h);
		else
			delete h;
	}
}

void ModuleWidget::onDragMove(const DragMoveEvent& e) {
	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		if (!settings::lockModules) {
			math::Vec mousePos = APP->scene->rack->getMousePos();

			if (!internal->dragEnabled) {
				// Set dragRackPos on the first time after dragging
				if (!internal->dragRackPos.isFinite())
					internal->dragRackPos = mousePos;
				// Check if the mouse has moved enough to start dragging the module.
				const float minDist = RACK_GRID_WIDTH;
				if (internal->dragRackPos.minus(mousePos).square() >= std::pow(minDist, 2))
					internal->dragEnabled = true;
			}

			// Move module
			if (internal->dragEnabled) {
				// Round y coordinate to nearest rack height
				math::Vec pos = mousePos;
				pos.x -= internal->dragOffset.x;
				pos.y -= RACK_GRID_HEIGHT / 2;
				if (internal->selected) {
					pos = (pos / RACK_GRID_SIZE).round() * RACK_GRID_SIZE;
					math::Vec delta = pos.minus(box.pos);
					APP->scene->rack->requestSelectedModulePos(delta);
				}
				else {
					if ((APP->window->getMods() & RACK_MOD_MASK) == RACK_MOD_CTRL)
						APP->scene->rack->setModulePosForce(this, pos);
					else
						APP->scene->rack->setModulePosNearest(this, pos);
				}
			}
		}
	}
}

json_t* ModuleWidget::toJson() {
	json_t* moduleJ = APP->engine->moduleToJson(module);
	return moduleJ;
}

void ModuleWidget::fromJson(json_t* rootJ) {
	APP->engine->moduleFromJson(module, rootJ);
}

void ModuleWidget::copyClipboard() {
	json_t* moduleJ = toJson();
	DEFER({json_decref(moduleJ);});
	char* moduleJson = json_dumps(moduleJ, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
	DEFER({std::free(moduleJson);});
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
	DEFER({json_decref(moduleJ);});

	// Don't use IDs from JSON
	json_object_del(moduleJ, "id");
	json_object_del(moduleJ, "leftModuleId");
	json_object_del(moduleJ, "rightModuleId");

	json_t* oldModuleJ = toJson();

	try {
		fromJson(moduleJ);
	}
	catch (Exception& e) {
		WARN("%s", e.what());
		json_decref(oldModuleJ);
		return;
	}

	// history::ModuleChange
	history::ModuleChange* h = new history::ModuleChange;
	h->name = "paste module preset";
	h->moduleId = module->id;
	h->oldModuleJ = oldModuleJ;
	h->newModuleJ = toJson();
	APP->history->push(h);
}

void ModuleWidget::load(std::string filename) {
	FILE* file = std::fopen(filename.c_str(), "r");
	if (!file)
		throw Exception("Could not load patch file %s", filename.c_str());
	DEFER({std::fclose(file);});

	INFO("Loading preset %s", filename.c_str());

	json_error_t error;
	json_t* moduleJ = json_loadf(file, 0, &error);
	if (!moduleJ)
		throw Exception("File is not a valid patch file. JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
	DEFER({json_decref(moduleJ);});

	// Don't use IDs from JSON
	json_object_del(moduleJ, "id");
	json_object_del(moduleJ, "leftModuleId");
	json_object_del(moduleJ, "rightModuleId");

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
	std::string templatePath = system::join(model->getUserPresetDirectory(), "template.vcvm");
	try {
		load(templatePath);
	}
	catch (Exception& e) {
		// Do nothing
	}
}

void ModuleWidget::loadDialog() {
	std::string presetDir = model->getUserPresetDirectory();
	system::createDirectories(presetDir);

	// Delete directories if empty
	DEFER({
		try {
			system::remove(presetDir);
			system::remove(system::getDirectory(presetDir));
		}
		catch (Exception& e) {
			// Ignore exceptions if directory cannot be removed.
		}
	});

	osdialog_filters* filters = osdialog_filters_parse(PRESET_FILTERS);
	DEFER({osdialog_filters_free(filters);});

	char* pathC = osdialog_file(OSDIALOG_OPEN, presetDir.c_str(), NULL, filters);
	if (!pathC) {
		// No path selected
		return;
	}
	DEFER({std::free(pathC);});

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
	DEFER({json_decref(moduleJ);});

	FILE* file = std::fopen(filename.c_str(), "w");
	if (!file) {
		std::string message = string::f("Could not save preset to file %s", filename.c_str());
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
		return;
	}
	DEFER({std::fclose(file);});

	json_dumpf(moduleJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
}

void ModuleWidget::saveTemplate() {
	std::string presetDir = model->getUserPresetDirectory();
	system::createDirectories(presetDir);
	std::string templatePath = system::join(presetDir, "template.vcvm");
	save(templatePath);
}

void ModuleWidget::saveTemplateDialog() {
	if (hasTemplate()) {
		std::string message = string::f("Overwrite template preset for %s?", model->getFullName().c_str());
		if (!osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, message.c_str()))
			return;
	}
	saveTemplate();
}

bool ModuleWidget::hasTemplate() {
	std::string presetDir = model->getUserPresetDirectory();
	std::string templatePath = system::join(presetDir, "template.vcvm");
	return system::exists(templatePath);;
}

void ModuleWidget::clearTemplate() {
	std::string presetDir = model->getUserPresetDirectory();
	std::string templatePath = system::join(presetDir, "template.vcvm");
	system::remove(templatePath);
}

void ModuleWidget::clearTemplateDialog() {
	std::string message = string::f("Delete template preset for %s?", model->getFullName().c_str());
	if (!osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, message.c_str()))
		return;
	clearTemplate();
}

void ModuleWidget::saveDialog() {
	std::string presetDir = model->getUserPresetDirectory();
	system::createDirectories(presetDir);

	// Delete directories if empty
	DEFER({
		try {
			system::remove(presetDir);
			system::remove(system::getDirectory(presetDir));
		}
		catch (Exception& e) {
			// Ignore exceptions if directory cannot be removed.
		}
	});

	osdialog_filters* filters = osdialog_filters_parse(PRESET_FILTERS);
	DEFER({osdialog_filters_free(filters);});

	char* pathC = osdialog_file(OSDIALOG_SAVE, presetDir.c_str(), "Untitled.vcvm", filters);
	if (!pathC) {
		// No path selected
		return;
	}
	DEFER({std::free(pathC);});

	std::string path = pathC;
	// Automatically append .vcvm extension
	if (system::getExtension(path) != ".vcvm")
		path += ".vcvm";

	save(path);
}

void ModuleWidget::disconnect() {
	for (PortWidget* pw : getPorts()) {
		APP->scene->rack->clearCablesOnPort(pw);
	}
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

void ModuleWidget::appendDisconnectActions(history::ComplexAction* complexAction) {
	for (PortWidget* pw : getPorts()) {
		for (CableWidget* cw : APP->scene->rack->getCablesOnPort(pw)) {
			if (!cw->isComplete())
				continue;
			// history::CableRemove
			history::CableRemove* h = new history::CableRemove;
			h->setCable(cw);
			complexAction->push(h);
			// Delete cable
			APP->scene->rack->removeCable(cw);
			delete cw;
		}
	};
}

void ModuleWidget::disconnectAction() {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "disconnect cables";
	appendDisconnectActions(complexAction);

	if (!complexAction->isEmpty())
		APP->history->push(complexAction);
	else
		delete complexAction;
}

void ModuleWidget::cloneAction() {
	// history::ComplexAction
	history::ComplexAction* h = new history::ComplexAction;

	// JSON serialization is the obvious way to do this
	json_t* moduleJ = toJson();
	// Don't use IDs from JSON
	json_object_del(moduleJ, "id");
	json_object_del(moduleJ, "leftModuleId");
	json_object_del(moduleJ, "rightModuleId");

	// Clone Module
	engine::Module* clonedModule = model->createModule();
	// This doesn't need a lock (via Engine::moduleFromJson()) because the Module is not added to the Engine yet.
	try {
		clonedModule->fromJson(moduleJ);
	}
	catch (Exception& e) {
		WARN("%s", e.what());
	}
	json_decref(moduleJ);
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
	for (PortWidget* pw : getInputs()) {
		for (CableWidget* cw : APP->scene->rack->getCablesOnPort(pw)) {
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
	}

	APP->history->push(h);
}

void ModuleWidget::bypassAction() {
	assert(module);
	bool bypassed = !module->isBypassed();

	// history::ModuleBypass
	history::ModuleBypass* h = new history::ModuleBypass;
	h->moduleId = module->id;
	h->bypassed = bypassed;
	APP->history->push(h);

	APP->engine->bypassModule(module, bypassed);
}

void ModuleWidget::removeAction() {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "remove module";
	appendDisconnectActions(complexAction);

	// history::ModuleRemove
	history::ModuleRemove* moduleRemove = new history::ModuleRemove;
	moduleRemove->setModule(this);
	complexAction->push(moduleRemove);

	APP->history->push(complexAction);

	// This removes the module and transfers ownership to caller
	APP->scene->rack->removeModule(this);
	delete this;
}


// Create ModulePresetPathItems for each patch in a directory.
static void appendPresetItems(ui::Menu* menu, WeakPtr<ModuleWidget> moduleWidget, std::string presetDir) {
	bool hasPresets = false;
	// Note: This is not cached, so opening this menu each time might have a bit of latency.
	if (system::isDirectory(presetDir)) {
		for (std::string path : system::getEntries(presetDir)) {
			std::string name = system::getStem(path);
			// Remove "1_", "42_", "001_", etc at the beginning of preset filenames
			std::regex r("^\\d*_");
			name = std::regex_replace(name, r, "");

			if (system::isDirectory(path)) {
				hasPresets = true;

				menu->addChild(createSubmenuItem(name, [=](ui::Menu* menu) {
					if (!moduleWidget)
						return;
					appendPresetItems(menu, moduleWidget, path);
				}));
			}
			else if (system::getExtension(path) == ".vcvm") {
				hasPresets = true;

				menu->addChild(createMenuItem(name, "", [=]() {
					if (!moduleWidget)
						return;
					try {
						moduleWidget->loadAction(path);
					}
					catch (Exception& e) {
						osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, e.what());
					}
				}));
			}
		}
	}
	if (!hasPresets) {
		menu->addChild(createMenuLabel("(None)"));
	}
};


void ModuleWidget::createContextMenu() {
	ui::Menu* menu = createMenu();
	assert(model);

	WeakPtr<ModuleWidget> weakThis = this;

	// Brand and module name
	menu->addChild(createMenuLabel(model->plugin->brand + " " + model->name));

	// Info
	menu->addChild(createSubmenuItem("Info", [=](ui::Menu* menu) {
		if (!weakThis)
			return;

		// plugin
		menu->addChild(createMenuItem("Plugin: " + model->plugin->name, "", [=]() {
			system::openBrowser(model->plugin->pluginUrl);
		}, model->plugin->pluginUrl == ""));

		// version
		menu->addChild(createMenuLabel(model->plugin->version));

		// author
		if (model->plugin->author != "") {
			menu->addChild(createMenuItem("Author: " + model->plugin->author, "", [=]() {
				system::openBrowser(model->plugin->authorUrl);
			}, model->plugin->authorUrl.empty()));
		}

		// license
		std::string license = model->plugin->license;
		if (string::startsWith(license, "https://") || string::startsWith(license, "http://")) {
			menu->addChild(createMenuItem("License: Open in browser", "", [=]() {
				system::openBrowser(license);
			}));
		}
		else if (license != "") {
			menu->addChild(createMenuLabel("License: " + license));
		}

		// tags
		if (!model->tagIds.empty()) {
			menu->addChild(createMenuLabel("Tags:"));
			for (int tagId : model->tagIds) {
				menu->addChild(createMenuLabel("• " + tag::getTag(tagId)));
			}
		}

		menu->addChild(new ui::MenuSeparator);

		// VCV Library page
		menu->addChild(createMenuItem("VCV Library page", "", [=]() {
			system::openBrowser("https://library.vcvrack.com/" + model->plugin->slug + "/" + model->slug);
		}));

		// modularGridUrl
		if (model->modularGridUrl != "") {
			menu->addChild(createMenuItem("ModularGrid page", "", [=]() {
				system::openBrowser(model->modularGridUrl);
			}));
		}

		// manual
		std::string manualUrl = (model->manualUrl != "") ? model->manualUrl : model->plugin->manualUrl;
		if (manualUrl != "") {
			menu->addChild(createMenuItem("User manual", "", [=]() {
				system::openBrowser(manualUrl);
			}));
		}

		// donate
		if (model->plugin->donateUrl != "") {
			menu->addChild(createMenuItem("Donate", "", [=]() {
				system::openBrowser(model->plugin->donateUrl);
			}));
		}

		// source code
		if (model->plugin->sourceUrl != "") {
			menu->addChild(createMenuItem("Source code", "", [=]() {
				system::openBrowser(model->plugin->sourceUrl);
			}));
		}

		// changelog
		if (model->plugin->changelogUrl != "") {
			menu->addChild(createMenuItem("Changelog", "", [=]() {
				system::openBrowser(model->plugin->changelogUrl);
			}));
		}

		// plugin folder
		if (model->plugin->path != "") {
			menu->addChild(createMenuItem("Open plugin folder", "", [=]() {
				system::openDirectory(model->plugin->path);
			}));
		}
	}));

	// Preset
	menu->addChild(createSubmenuItem("Preset", [=](ui::Menu* menu) {
		menu->addChild(createMenuItem("Copy", RACK_MOD_CTRL_NAME "+C", [=]() {
			if (!weakThis)
				return;
			weakThis->copyClipboard();
		}));

		menu->addChild(createMenuItem("Paste", RACK_MOD_CTRL_NAME "+V", [=]() {
			if (!weakThis)
				return;
			weakThis->pasteClipboardAction();
		}));

		menu->addChild(createMenuItem("Open", "", [=]() {
			if (!weakThis)
				return;
			weakThis->loadDialog();
		}));

		menu->addChild(createMenuItem("Save as", "", [=]() {
			if (!weakThis)
				return;
			weakThis->saveDialog();
		}));

		menu->addChild(createMenuItem("Save template", "", [=]() {
			if (!weakThis)
				return;
			weakThis->saveTemplateDialog();
		}));

		menu->addChild(createMenuItem("Clear template", "", [=]() {
			if (!weakThis)
				return;
			weakThis->clearTemplateDialog();
		}, !weakThis->hasTemplate()));

		// Scan `<user dir>/presets/<plugin slug>/<module slug>` for presets.
		menu->addChild(new ui::MenuSeparator);
		menu->addChild(createMenuLabel("User presets"));
		appendPresetItems(menu, weakThis, weakThis->model->getUserPresetDirectory());

		// Scan `<plugin dir>/presets/<module slug>` for presets.
		menu->addChild(new ui::MenuSeparator);
		menu->addChild(createMenuLabel("Factory presets"));
		appendPresetItems(menu, weakThis, weakThis->model->getFactoryPresetDirectory());
	}));

	// Initialize
	menu->addChild(createMenuItem("Initialize", RACK_MOD_CTRL_NAME "+I", [=]() {
		if (!weakThis)
			return;
		weakThis->resetAction();
	}));

	// Randomize
	menu->addChild(createMenuItem("Randomize", RACK_MOD_CTRL_NAME "+R", [=]() {
		if (!weakThis)
			return;
		weakThis->randomizeAction();
	}));

	menu->addChild(createMenuItem("Disconnect cables", RACK_MOD_CTRL_NAME "+U", [=]() {
		if (!weakThis)
			return;
		weakThis->disconnectAction();
	}));

	menu->addChild(createMenuItem("Duplicate", RACK_MOD_CTRL_NAME "+D", [=]() {
		if (!weakThis)
			return;
		weakThis->cloneAction();
	}));

	std::string bypassText = RACK_MOD_CTRL_NAME "+E";
	if (module && module->isBypassed())
		bypassText += " " CHECKMARK_STRING;
	menu->addChild(createMenuItem("Bypass", bypassText, [=]() {
		if (!weakThis)
			return;
		weakThis->bypassAction();
	}));

	menu->addChild(createMenuItem("Delete", "Backspace/Delete", [=]() {
		if (!weakThis)
			return;
		weakThis->removeAction();
	}));

	appendContextMenu(menu);
}

void ModuleWidget::createSelectionContextMenu() {
	ui::Menu* menu = createMenu();

	int n = APP->scene->rack->getNumSelectedModules();
	menu->addChild(createMenuLabel(string::f("%d selected %s", n, n == 1 ? "module" : "modules")));

	// Initialize
	menu->addChild(createMenuItem("Initialize", "", [=]() {
		APP->scene->rack->resetSelectedModulesAction();
	}));

	// Randomize
	menu->addChild(createMenuItem("Randomize", "", [=]() {
		APP->scene->rack->randomizeSelectedModulesAction();
	}));

	// Disconnect cables
	menu->addChild(createMenuItem("Disconnect cables", "", [=]() {
		APP->scene->rack->disconnectSelectedModulesAction();
	}));

	// Bypass
	menu->addChild(createBoolMenuItem("Bypass",
		[=]() {
			return APP->scene->rack->areSelectedModulesBypassed();
		},
		[=](bool bypassed) {
			APP->scene->rack->bypassSelectedModulesAction(bypassed);
		}
	));

	// Delete
	menu->addChild(createMenuItem("Delete", "", [=]() {
		APP->scene->rack->deleteSelectedModulesAction();
	}));
}

math::Vec& ModuleWidget::dragOffset() {
	return internal->dragOffset;
}

bool& ModuleWidget::dragEnabled() {
	return internal->dragEnabled;
}

math::Vec& ModuleWidget::oldPos() {
	return internal->oldPos;
}

engine::Module* ModuleWidget::releaseModule() {
	engine::Module* module = this->module;
	this->module = NULL;
	return module;
}

bool& ModuleWidget::selected() {
	return internal->selected;
}



} // namespace app
} // namespace rack