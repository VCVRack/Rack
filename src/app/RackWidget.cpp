#include <map>
#include <algorithm>
#include <queue>
#include <functional>

#include <osdialog.h>

#include <app/RackWidget.hpp>
#include <widget/TransparentWidget.hpp>
#include <app/RailWidget.hpp>
#include <app/Scene.hpp>
#include <app/ModuleBrowser.hpp>
#include <settings.hpp>
#include <plugin.hpp>
#include <engine/Engine.hpp>
#include <context.hpp>
#include <asset.hpp>
#include <patch.hpp>
#include <helpers.hpp>


namespace rack {
namespace app {


/** Creates a new Module and ModuleWidget */
static ModuleWidget* moduleWidgetFromJson(json_t* moduleJ) {
	plugin::Model* model = plugin::modelFromJson(moduleJ);
	assert(model);
	engine::Module* module = model->createModule();
	assert(module);
	module->fromJson(moduleJ);

	// Create ModuleWidget
	ModuleWidget* moduleWidget = module->model->createModuleWidget(module);
	assert(moduleWidget);
	return moduleWidget;
}


struct ModuleContainer : widget::Widget {
	void draw(const DrawArgs& args) override {
		// Draw shadows behind each ModuleWidget first, so the shadow doesn't overlap the front of other ModuleWidgets.
		for (widget::Widget* child : children) {
			ModuleWidget* w = dynamic_cast<ModuleWidget*>(child);
			assert(w);

			nvgSave(args.vg);
			nvgTranslate(args.vg, child->box.pos.x, child->box.pos.y);
			w->drawShadow(args);
			nvgRestore(args.vg);
		}

		Widget::draw(args);
	}
};


struct CableContainer : widget::TransparentWidget {
	void draw(const DrawArgs& args) override {
		// Draw Plugs
		Widget::draw(args);

		// Draw cable shadows
		DrawArgs args1 = args;
		args1.layer = 1;
		Widget::draw(args1);

		// Draw cables
		DrawArgs args2 = args;
		args2.layer = 2;
		Widget::draw(args2);
	}
};


struct RackWidget::Internal {
	RailWidget* rail = NULL;
	widget::Widget* moduleContainer = NULL;
	widget::Widget* cableContainer = NULL;
	/** The last mouse position in the RackWidget */
	math::Vec mousePos;

	bool selecting = false;
	math::Vec selectionStart;
	math::Vec selectionEnd;
};


RackWidget::RackWidget() {
	internal = new Internal;

	internal->rail = new RailWidget;
	addChild(internal->rail);

	internal->moduleContainer = new ModuleContainer;
	addChild(internal->moduleContainer);

	internal->cableContainer = new CableContainer;
	addChild(internal->cableContainer);
}

RackWidget::~RackWidget() {
	clear();
	delete internal;
}

void RackWidget::step() {
	Widget::step();
}

void RackWidget::draw(const DrawArgs& args) {
	// Darken all children by user setting
	float b = settings::rackBrightness;
	nvgGlobalTint(args.vg, nvgRGBAf(b, b, b, 1));

	Widget::draw(args);

	// Draw selection rectangle
	if (internal->selecting) {
		nvgBeginPath(args.vg);
		math::Rect selectionBox = math::Rect::fromCorners(internal->selectionStart, internal->selectionEnd);
		nvgRect(args.vg, RECT_ARGS(selectionBox));
		nvgFillColor(args.vg, nvgRGBAf(1, 0, 0, 0.25));
		nvgFill(args.vg);
		nvgStrokeWidth(args.vg, 2.0);
		nvgStrokeColor(args.vg, nvgRGBAf(1, 0, 0, 0.5));
		nvgStroke(args.vg);
	}
}

void RackWidget::onHover(const HoverEvent& e) {
	// Set before calling children's onHover()
	internal->mousePos = e.pos;

	OpaqueWidget::onHover(e);
}

void RackWidget::onHoverKey(const HoverKeyEvent& e) {
	OpaqueWidget::onHoverKey(e);
	if (e.isConsumed())
		return;

	if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		if (e.keyName == "v" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			pastePresetClipboardAction();
			e.consume(this);
		}
	}
}

void RackWidget::onButton(const ButtonEvent& e) {
	Widget::onButton(e);
	e.stopPropagating();
	if (e.isConsumed())
		return;

	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
		APP->scene->moduleBrowser->show();
		e.consume(this);
	}
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
		e.consume(this);
	}
}

void RackWidget::onDragStart(const DragStartEvent& e) {
	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		// Deselect all modules
		updateModuleSelections();
		internal->selecting = true;
		internal->selectionStart = internal->mousePos;
		internal->selectionEnd = internal->mousePos;
	}
}

void RackWidget::onDragEnd(const DragEndEvent& e) {
	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		internal->selecting = false;
	}
}

void RackWidget::onDragHover(const DragHoverEvent& e) {
	// Set before calling children's onDragHover()
	internal->mousePos = e.pos;

	if (internal->selecting) {
		internal->selectionEnd = internal->mousePos;
		updateModuleSelections();
	}

	OpaqueWidget::onDragHover(e);
}

widget::Widget* RackWidget::getModuleContainer() {
	return internal->moduleContainer;
}

widget::Widget* RackWidget::getCableContainer() {
	return internal->cableContainer;
}

math::Vec RackWidget::getMousePos() {
	return internal->mousePos;
}

void RackWidget::clear() {
	// This isn't required because removing all ModuleWidgets should remove all cables, but do it just in case.
	clearCables();
	// Remove ModuleWidgets
	for (ModuleWidget* mw : getModules()) {
		removeModule(mw);
		delete mw;
	}
}

void RackWidget::mergeJson(json_t* rootJ) {
	// Get module offset so modules are aligned to (0, 0) when the patch is loaded.
	math::Vec moduleOffset = math::Vec(INFINITY, INFINITY);
	for (widget::Widget* w : internal->moduleContainer->children) {
		moduleOffset = moduleOffset.min(w->box.pos);
	}
	if (internal->moduleContainer->children.empty()) {
		moduleOffset = RACK_OFFSET;
	}

	// modules
	json_t* modulesJ = json_object_get(rootJ, "modules");
	if (!modulesJ)
		return;
	size_t moduleIndex;
	json_t* moduleJ;
	json_array_foreach(modulesJ, moduleIndex, moduleJ) {
		// module
		json_t* idJ = json_object_get(moduleJ, "id");
		if (!idJ)
			continue;
		int64_t id = json_integer_value(idJ);
		// TODO Legacy v0.6?
		ModuleWidget* moduleWidget = getModule(id);
		if (!moduleWidget) {
			WARN("Cannot find ModuleWidget %" PRId64, id);
			continue;
		}

		// pos
		math::Vec pos = moduleWidget->box.pos.minus(moduleOffset);
		pos = pos.div(RACK_GRID_SIZE).round();
		json_t* posJ = json_pack("[i, i]", (int) pos.x, (int) pos.y);
		json_object_set_new(moduleJ, "pos", posJ);
	}

	// cables
	json_t* cablesJ = json_object_get(rootJ, "cables");
	if (!cablesJ)
		return;
	size_t cableIndex;
	json_t* cableJ;
	json_array_foreach(cablesJ, cableIndex, cableJ) {
		// cable
		json_t* idJ = json_object_get(cableJ, "id");
		if (!idJ)
			continue;
		int64_t id = json_integer_value(idJ);
		CableWidget* cw = getCable(id);
		if (!cw) {
			WARN("Cannot find CableWidget %" PRId64, id);
			continue;
		}

		json_t* cwJ = cw->toJson();
		// Merge cable JSON object
		json_object_update(cableJ, cwJ);
		json_decref(cwJ);
	}
}

void RackWidget::fromJson(json_t* rootJ) {
	// version
	std::string version;
	json_t* versionJ = json_object_get(rootJ, "version");
	if (versionJ)
		version = json_string_value(versionJ);

	bool legacyV05 = false;
	if (string::startsWith(version, "0.3.") || string::startsWith(version, "0.4.") || string::startsWith(version, "0.5.") || version == "dev") {
		legacyV05 = true;
	}


	// modules
	json_t* modulesJ = json_object_get(rootJ, "modules");
	if (!modulesJ)
		return;

	size_t moduleIndex;
	json_t* moduleJ;
	json_array_foreach(modulesJ, moduleIndex, moduleJ) {
		// Get module ID
		json_t* idJ = json_object_get(moduleJ, "id");
		int64_t id;
		if (idJ)
			id = json_integer_value(idJ);
		else
			id = moduleIndex;

		// Get Module
		engine::Module* module = APP->engine->getModule(id);
		if (!module) {
			WARN("Cannot find Module %" PRId64, id);
			continue;
		}

		// Create ModuleWidget
		ModuleWidget* moduleWidget = module->model->createModuleWidget(module);

		// pos
		json_t* posJ = json_object_get(moduleJ, "pos");
		double x = 0.0, y = 0.0;
		json_unpack(posJ, "[F, F]", &x, &y);
		math::Vec pos = math::Vec(x, y);
		if (legacyV05) {
			// In <=v0.5, positions were in pixel units
			moduleWidget->box.pos = pos;
		}
		else {
			moduleWidget->box.pos = pos.mult(RACK_GRID_SIZE);
		}
		moduleWidget->box.pos = moduleWidget->box.pos.plus(RACK_OFFSET);

		addModule(moduleWidget);
	}

	// cables
	json_t* cablesJ = json_object_get(rootJ, "cables");
	// In <=v0.6, cables were called wires
	if (!cablesJ)
		cablesJ = json_object_get(rootJ, "wires");
	if (!cablesJ)
		return;
	size_t cableIndex;
	json_t* cableJ;
	json_array_foreach(cablesJ, cableIndex, cableJ) {
		// Get cable ID
		json_t* idJ = json_object_get(cableJ, "id");
		int64_t id;
		if (idJ)
			id = json_integer_value(idJ);
		else
			id = cableIndex;

		// Get Cable
		engine::Cable* cable = APP->engine->getCable(id);
		if (!cable) {
			WARN("Cannot find Cable %" PRId64, id);
			continue;
		}

		// Create CableWidget
		CableWidget* cw = new CableWidget;
		cw->setCable(cable);
		cw->fromJson(cableJ);
		// In <=v1, cable colors were not serialized, so choose one from the available colors.
		if (cw->color.a == 0.f) {
			cw->setNextCableColor();
		}
		addCable(cw);
	}
}

void RackWidget::pastePresetClipboardAction() {
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

	// Because we are creating a new module, we don't want to use the IDs from the JSON.
	json_object_del(moduleJ, "id");
	json_object_del(moduleJ, "leftModuleId");
	json_object_del(moduleJ, "rightModuleId");

	ModuleWidget* mw;
	try {
		mw = moduleWidgetFromJson(moduleJ);
	}
	catch (Exception& e) {
		WARN("%s", e.what());
		return;
	}
	assert(mw);
	assert(mw->module);

	APP->engine->addModule(mw->module);
	addModuleAtMouse(mw);

	// history::ModuleAdd
	history::ModuleAdd* h = new history::ModuleAdd;
	h->setModule(mw);
	APP->history->push(h);
}

static void RackWidget_updateExpanders(RackWidget* that) {
	for (widget::Widget* w : that->internal->moduleContainer->children) {
		math::Vec pLeft = w->box.pos.div(RACK_GRID_SIZE).round();
		math::Vec pRight = w->box.getTopRight().div(RACK_GRID_SIZE).round();
		ModuleWidget* mwLeft = NULL;
		ModuleWidget* mwRight = NULL;

		// Find adjacent modules
		for (widget::Widget* w2 : that->internal->moduleContainer->children) {
			if (w2 == w)
				continue;

			math::Vec p2Left = w2->box.pos.div(RACK_GRID_SIZE).round();
			math::Vec p2Right = w2->box.getTopRight().div(RACK_GRID_SIZE).round();

			// Check if this is a left module
			if (p2Right.equals(pLeft)) {
				mwLeft = dynamic_cast<ModuleWidget*>(w2);
			}

			// Check if this is a right module
			if (p2Left.equals(pRight)) {
				mwRight = dynamic_cast<ModuleWidget*>(w2);
			}
		}

		ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
		mw->module->leftExpander.moduleId = mwLeft ? mwLeft->module->id : -1;
		mw->module->rightExpander.moduleId = mwRight ? mwRight->module->id : -1;
	}
}

void RackWidget::addModule(ModuleWidget* m) {
	assert(m);

	// Module must be 3U high and at least 1HP wide
	if (m->box.size.x < RACK_GRID_WIDTH / 2)
		throw Exception("Module %s width is %g px, must be at least %g px", m->model->getFullName().c_str(), m->box.size.x, RACK_GRID_WIDTH);

	if (m->box.size.y != RACK_GRID_HEIGHT)
		throw Exception("Module %s height is %g px, must be %g px", m->model->getFullName().c_str(), m->box.size.y, RACK_GRID_HEIGHT);

	internal->moduleContainer->addChild(m);

	RackWidget_updateExpanders(this);
}

void RackWidget::addModuleAtMouse(ModuleWidget* mw) {
	assert(mw);
	// Move module nearest to the mouse position
	math::Vec pos = internal->mousePos.minus(mw->box.size.div(2));
	setModulePosNearest(mw, pos);
	addModule(mw);
}

void RackWidget::removeModule(ModuleWidget* m) {
	// Unset touchedParamWidget
	if (touchedParam) {
		ModuleWidget* touchedModule = touchedParam->getAncestorOfType<ModuleWidget>();
		if (touchedModule == m)
			touchedParam = NULL;
	}

	// Disconnect cables
	m->disconnect();

	// Remove module from ModuleContainer
	internal->moduleContainer->removeChild(m);
}

bool RackWidget::requestModulePos(ModuleWidget* mw, math::Vec pos) {
	// Check intersection with other modules
	math::Rect mwBox = math::Rect(pos, mw->box.size);
	for (widget::Widget* w2 : internal->moduleContainer->children) {
		// Don't intersect with self
		if (mw == w2)
			continue;
		// Check intersection
		math::Rect w2Box = w2->box;
		if (mwBox.intersects(w2Box))
			return false;
	}

	// Accept requested position
	mw->setPosition(mwBox.pos);
	RackWidget_updateExpanders(this);
	return true;
}

static void eachNearestGridPos(math::Vec pos, std::function<bool(math::Vec pos)> f) {
	// Dijkstra's algorithm to generate a sorted list of Vecs closest to `pos`.

	// Comparison of distance of Vecs to `pos`
	auto cmpNearest = [&](const math::Vec & a, const math::Vec & b) {
		return a.minus(pos).square() > b.minus(pos).square();
	};
	// Comparison of dictionary order of Vecs
	auto cmp = [&](const math::Vec & a, const math::Vec & b) {
		if (a.x != b.x)
			return a.x < b.x;
		return a.y < b.y;
	};
	// Priority queue sorted by distance from `pos`
	std::priority_queue<math::Vec, std::vector<math::Vec>, decltype(cmpNearest)> queue(cmpNearest);
	// Set of already-tested Vecs
	std::set<math::Vec, decltype(cmp)> visited(cmp);
	// Seed priority queue with closest Vec
	math::Vec closestPos = pos.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);
	queue.push(closestPos);

	while (!queue.empty()) {
		math::Vec testPos = queue.top();
		// Check testPos
		if (f(testPos))
			return;
		// Move testPos to visited set
		queue.pop();
		visited.insert(testPos);

		// Add adjacent Vecs
		static const std::vector<math::Vec> deltas = {
			math::Vec(-1, 0).mult(RACK_GRID_SIZE),
			math::Vec(1, 0).mult(RACK_GRID_SIZE),
			math::Vec(0, -1).mult(RACK_GRID_SIZE),
			math::Vec(0, 1).mult(RACK_GRID_SIZE),
		};
		for (math::Vec delta : deltas) {
			math::Vec newPos = testPos.plus(delta);
			if (visited.find(newPos) == visited.end()) {
				queue.push(newPos);
			}
		}
	}

	// We failed to find a box. This shouldn't happen on an infinite rack.
	assert(false);
}

void RackWidget::setModulePosNearest(ModuleWidget* mw, math::Vec pos) {
	eachNearestGridPos(pos, [&](math::Vec pos) -> bool {
		return requestModulePos(mw, pos);
	});
}

void RackWidget::setModulePosForce(ModuleWidget* mw, math::Vec pos) {
	mw->setPosition(pos.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE));

	// Comparison of center X coordinates
	auto cmp = [&](const widget::Widget * a, const widget::Widget * b) {
		return a->box.pos.x + a->box.size.x / 2 < b->box.pos.x + b->box.size.x / 2;
	};

	// Collect modules to the left and right of `mw`
	std::set<widget::Widget*, decltype(cmp)> leftModules(cmp);
	std::set<widget::Widget*, decltype(cmp)> rightModules(cmp);
	for (widget::Widget* w2 : internal->moduleContainer->children) {
		if (w2 == mw)
			continue;
		// Modules must be on the same row as `mw`
		if (w2->box.pos.y != mw->box.pos.y)
			continue;
		if (cmp(w2, mw))
			leftModules.insert(w2);
		else
			rightModules.insert(w2);
	}

	// Shove left modules
	float xLimit = mw->box.pos.x;
	for (auto it = leftModules.rbegin(); it != leftModules.rend(); it++) {
		widget::Widget* w = *it;
		math::Vec newPos = w->box.pos;
		newPos.x = xLimit - w->box.size.x;
		newPos.x = std::round(newPos.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
		if (w->box.pos.x < newPos.x)
			break;
		w->setPosition(newPos);
		xLimit = newPos.x;
	}

	// Shove right modules
	xLimit = mw->box.pos.x + mw->box.size.x;
	for (auto it = rightModules.begin(); it != rightModules.end(); it++) {
		widget::Widget* w = *it;
		math::Vec newPos = w->box.pos;
		newPos.x = xLimit;
		newPos.x = std::round(newPos.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
		if (w->box.pos.x > newPos.x)
			break;
		w->setPosition(newPos);
		xLimit = newPos.x + w->box.size.x;
	}

	RackWidget_updateExpanders(this);
}

ModuleWidget* RackWidget::getModule(int64_t moduleId) {
	for (widget::Widget* w : internal->moduleContainer->children) {
		ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
		assert(mw);
		if (mw->module->id == moduleId)
			return mw;
	}
	return NULL;
}

std::list<ModuleWidget*> RackWidget::getModules() {
	std::list<ModuleWidget*> mws;
	for (widget::Widget* w : internal->moduleContainer->children) {
		ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
		assert(mw);
		mws.push_back(mw);
	}
	return mws;
}

bool RackWidget::hasModules() {
	return internal->moduleContainer->children.empty();
}

void RackWidget::updateModuleOldPositions() {
	// Set all modules' oldPos field from their current position.
	for (ModuleWidget* mw : getModules()) {
		mw->oldPos() = mw->box.pos;
	}
}

history::ComplexAction* RackWidget::getModuleDragAction() {
	history::ComplexAction* h = new history::ComplexAction;

	for (ModuleWidget* mw : getModules()) {
		// Create ModuleMove action if the module was moved.
		math::Vec oldPos = mw->oldPos();
		if (!oldPos.equals(mw->box.pos)) {
			history::ModuleMove* mmh = new history::ModuleMove;
			mmh->moduleId = mw->module->id;
			mmh->oldPos = oldPos;
			mmh->newPos = mw->box.pos;
			h->push(mmh);
		}
	}

	return h;
}

void RackWidget::updateModuleSelections() {
	math::Rect selectionBox = math::Rect::fromCorners(internal->selectionStart, internal->selectionEnd);
	for (ModuleWidget* mw : getModules()) {
		bool selected = internal->selecting && selectionBox.intersects(mw->box);
		mw->selected() = selected;
	}
}

void RackWidget::deselectModules() {
	for (ModuleWidget* mw : getModules()) {
		mw->selected() = false;
	}
}

void RackWidget::selectAllModules() {
	for (ModuleWidget* mw : getModules()) {
		mw->selected() = true;
	}
}

bool RackWidget::hasSelectedModules() {
	for (widget::Widget* w : internal->moduleContainer->children) {
		ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
		assert(mw);
		if (mw->selected())
			return true;
	}
	return false;
}

int RackWidget::getNumSelectedModules() {
	int count = 0;
	for (widget::Widget* w : internal->moduleContainer->children) {
		ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
		assert(mw);
		if (mw->selected())
			count++;
	}
	return count;
}

std::list<ModuleWidget*> RackWidget::getSelectedModules() {
	std::list<ModuleWidget*> mws;
	for (widget::Widget* w : internal->moduleContainer->children) {
		ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
		assert(mw);
		if (mw->selected())
			mws.push_back(mw);
	}
	return mws;
}

void RackWidget::resetSelectedModulesAction() {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "reset modules";

	for (ModuleWidget* mw : getSelectedModules()) {
		assert(mw->module);

		// history::ModuleChange
		history::ModuleChange* h = new history::ModuleChange;
		h->moduleId = mw->module->id;
		h->oldModuleJ = mw->toJson();

		APP->engine->resetModule(mw->module);

		h->newModuleJ = mw->toJson();
		complexAction->push(h);
	}

	APP->history->push(complexAction);
}

void RackWidget::randomizeSelectedModulesAction() {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "randomize modules";

	for (ModuleWidget* mw : getSelectedModules()) {
		assert(mw->module);

		// history::ModuleChange
		history::ModuleChange* h = new history::ModuleChange;
		h->moduleId = mw->module->id;
		h->oldModuleJ = mw->toJson();

		APP->engine->randomizeModule(mw->module);

		h->newModuleJ = mw->toJson();
		complexAction->push(h);
	}

	APP->history->push(complexAction);
}

void RackWidget::disconnectSelectedModulesAction() {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "disconnect cables";

	for (ModuleWidget* mw : getSelectedModules()) {
		mw->appendDisconnectActions(complexAction);
	}

	if (!complexAction->isEmpty())
		APP->history->push(complexAction);
	else
		delete complexAction;
}

void RackWidget::cloneSelectedModulesAction() {
	// TODO
}

void RackWidget::bypassSelectedModulesAction(bool bypassed) {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = bypassed ? "bypass modules" : "un-bypass modules";

	for (ModuleWidget* mw : getSelectedModules()) {
		assert(mw->module);
		if (mw->module->isBypassed() == bypassed)
			continue;

		// history::ModuleBypass
		history::ModuleBypass* h = new history::ModuleBypass;
		h->moduleId = mw->module->id;
		h->bypassed = bypassed;
		complexAction->push(h);

		APP->engine->bypassModule(mw->module, bypassed);
	}

	if (!complexAction->isEmpty())
		APP->history->push(complexAction);
	else
		delete complexAction;
}

bool RackWidget::areSelectedModulesBypassed() {
	for (ModuleWidget* mw : getSelectedModules()) {
		if (!mw->getModule()->isBypassed())
			return false;
	}
	return true;
}

void RackWidget::deleteSelectedModulesAction() {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "remove modules";

	for (ModuleWidget* mw : getSelectedModules()) {
		mw->appendDisconnectActions(complexAction);

		// history::ModuleRemove
		history::ModuleRemove* moduleRemove = new history::ModuleRemove;
		moduleRemove->setModule(mw);
		complexAction->push(moduleRemove);

		removeModule(mw);
		delete mw;
	}

	APP->history->push(complexAction);
}

bool RackWidget::requestSelectedModulePos(math::Vec delta) {
	// Check intersection with other modules
	std::map<widget::Widget*, math::Rect> mwBoxes;
	for (ModuleWidget* mw : getSelectedModules()) {
		math::Rect mwBox = mw->box;
		mwBox.pos += delta;
		mwBoxes[mw] = mwBox;
	}

	for (widget::Widget* w2 : internal->moduleContainer->children) {
		// Don't intersect with selected modules
		auto it = mwBoxes.find(w2);
		if (it != mwBoxes.end())
			continue;
		math::Rect w2Box = w2->box;
		// Check intersection with all selected modules
		for (const auto& pair : mwBoxes) {
			if (pair.second.intersects(w2Box))
				return false;
		}
	}

	// Accept requested position
	for (const auto& pair : mwBoxes) {
		pair.first->setPosition(pair.second.pos);
	}
	RackWidget_updateExpanders(this);
	return true;
}

void RackWidget::setSelectedModulesPosNearest(math::Vec delta) {
	eachNearestGridPos(delta, [&](math::Vec delta) -> bool {
		return requestSelectedModulePos(delta);
	});
}

void RackWidget::appendSelectionContextMenu(ui::Menu* menu) {
	int n = getNumSelectedModules();
	menu->addChild(createMenuLabel(string::f("%d selected %s", n, n == 1 ? "module" : "modules")));

	// Select all
	menu->addChild(createMenuItem("Select all", RACK_MOD_CTRL_NAME "+A", [=]() {
		selectAllModules();
	}));

	// Deselect
	menu->addChild(createMenuItem("Deselect", RACK_MOD_CTRL_NAME "+" RACK_MOD_SHIFT_NAME "+A", [=]() {
		deselectModules();
	}, n == 0));

	// Initialize
	menu->addChild(createMenuItem("Initialize", RACK_MOD_CTRL_NAME "+I", [=]() {
		resetSelectedModulesAction();
	}, n == 0));

	// Randomize
	menu->addChild(createMenuItem("Randomize", RACK_MOD_CTRL_NAME "+R", [=]() {
		randomizeSelectedModulesAction();
	}, n == 0));

	// Disconnect cables
	menu->addChild(createMenuItem("Disconnect cables", RACK_MOD_CTRL_NAME "+U", [=]() {
		disconnectSelectedModulesAction();
	}, n == 0));

	// Duplicate
	menu->addChild(createMenuItem("Duplicate", RACK_MOD_CTRL_NAME "+D", [=]() {
		cloneSelectedModulesAction();
	}));

	// Bypass
	std::string bypassText = RACK_MOD_CTRL_NAME "+E";
	bool bypassed = (n > 0) && areSelectedModulesBypassed();
	if (bypassed)
		bypassText += " " CHECKMARK_STRING;
	menu->addChild(createMenuItem("Bypass", bypassText, [=]() {
		bypassSelectedModulesAction(!bypassed);
	}, n == 0));

	// Delete
	menu->addChild(createMenuItem("Delete", "Backspace/Delete", [=]() {
		deleteSelectedModulesAction();
	}, n == 0));
}

void RackWidget::clearCables() {
	incompleteCable = NULL;
	internal->cableContainer->clearChildren();
}

void RackWidget::clearCablesAction() {
	// Add CableRemove for every cable to a ComplexAction
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "clear cables";

	for (CableWidget* cw : getCompleteCables()) {
		// history::CableRemove
		history::CableRemove* h = new history::CableRemove;
		h->setCable(cw);
		complexAction->push(h);
	}

	if (!complexAction->isEmpty())
		APP->history->push(complexAction);
	else
		delete complexAction;

	clearCables();
}

void RackWidget::clearCablesOnPort(PortWidget* port) {
	for (CableWidget* cw : getCablesOnPort(port)) {
		// Check if cable is connected to port
		if (cw == incompleteCable) {
			incompleteCable = NULL;
			internal->cableContainer->removeChild(cw);
		}
		else {
			removeCable(cw);
		}
		delete cw;
	}
}

void RackWidget::addCable(CableWidget* cw) {
	assert(cw->isComplete());
	internal->cableContainer->addChild(cw);
}

void RackWidget::removeCable(CableWidget* cw) {
	assert(cw->isComplete());
	internal->cableContainer->removeChild(cw);
}

void RackWidget::setIncompleteCable(CableWidget* cw) {
	if (incompleteCable) {
		internal->cableContainer->removeChild(incompleteCable);
		delete incompleteCable;
		incompleteCable = NULL;
	}
	if (cw) {
		internal->cableContainer->addChild(cw);
		incompleteCable = cw;
	}
}

CableWidget* RackWidget::releaseIncompleteCable() {
	if (!incompleteCable)
		return NULL;

	CableWidget* cw = incompleteCable;
	internal->cableContainer->removeChild(incompleteCable);
	incompleteCable = NULL;
	return cw;
}

CableWidget* RackWidget::getTopCable(PortWidget* port) {
	for (auto it = internal->cableContainer->children.rbegin(); it != internal->cableContainer->children.rend(); it++) {
		CableWidget* cw = dynamic_cast<CableWidget*>(*it);
		assert(cw);
		if (cw->inputPort == port || cw->outputPort == port)
			return cw;
	}
	return NULL;
}

CableWidget* RackWidget::getCable(int64_t cableId) {
	for (widget::Widget* w : internal->cableContainer->children) {
		CableWidget* cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (!cw->cable)
			continue;
		if (cw->cable->id == cableId)
			return cw;
	}
	return NULL;
}

std::list<CableWidget*> RackWidget::getCompleteCables() {
	std::list<CableWidget*> cws;
	for (widget::Widget* w : internal->cableContainer->children) {
		CableWidget* cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (cw->isComplete())
			cws.push_back(cw);
	}
	return cws;
}

std::list<CableWidget*> RackWidget::getCablesOnPort(PortWidget* port) {
	assert(port);
	std::list<CableWidget*> cws;
	for (widget::Widget* w : internal->cableContainer->children) {
		CableWidget* cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (cw->inputPort == port || cw->outputPort == port) {
			cws.push_back(cw);
		}
	}
	return cws;
}


} // namespace app
} // namespace rack
