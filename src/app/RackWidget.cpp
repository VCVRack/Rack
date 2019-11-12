#include <app/RackWidget.hpp>
#include <widget/TransparentWidget.hpp>
#include <app/RackRail.hpp>
#include <app/Scene.hpp>
#include <app/ModuleBrowser.hpp>
#include <settings.hpp>
#include <plugin.hpp>
#include <engine/Engine.hpp>
#include <app.hpp>
#include <asset.hpp>
#include <patch.hpp>
#include <osdialog.h>
#include <map>
#include <algorithm>
#include <queue>


namespace rack {
namespace app {


static ModuleWidget* moduleFromJson(json_t* moduleJ) {
	// Get slugs
	json_t* pluginSlugJ = json_object_get(moduleJ, "plugin");
	if (!pluginSlugJ)
		return NULL;
	std::string pluginSlug = json_string_value(pluginSlugJ);
	pluginSlug = plugin::normalizeSlug(pluginSlug);

	json_t* modelSlugJ = json_object_get(moduleJ, "model");
	if (!modelSlugJ)
		return NULL;
	std::string modelSlug = json_string_value(modelSlugJ);
	modelSlug = plugin::normalizeSlug(modelSlug);

	// Get Model
	plugin::Model* model = plugin::getModel(pluginSlug, modelSlug);
	if (!model)
		return NULL;

	// Create ModuleWidget
	ModuleWidget* moduleWidget = model->createModuleWidget();
	assert(moduleWidget);
	moduleWidget->fromJson(moduleJ);
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
		switch (e.key) {
			case GLFW_KEY_V: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					pastePresetClipboardAction();
					e.consume(this);
				}
			} break;
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

json_t* RackWidget::toJson() {
	// root
	json_t* rootJ = json_object();

	// Get module offset so modules are aligned to (0, 0) when the patch is loaded.
	math::Vec moduleOffset = math::Vec(INFINITY, INFINITY);
	for (widget::Widget* w : moduleContainer->children) {
		moduleOffset = moduleOffset.min(w->box.pos);
	}
	if (moduleContainer->children.empty()) {
		moduleOffset = RACK_OFFSET;
	}

	// modules
	json_t* modulesJ = json_array();
	for (widget::Widget* w : moduleContainer->children) {
		ModuleWidget* moduleWidget = dynamic_cast<ModuleWidget*>(w);
		assert(moduleWidget);
		// module
		json_t* moduleJ = moduleWidget->toJson();
		{
			// pos
			math::Vec pos = moduleWidget->box.pos.minus(moduleOffset);
			pos = pos.div(RACK_GRID_SIZE).round();
			json_t* posJ = json_pack("[i, i]", (int) pos.x, (int) pos.y);
			json_object_set_new(moduleJ, "pos", posJ);
		}
		json_array_append_new(modulesJ, moduleJ);
	}
	json_object_set_new(rootJ, "modules", modulesJ);

	// cables
	json_t* cablesJ = json_array();
	for (widget::Widget* w : cableContainer->children) {
		CableWidget* cw = dynamic_cast<CableWidget*>(w);
		assert(cw);

		// Only serialize complete cables
		if (!cw->isComplete())
			continue;

		json_t* cableJ = cw->toJson();
		json_array_append_new(cablesJ, cableJ);
	}
	json_object_set_new(rootJ, "cables", cablesJ);

	return rootJ;
}

void RackWidget::fromJson(json_t* rootJ) {
	// modules
	json_t* modulesJ = json_object_get(rootJ, "modules");
	if (!modulesJ)
		return;
	size_t moduleIndex;
	json_t* moduleJ;
	json_array_foreach(modulesJ, moduleIndex, moduleJ) {
		ModuleWidget* moduleWidget = moduleFromJson(moduleJ);

		if (moduleWidget) {
			// Before 1.0, the module ID was the index in the "modules" array
			if (APP->patch->isLegacy(2)) {
				moduleWidget->module->id = moduleIndex;
			}

			// pos
			json_t* posJ = json_object_get(moduleJ, "pos");
			double x, y;
			json_unpack(posJ, "[F, F]", &x, &y);
			math::Vec pos = math::Vec(x, y);
			if (APP->patch->isLegacy(1)) {
				// Before 0.6, positions were in pixel units
				moduleWidget->box.pos = pos;
			}
			else {
				moduleWidget->box.pos = pos.mult(RACK_GRID_SIZE);
			}
			moduleWidget->box.pos = moduleWidget->box.pos.plus(RACK_OFFSET);

			addModule(moduleWidget);
		}
		else {
			json_t* pluginSlugJ = json_object_get(moduleJ, "plugin");
			json_t* modelSlugJ = json_object_get(moduleJ, "model");
			std::string pluginSlug = json_string_value(pluginSlugJ);
			std::string modelSlug = json_string_value(modelSlugJ);
			APP->patch->warningLog += string::f("Could not find module \"%s\" of plugin \"%s\"\n", modelSlug.c_str(), pluginSlug.c_str());
		}
	}

	// cables
	json_t* cablesJ = json_object_get(rootJ, "cables");
	// Before 1.0, cables were called wires
	if (!cablesJ)
		cablesJ = json_object_get(rootJ, "wires");
	assert(cablesJ);
	size_t cableIndex;
	json_t* cableJ;
	json_array_foreach(cablesJ, cableIndex, cableJ) {
		// Create a unserialize cable
		CableWidget* cw = new CableWidget;
		cw->fromJson(cableJ);
		if (!cw->isComplete()) {
			delete cw;
			continue;
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
	if (moduleJ) {
		ModuleWidget* mw = moduleFromJson(moduleJ);
		json_decref(moduleJ);

		// Reset ID so the Engine automatically assigns a new one
		mw->module->id = -1;

		addModuleAtMouse(mw);

		// history::ModuleAdd
		history::ModuleAdd* h = new history::ModuleAdd;
		h->setModule(mw);
		APP->history->push(h);
	}
	else {
		WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
	}
}

static void RackWidget_updateAdjacent(RackWidget* that) {
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

	if (m->module) {
		// Add module to Engine
		APP->engine->addModule(m->module);
	}
	RackWidget_updateAdjacent(this);
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

	if (m->module) {
		// Remove module from Engine
		APP->engine->removeModule(m->module);
	}

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
		if (mwBox.isIntersecting(w2->box)) {
			return false;
		}
	}

	// Accept requested position
	mw->box = mwBox;
	RackWidget_updateAdjacent(this);
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
	assert(0);
}

void RackWidget::setModulePosForce(ModuleWidget* mw, math::Vec pos) {
	mw->box.pos = pos.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);

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
		float x = xLimit - w->box.size.x;
		x = std::round(x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
		if (w->box.pos.x < x)
			break;
		w->box.pos.x = x;
		xLimit = x;
	}

	// Shove right modules
	xLimit = mw->box.pos.x + mw->box.size.x;
	for (auto it = rightModules.begin(); it != rightModules.end(); it++) {
		widget::Widget* w = *it;
		float x = xLimit;
		x = std::round(x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
		if (w->box.pos.x > x)
			break;
		w->box.pos.x = x;
		xLimit = x + w->box.size.x;
	}

	RackWidget_updateAdjacent(this);
}

ModuleWidget* RackWidget::getModule(int moduleId) {
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

void RackWidget::updateModuleDragPositions() {
	moduleDragPositions.clear();
	for (widget::Widget* w : moduleContainer->children) {
		ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
		assert(mw);
		moduleDragPositions[mw->module->id] = mw->box.pos;
	}
}

history::ComplexAction* RackWidget::getModuleDragAction() {
	history::ComplexAction* h = new history::ComplexAction;

	for (widget::Widget* w : moduleContainer->children) {
		ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
		assert(mw);
		// It is possible to add modules to the rack while dragging, so ignore modules that don't exist.
		auto it = moduleDragPositions.find(mw->module->id);
		if (it == moduleDragPositions.end())
			continue;
		// Create ModuleMove action if the module was moved.
		math::Vec pos = it->second;
		if (!pos.isEqual(mw->box.pos)) {
			history::ModuleMove* mmh = new history::ModuleMove;
			mmh->moduleId = mw->module->id;
			mmh->oldPos = pos;
			mmh->newPos = mw->box.pos;
			h->push(mmh);
		}
	}
	return h;
}


void RackWidget::clearCables() {
	for (widget::Widget* w : cableContainer->children) {
		CableWidget* cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (!cw->isComplete())
			continue;

		APP->engine->removeCable(cw->cable);
	}
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

	APP->history->push(complexAction);
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

void RackWidget::addCable(CableWidget* w) {
	assert(w->isComplete());
	APP->engine->addCable(w->cable);
	cableContainer->addChild(w);
}

void RackWidget::removeCable(CableWidget* w) {
	assert(w->isComplete());
	APP->engine->removeCable(w->cable);
	cableContainer->removeChild(w);
}

void RackWidget::setIncompleteCable(CableWidget* w) {
	if (incompleteCable) {
		cableContainer->removeChild(incompleteCable);
		delete incompleteCable;
		incompleteCable = NULL;
	}
	if (w) {
		cableContainer->addChild(w);
		incompleteCable = w;
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
		// Ignore incomplete cables
		if (!cw->isComplete())
			continue;
		if (cw->inputPort == port || cw->outputPort == port)
			return cw;
	}
	return NULL;
}

CableWidget* RackWidget::getCable(int cableId) {
	for (widget::Widget* w : cableContainer->children) {
		CableWidget* cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (cw->cable->id == cableId)
			return cw;
	}
	return NULL;
}

std::list<CableWidget*> RackWidget::getCablesOnPort(PortWidget* port) {
	assert(port);
	std::list<CableWidget*> cables;
	for (widget::Widget* w : cableContainer->children) {
		CableWidget* cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (cw->inputPort == port || cw->outputPort == port) {
			cables.push_back(cw);
		}
	}
	return cables;
}


} // namespace app
} // namespace rack
