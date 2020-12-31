#include <map>
#include <algorithm>
#include <queue>

#include <osdialog.h>

#include <app/RackWidget.hpp>
#include <widget/TransparentWidget.hpp>
#include <app/RackRail.hpp>
#include <app/Scene.hpp>
#include <app/ModuleBrowser.hpp>
#include <settings.hpp>
#include <plugin.hpp>
#include <engine/Engine.hpp>
#include <context.hpp>
#include <asset.hpp>
#include <patch.hpp>


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
		// Draw cable plugs
		for (widget::Widget* w : children) {
			CableWidget* cw = dynamic_cast<CableWidget*>(w);
			assert(cw);
			cw->drawPlugs(args);
		}

		Widget::draw(args);
	}
};


RackWidget::RackWidget() {
	railFb = new widget::FramebufferWidget;
	railFb->box.size = math::Vec();
	railFb->oversample = 1.0;
	// Don't redraw when the world offset of the rail FramebufferWidget changes its fractional value.
	railFb->dirtyOnSubpixelChange = false;
	{
		RackRail* rail = new RackRail;
		rail->box.size = math::Vec();
		railFb->addChild(rail);
	}
	addChild(railFb);

	moduleContainer = new ModuleContainer;
	addChild(moduleContainer);

	cableContainer = new CableContainer;
	addChild(cableContainer);
}

RackWidget::~RackWidget() {
	clear();
}

void RackWidget::step() {
	Widget::step();
}

void RackWidget::draw(const DrawArgs& args) {
	// Resize and reposition the RackRail to align on the grid.
	math::Rect railBox;
	railBox.pos = args.clipBox.pos.div(BUS_BOARD_GRID_SIZE).floor().mult(BUS_BOARD_GRID_SIZE);
	railBox.size = args.clipBox.size.div(BUS_BOARD_GRID_SIZE).ceil().plus(math::Vec(1, 1)).mult(BUS_BOARD_GRID_SIZE);
	if (!railFb->box.size.isEqual(railBox.size)) {
		railFb->dirty = true;
	}
	railFb->box = railBox;

	RackRail* rail = railFb->getFirstDescendantOfType<RackRail>();
	rail->box.size = railFb->box.size;

	Widget::draw(args);
}

void RackWidget::onHover(const event::Hover& e) {
	// Set before calling children's onHover()
	mousePos = e.pos;
	OpaqueWidget::onHover(e);
}

void RackWidget::onHoverKey(const event::HoverKey& e) {
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

void RackWidget::onDragHover(const event::DragHover& e) {
	// Set before calling children's onDragHover()
	mousePos = e.pos;
	OpaqueWidget::onDragHover(e);
}

void RackWidget::onButton(const event::Button& e) {
	Widget::onButton(e);
	e.stopPropagating();
	if (e.isConsumed())
		return;

	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
		APP->scene->moduleBrowser->show();
		e.consume(this);
	}
}

void RackWidget::clear() {
	// This isn't required because removing all ModuleWidgets should remove all cables, but do it just in case.
	clearCables();
	// Remove ModuleWidgets
	std::list<widget::Widget*> widgets = moduleContainer->children;
	for (widget::Widget* w : widgets) {
		ModuleWidget* moduleWidget = dynamic_cast<ModuleWidget*>(w);
		assert(moduleWidget);
		removeModule(moduleWidget);
		delete moduleWidget;
	}
}

void RackWidget::mergeJson(json_t* rootJ) {
	// Get module offset so modules are aligned to (0, 0) when the patch is loaded.
	math::Vec moduleOffset = math::Vec(INFINITY, INFINITY);
	for (widget::Widget* w : moduleContainer->children) {
		moduleOffset = moduleOffset.min(w->box.pos);
	}
	if (moduleContainer->children.empty()) {
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
			WARN("Cannot find ModuleWidget with ID %lld", id);
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
			WARN("Cannot find CableWidget with ID %lld", id);
			continue;
		}

		json_t* cwJ = cw->toJson();
		// Merge cable JSON object
		json_object_update(cableJ, cwJ);
		json_decref(cwJ);
	}
}

void RackWidget::fromJson(json_t* rootJ) {
	// modules
	json_t* modulesJ = json_object_get(rootJ, "modules");
	assert(modulesJ);
	size_t moduleIndex;
	json_t* moduleJ;
	json_array_foreach(modulesJ, moduleIndex, moduleJ) {
		// module
		// Create ModuleWidget and attach it to existing Module from Engine.
		json_t* idJ = json_object_get(moduleJ, "id");
		if (!idJ)
			continue;
		int64_t id = json_integer_value(idJ);
		engine::Module* module = APP->engine->getModule(id);
		if (!module) {
			WARN("Cannot find module with ID %lld", id);
			continue;
		}

		ModuleWidget* moduleWidget = module->model->createModuleWidget(module);

		// Before 1.0, the module ID was the index in the "modules" array
		if (APP->patch->isLegacy(2)) {
			module->id = moduleIndex;
		}

		// pos
		json_t* posJ = json_object_get(moduleJ, "pos");
		double x, y;
		json_unpack(posJ, "[F, F]", &x, &y);
		math::Vec pos = math::Vec(x, y);
		if (APP->patch->isLegacy(1)) {
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
	assert(cablesJ);
	size_t cableIndex;
	json_t* cableJ;
	json_array_foreach(cablesJ, cableIndex, cableJ) {
		// cable
		// Get Cable from Engine
		json_t* idJ = json_object_get(cableJ, "id");
		if (!idJ)
			continue;
		int64_t id = json_integer_value(idJ);
		engine::Cable* cable = APP->engine->getCable(id);
		if (!cable) {
			WARN("Cannot find cable with ID %lld", id);
			continue;
		}

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

	try {
		ModuleWidget* mw = moduleWidgetFromJson(moduleJ);
		assert(mw);
		assert(mw->module);

		APP->engine->addModule(mw->module);
		addModuleAtMouse(mw);

		// history::ModuleAdd
		history::ModuleAdd* h = new history::ModuleAdd;
		h->setModule(mw);
		APP->history->push(h);
	}
	catch (Exception& e) {
		WARN("%s", e.what());
		return;
	}
}

static void RackWidget_updateExpanders(RackWidget* that) {
	for (widget::Widget* w : that->moduleContainer->children) {
		math::Vec pLeft = w->box.pos.div(RACK_GRID_SIZE).round();
		math::Vec pRight = w->box.getTopRight().div(RACK_GRID_SIZE).round();
		ModuleWidget* mwLeft = NULL;
		ModuleWidget* mwRight = NULL;

		// Find adjacent modules
		for (widget::Widget* w2 : that->moduleContainer->children) {
			if (w2 == w)
				continue;

			math::Vec p2Left = w2->box.pos.div(RACK_GRID_SIZE).round();
			math::Vec p2Right = w2->box.getTopRight().div(RACK_GRID_SIZE).round();

			// Check if this is a left module
			if (p2Right.isEqual(pLeft)) {
				mwLeft = dynamic_cast<ModuleWidget*>(w2);
			}

			// Check if this is a right module
			if (p2Left.isEqual(pRight)) {
				mwRight = dynamic_cast<ModuleWidget*>(w2);
			}
		}

		ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
		mw->module->leftExpander.moduleId = mwLeft ? mwLeft->module->id : -1;
		mw->module->rightExpander.moduleId = mwRight ? mwRight->module->id : -1;
	}
}

void RackWidget::addModule(ModuleWidget* m) {
	// Add module to ModuleContainer
	assert(m);
	// Module must be 3U high and at least 1HP wide
	assert(m->box.size.x >= RACK_GRID_WIDTH);
	assert(m->box.size.y == RACK_GRID_HEIGHT);
	moduleContainer->addChild(m);

	RackWidget_updateExpanders(this);
}

void RackWidget::addModuleAtMouse(ModuleWidget* mw) {
	assert(mw);
	// Move module nearest to the mouse position
	math::Vec pos = mousePos.minus(mw->box.size.div(2));
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
	moduleContainer->removeChild(m);
}

bool RackWidget::requestModulePos(ModuleWidget* mw, math::Vec pos) {
	// Check intersection with other modules
	math::Rect mwBox = math::Rect(pos, mw->box.size);
	for (widget::Widget* w2 : moduleContainer->children) {
		// Don't intersect with self
		if (mw == w2)
			continue;
		// Don't intersect with invisible modules
		if (!w2->visible)
			continue;
		// Check intersection
		if (mwBox.isIntersecting(w2->box))
			return false;
	}

	// Accept requested position
	mw->setPosition(mwBox.pos);
	RackWidget_updateExpanders(this);
	return true;
}

void RackWidget::setModulePosNearest(ModuleWidget* mw, math::Vec pos) {
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
		if (requestModulePos(mw, testPos))
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

void RackWidget::setModulePosForce(ModuleWidget* mw, math::Vec pos) {
	mw->setPosition(pos.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE));

	// Comparison of center X coordinates
	auto cmp = [&](const widget::Widget * a, const widget::Widget * b) {
		return a->box.pos.x + a->box.size.x / 2 < b->box.pos.x + b->box.size.x / 2;
	};

	// Collect modules to the left and right of `mw`
	std::set<widget::Widget*, decltype(cmp)> leftModules(cmp);
	std::set<widget::Widget*, decltype(cmp)> rightModules(cmp);
	for (widget::Widget* w2 : moduleContainer->children) {
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
	for (widget::Widget* w : moduleContainer->children) {
		ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
		assert(mw);
		if (mw->module->id == moduleId)
			return mw;
	}
	return NULL;
}

bool RackWidget::isEmpty() {
	return moduleContainer->children.empty();
}

void RackWidget::updateModuleOldPositions() {
	// Set all modules' oldPos field from their current position.
	for (widget::Widget* w : moduleContainer->children) {
		ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
		assert(mw);
		mw->oldPos() = mw->box.pos;
	}
}

history::ComplexAction* RackWidget::getModuleDragAction() {
	history::ComplexAction* h = new history::ComplexAction;

	for (widget::Widget* w : moduleContainer->children) {
		ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
		assert(mw);
		// Create ModuleMove action if the module was moved.
		math::Vec oldPos = mw->oldPos();
		if (!oldPos.isEqual(mw->box.pos)) {
			history::ModuleMove* mmh = new history::ModuleMove;
			mmh->moduleId = mw->module->id;
			mmh->oldPos = oldPos;
			mmh->newPos = mw->box.pos;
			h->push(mmh);
		}
	}

	if (h->isEmpty()) {
		delete h;
		return NULL;
	}
	return h;
}


void RackWidget::clearCables() {
	incompleteCable = NULL;
	cableContainer->clearChildren();
}

void RackWidget::clearCablesAction() {
	// Add CableRemove for every cable to a ComplexAction
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "clear cables";

	for (widget::Widget* w : cableContainer->children) {
		CableWidget* cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (!cw->isComplete())
			continue;

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
			cableContainer->removeChild(cw);
		}
		else {
			removeCable(cw);
		}
		delete cw;
	}
}

void RackWidget::addCable(CableWidget* cw) {
	assert(cw->isComplete());
	cableContainer->addChild(cw);
}

void RackWidget::removeCable(CableWidget* cw) {
	assert(cw->isComplete());
	cableContainer->removeChild(cw);
}

void RackWidget::setIncompleteCable(CableWidget* cw) {
	if (incompleteCable) {
		cableContainer->removeChild(incompleteCable);
		delete incompleteCable;
		incompleteCable = NULL;
	}
	if (cw) {
		cableContainer->addChild(cw);
		incompleteCable = cw;
	}
}

CableWidget* RackWidget::releaseIncompleteCable() {
	if (!incompleteCable)
		return NULL;

	CableWidget* cw = incompleteCable;
	cableContainer->removeChild(incompleteCable);
	incompleteCable = NULL;
	return cw;
}

CableWidget* RackWidget::getTopCable(PortWidget* port) {
	for (auto it = cableContainer->children.rbegin(); it != cableContainer->children.rend(); it++) {
		CableWidget* cw = dynamic_cast<CableWidget*>(*it);
		assert(cw);
		if (cw->inputPort == port || cw->outputPort == port)
			return cw;
	}
	return NULL;
}

CableWidget* RackWidget::getCable(int64_t cableId) {
	for (widget::Widget* w : cableContainer->children) {
		CableWidget* cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (!cw->cable)
			continue;
		if (cw->cable->id == cableId)
			return cw;
	}
	return NULL;
}

std::list<CableWidget*> RackWidget::getCablesOnPort(PortWidget* port) {
	assert(port);
	std::list<CableWidget*> cws;
	for (widget::Widget* w : cableContainer->children) {
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
