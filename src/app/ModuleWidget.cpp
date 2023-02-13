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
#include <componentlibrary.hpp>


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

widget::Widget* ModuleWidget::getPanel() {
	return internal->panel;
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
		// If width is zero, set it to 12HP for sanity
		if (box.size.x == 0.0)
			box.size.x = 12 * RACK_GRID_WIDTH;
	}
}

void ModuleWidget::setPanel(std::shared_ptr<window::Svg> svg) {
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

std::vector<ParamWidget*> ModuleWidget::getParams() {
	std::vector<ParamWidget*> pws;
	doIfTypeRecursive<ParamWidget>(this, [&](ParamWidget* pw) {
		pws.push_back(pw);
	});
	return pws;
}

std::vector<PortWidget*> ModuleWidget::getPorts() {
	std::vector<PortWidget*> pws;
	doIfTypeRecursive<PortWidget>(this, [&](PortWidget* pw) {
		pws.push_back(pw);
	});
	return pws;
}

std::vector<PortWidget*> ModuleWidget::getInputs() {
	std::vector<PortWidget*> pws;
	doIfTypeRecursive<PortWidget>(this, [&](PortWidget* pw) {
		if (pw->type == engine::Port::INPUT)
			pws.push_back(pw);
	});
	return pws;
}

std::vector<PortWidget*> ModuleWidget::getOutputs() {
	std::vector<PortWidget*> pws;
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

		// // Text background
		// nvgBeginPath(args.vg);
		// nvgRect(args.vg, 0.0, box.size.y - infoHeight, box.size.x, infoHeight);
		// nvgFillColor(args.vg, nvgRGBAf(0, 0, 0, 0.75));
		// nvgFill(args.vg);

		// Draw time plot
		const float plotHeight = box.size.y - BND_WIDGET_HEIGHT;
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, 0.0, plotHeight);
		math::Vec p1;
		for (int i = 0; i < meterLength; i++) {
			int index = math::eucMod(meterIndex + i + 1, meterLength);
			float meter = math::clamp(meterBuffer[index] * sampleRate, 0.f, 1.f);
			meter = std::max(0.f, meter);
			math::Vec p;
			p.x = (float) i / (meterLength - 1) * box.size.x;
			p.y = (1.f - meter) * plotHeight;
			if (i == 0) {
				nvgLineTo(args.vg, VEC_ARGS(p));
			}
			else {
				math::Vec p2 = p;
				p2.x -= 0.5f / (meterLength - 1) * box.size.x;
				nvgBezierTo(args.vg, VEC_ARGS(p1), VEC_ARGS(p2), VEC_ARGS(p));
			}
			p1 = p;
			p1.x += 0.5f / (meterLength - 1) * box.size.x;
		}
		nvgLineTo(args.vg, box.size.x, plotHeight);
		nvgClosePath(args.vg);
		NVGcolor color = componentlibrary::SCHEME_ORANGE;
		nvgFillColor(args.vg, color::alpha(color, 0.75));
		nvgFill(args.vg);
		nvgStrokeWidth(args.vg, 2.0);
		nvgStrokeColor(args.vg, color);
		nvgStroke(args.vg);

		// Text background
		bndMenuBackground(args.vg, 0.0, plotHeight, box.size.x, BND_WIDGET_HEIGHT, BND_CORNER_ALL);

		// Text
		float percent = meterBuffer[meterIndex] * sampleRate * 100.f;
		// float microseconds = meterBuffer[meterIndex] * 1e6f;
		std::string meterText = string::f("%.1f", percent);
		// Only append "%" if wider than 2 HP
		if (box.getWidth() > RACK_GRID_WIDTH * 2)
			meterText += "%";
		math::Vec pt;
		pt.x = box.size.x - bndLabelWidth(args.vg, -1, meterText.c_str()) + 3;
		pt.y = plotHeight + 0.5;
		bndMenuLabel(args.vg, VEC_ARGS(pt), INFINITY, BND_WIDGET_HEIGHT, -1, meterText.c_str());
	}

	// Selection
	if (APP->scene->rack->isSelected(this)) {
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

void ModuleWidget::drawLayer(const DrawArgs& args, int layer) {
	if (layer == -1) {
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
	else {
		Widget::drawLayer(args, layer);
	}
}

void ModuleWidget::onHover(const HoverEvent& e) {
	if (APP->scene->rack->isSelected(this)) {
		e.consume(this);
	}

	OpaqueWidget::onHover(e);
}

void ModuleWidget::onHoverKey(const HoverKeyEvent& e) {
	if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		if (e.keyName == "c" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			copyClipboard();
			e.consume(this);
		}
		if (e.keyName == "v" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			if (pasteClipboardAction()) {
				e.consume(this);
			}
		}
		if (e.keyName == "d" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			cloneAction(false);
			e.consume(this);
		}
		if (e.keyName == "d" && (e.mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT)) {
			cloneAction(true);
			e.consume(this);
		}
		if (e.keyName == "i" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			resetAction();
			e.consume(this);
		}
		if (e.keyName == "r" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			randomizeAction();
			e.consume(this);
		}
		if (e.keyName == "u" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			disconnectAction();
			e.consume(this);
		}
		if (e.keyName == "e" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			bypassAction(!module->isBypassed());
			e.consume(this);
		}
		if ((e.key == GLFW_KEY_DELETE || e.key == GLFW_KEY_BACKSPACE) && (e.mods & RACK_MOD_MASK) == 0) {
			// Deletes `this`
			removeAction();
			e.consume(NULL);
			return;
		}
		if (e.key == GLFW_KEY_F1 && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			std::string manualUrl = model->getManualUrl();
			if (!manualUrl.empty())
				system::openBrowser(manualUrl);
			e.consume(this);
		}
	}

	if (e.isConsumed())
		return;
	OpaqueWidget::onHoverKey(e);
}

void ModuleWidget::onButton(const ButtonEvent& e) {
	bool selected = APP->scene->rack->isSelected(this);

	if (selected) {
		if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (e.action == GLFW_PRESS) {
				// Open selection context menu on right-click
				ui::Menu* menu = createMenu();
				APP->scene->rack->appendSelectionContextMenu(menu);
			}
			e.consume(this);
		}

		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			if (e.action == GLFW_PRESS) {
				// Toggle selection on Shift-click
				if ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
					APP->scene->rack->select(this, false);
					e.consume(NULL);
					return;
				}

				// If module positions are locked, don't consume left-click
				if (settings::lockModules) {
					e.consume(NULL);
					return;
				}

				internal->dragOffset = e.pos;
			}

			e.consume(this);
		}

		return;
	}

	// Dispatch event to children
	Widget::onButton(e);
	e.stopPropagating();
	if (e.isConsumed())
		return;

	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		if (e.action == GLFW_PRESS) {
			// Toggle selection on Shift-click
			if ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
				APP->scene->rack->select(this, true);
				e.consume(NULL);
				return;
			}

			// If module positions are locked, don't consume left-click
			if (settings::lockModules) {
				e.consume(NULL);
				return;
			}

			internal->dragOffset = e.pos;
		}
		e.consume(this);
	}

	// Open context menu on right-click
	if (e.button == GLFW_MOUSE_BUTTON_RIGHT && e.action == GLFW_PRESS) {
		createContextMenu();
		e.consume(this);
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

			if (APP->scene->rack->isSelected(this)) {
				pos = (pos / RACK_GRID_SIZE).round() * RACK_GRID_SIZE;
				math::Vec delta = pos.minus(box.pos);
				APP->scene->rack->setSelectionPosNearest(delta);
			}
			else {
				if (settings::squeezeModules) {
					APP->scene->rack->setModulePosSqueeze(this, pos);
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

void ModuleWidget::onDragHover(const DragHoverEvent& e) {
	if (APP->scene->rack->isSelected(this)) {
		e.consume(this);
	}

	OpaqueWidget::onDragHover(e);
}

json_t* ModuleWidget::toJson() {
	json_t* moduleJ = APP->engine->moduleToJson(module);
	return moduleJ;
}

void ModuleWidget::fromJson(json_t* moduleJ) {
	APP->engine->moduleFromJson(module, moduleJ);
}

bool ModuleWidget::pasteJsonAction(json_t* moduleJ) {
	engine::Module::jsonStripIds(moduleJ);

	json_t* oldModuleJ = toJson();
	DEFER({json_decref(oldModuleJ);});

	try {
		fromJson(moduleJ);
	}
	catch (Exception& e) {
		WARN("%s", e.what());
		return false;
	}

	// history::ModuleChange
	history::ModuleChange* h = new history::ModuleChange;
	h->name = "paste module preset";
	h->moduleId = module->id;
	json_incref(oldModuleJ);
	h->oldModuleJ = oldModuleJ;
	json_incref(moduleJ);
	h->newModuleJ = moduleJ;
	APP->history->push(h);
	return true;
}

void ModuleWidget::copyClipboard() {
	json_t* moduleJ = toJson();
	engine::Module::jsonStripIds(moduleJ);

	DEFER({json_decref(moduleJ);});
	char* json = json_dumps(moduleJ, JSON_INDENT(2));
	DEFER({std::free(json);});
	glfwSetClipboardString(APP->window->win, json);
}

bool ModuleWidget::pasteClipboardAction() {
	const char* json = glfwGetClipboardString(APP->window->win);
	if (!json) {
		WARN("Could not get text from clipboard.");
		return false;
	}

	json_error_t error;
	json_t* moduleJ = json_loads(json, 0, &error);
	if (!moduleJ) {
		WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		return false;
	}
	DEFER({json_decref(moduleJ);});

	return pasteJsonAction(moduleJ);
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

	engine::Module::jsonStripIds(moduleJ);
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

	// TODO We can use `moduleJ` here instead to save a toJson() call.
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

	engine::Module::jsonStripIds(moduleJ);

	FILE* file = std::fopen(filename.c_str(), "w");
	if (!file) {
		std::string message = string::f("Could not save preset to file %s", filename.c_str());
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
		return;
	}
	DEFER({std::fclose(file);});

	json_dumpf(moduleJ, file, JSON_INDENT(2));
}

void ModuleWidget::saveTemplate() {
	std::string presetDir = model->getUserPresetDirectory();
	system::createDirectories(presetDir);
	std::string templatePath = system::join(presetDir, "template.vcvm");
	save(templatePath);
}

void ModuleWidget::saveTemplateDialog() {
	if (hasTemplate()) {
		std::string message = string::f("Overwrite default preset for %s?", model->getFullName().c_str());
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
	std::string message = string::f("Delete default preset for %s?", model->getFullName().c_str());
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
		for (CableWidget* cw : APP->scene->rack->getCompleteCablesOnPort(pw)) {
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

void ModuleWidget::cloneAction(bool cloneCables) {
	// history::ComplexAction
	history::ComplexAction* h = new history::ComplexAction;
	h->name = "duplicate module";

	// Save patch store in this module so we can copy it below
	APP->engine->prepareSaveModule(module);

	// JSON serialization is the obvious way to do this
	json_t* moduleJ = toJson();
	DEFER({
		json_decref(moduleJ);
	});
	engine::Module::jsonStripIds(moduleJ);

	// Clone Module
	INFO("Creating module %s", model->getFullName().c_str());
	engine::Module* clonedModule = model->createModule();

	// Set ID here so we can copy module storage dir
	clonedModule->id = random::u64() % (1ull << 53);
	system::copy(module->getPatchStorageDirectory(), clonedModule->getPatchStorageDirectory());

	// This doesn't need a lock (via Engine::moduleFromJson()) because the Module is not added to the Engine yet.
	try {
		clonedModule->fromJson(moduleJ);
	}
	catch (Exception& e) {
		WARN("%s", e.what());
	}
	APP->engine->addModule(clonedModule);

	// Clone ModuleWidget
	INFO("Creating module widget %s", model->getFullName().c_str());
	ModuleWidget* clonedModuleWidget = model->createModuleWidget(clonedModule);
	APP->scene->rack->updateModuleOldPositions();
	APP->scene->rack->addModule(clonedModuleWidget);
	// Place module to the right of `this` module, by forcing it to 1 HP to the right.
	math::Vec clonedPos = box.pos;
	clonedPos.x += clonedModuleWidget->box.getWidth();
	if (settings::squeezeModules)
		APP->scene->rack->squeezeModulePos(clonedModuleWidget, clonedPos);
	else
		APP->scene->rack->setModulePosNearest(clonedModuleWidget, clonedPos);
	h->push(APP->scene->rack->getModuleDragAction());
	APP->scene->rack->updateExpanders();

	// history::ModuleAdd
	history::ModuleAdd* hma = new history::ModuleAdd;
	hma->setModule(clonedModuleWidget);
	h->push(hma);

	if (cloneCables) {
		// Clone cables attached to input ports
		for (PortWidget* pw : getInputs()) {
			for (CableWidget* cw : APP->scene->rack->getCompleteCablesOnPort(pw)) {
				// Create cable attached to cloned ModuleWidget's input
				engine::Cable* clonedCable = new engine::Cable;
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
	}

	APP->history->push(h);
}

void ModuleWidget::bypassAction(bool bypassed) {
	assert(module);

	// history::ModuleBypass
	history::ModuleBypass* h = new history::ModuleBypass;
	h->moduleId = module->id;
	h->bypassed = bypassed;
	if (!bypassed)
		h->name = "un-bypass module";
	APP->history->push(h);

	APP->engine->bypassModule(module, bypassed);
}

void ModuleWidget::removeAction() {
	history::ComplexAction* h = new history::ComplexAction;
	h->name = "delete module";

	// Disconnect cables
	appendDisconnectActions(h);

	// Unset module position from rack.
	APP->scene->rack->updateModuleOldPositions();
	if (settings::squeezeModules)
		APP->scene->rack->unsqueezeModulePos(this);
	h->push(APP->scene->rack->getModuleDragAction());

	// history::ModuleRemove
	history::ModuleRemove* moduleRemove = new history::ModuleRemove;
	moduleRemove->setModule(this);
	h->push(moduleRemove);

	APP->history->push(h);

	// This removes the module and transfers ownership to caller
	APP->scene->rack->removeModule(this);
	delete this;

	APP->scene->rack->updateExpanders();
}


// Create ModulePresetPathItems for each patch in a directory.
static void appendPresetItems(ui::Menu* menu, WeakPtr<ModuleWidget> moduleWidget, std::string presetDir) {
	bool hasPresets = false;
	if (system::isDirectory(presetDir)) {
		// Note: This is not cached, so opening this menu each time might have a bit of latency.
		std::vector<std::string> entries = system::getEntries(presetDir);
		std::sort(entries.begin(), entries.end());
		for (std::string path : entries) {
			std::string name = system::getStem(path);
			// Remove "1_", "42_", "001_", etc at the beginning of preset filenames
			std::regex r("^\\d+_");
			name = std::regex_replace(name, r, "");

			if (system::isDirectory(path)) {
				hasPresets = true;

				menu->addChild(createSubmenuItem(name, "", [=](ui::Menu* menu) {
					if (!moduleWidget)
						return;
					appendPresetItems(menu, moduleWidget, path);
				}));
			}
			else if (system::getExtension(path) == ".vcvm" && name != "template") {
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
	menu->addChild(createMenuLabel(model->name));
	menu->addChild(createMenuLabel(model->plugin->brand));

	// Info
	menu->addChild(createSubmenuItem("Info", "", [=](ui::Menu* menu) {
		model->appendContextMenu(menu);
	}));

	// Preset
	menu->addChild(createSubmenuItem("Preset", "", [=](ui::Menu* menu) {
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

		menu->addChild(createMenuItem("Save default", "", [=]() {
			if (!weakThis)
				return;
			weakThis->saveTemplateDialog();
		}));

		menu->addChild(createMenuItem("Clear default", "", [=]() {
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

	// Disconnect cables
	menu->addChild(createMenuItem("Disconnect cables", RACK_MOD_CTRL_NAME "+U", [=]() {
		if (!weakThis)
			return;
		weakThis->disconnectAction();
	}));

	// Bypass
	std::string bypassText = RACK_MOD_CTRL_NAME "+E";
	bool bypassed = module && module->isBypassed();
	if (bypassed)
		bypassText += " " CHECKMARK_STRING;
	menu->addChild(createMenuItem("Bypass", bypassText, [=]() {
		if (!weakThis)
			return;
		weakThis->bypassAction(!bypassed);
	}));

	// Duplicate
	menu->addChild(createMenuItem("Duplicate", RACK_MOD_CTRL_NAME "+D", [=]() {
		if (!weakThis)
			return;
		weakThis->cloneAction(false);
	}));

	// Duplicate with cables
	menu->addChild(createMenuItem("└ with cables", RACK_MOD_SHIFT_NAME "+" RACK_MOD_CTRL_NAME "+D", [=]() {
		if (!weakThis)
			return;
		weakThis->cloneAction(true);
	}));

	// Delete
	menu->addChild(createMenuItem("Delete", "Backspace/Delete", [=]() {
		if (!weakThis)
			return;
		weakThis->removeAction();
	}, false, true));

	appendContextMenu(menu);
}

math::Vec ModuleWidget::getGridPosition() {
	return ((getPosition() - RACK_OFFSET) / RACK_GRID_SIZE).round();
}

void ModuleWidget::setGridPosition(math::Vec pos) {
	setPosition(pos * RACK_GRID_SIZE + RACK_OFFSET);
}

math::Vec ModuleWidget::getGridSize() {
	return (getSize() / RACK_GRID_SIZE).round();
}

math::Rect ModuleWidget::getGridBox() {
	return math::Rect(getGridPosition(), getGridSize());
}

math::Vec& ModuleWidget::dragOffset() {
	return internal->dragOffset;
}

bool& ModuleWidget::dragEnabled() {
	return internal->dragEnabled;
}

engine::Module* ModuleWidget::releaseModule() {
	engine::Module* module = this->module;
	this->module = NULL;
	return module;
}


} // namespace app
} // namespace rack