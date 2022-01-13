#include <map>
#include <algorithm>
#include <queue>
#include <functional>

#include <osdialog.h>

#include <app/RackWidget.hpp>
#include <widget/TransparentWidget.hpp>
#include <app/RailWidget.hpp>
#include <app/Scene.hpp>
#include <settings.hpp>
#include <plugin.hpp>
#include <engine/Engine.hpp>
#include <context.hpp>
#include <system.hpp>
#include <asset.hpp>
#include <patch.hpp>
#include <helpers.hpp>


namespace rack {
namespace app {


struct RackWidget::Internal {
	RailWidget* rail = NULL;
	widget::Widget* moduleContainer = NULL;
	widget::Widget* cableContainer = NULL;
	CableWidget* incompleteCable = NULL;
	int nextCableColorId = 0;
	/** The last mouse position in the RackWidget */
	math::Vec mousePos;

	bool selecting = false;
	math::Vec selectionStart;
	math::Vec selectionEnd;
	std::set<ModuleWidget*> selectedModules;
};


/** Creates a new Module and ModuleWidget */
static ModuleWidget* moduleWidgetFromJson(json_t* moduleJ) {
	plugin::Model* model = plugin::modelFromJson(moduleJ);
	assert(model);
	INFO("Creating module %s", model->getFullName().c_str());
	engine::Module* module = model->createModule();
	assert(module);
	module->fromJson(moduleJ);

	// Create ModuleWidget
	INFO("Creating module widget %s", model->getFullName().c_str());
	ModuleWidget* moduleWidget = module->model->createModuleWidget(module);
	assert(moduleWidget);
	return moduleWidget;
}


struct ModuleContainer : widget::Widget {
	void draw(const DrawArgs& args) override {
		// Draw ModuleWidget shadows
		Widget::drawLayer(args, -1);

		Widget::draw(args);
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		// Draw lights after translucent rectangle
		if (layer == 1) {
			Widget::drawLayer(args, 1);
		}
	}
};


struct CableContainer : widget::TransparentWidget {
	void draw(const DrawArgs& args) override {
		// Don't draw on layer 0
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer == 2) {
			// Draw Plugs
			Widget::draw(args);

			// Draw cable lights
			nvgSave(args.vg);
			nvgGlobalTint(args.vg, color::WHITE);
			Widget::drawLayer(args, 1);
			nvgRestore(args.vg);

			// Draw cable shadows
			Widget::drawLayer(args, 2);

			// Draw cables
			Widget::drawLayer(args, 3);
		}
	}
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
	float b = settings::rackBrightness;

	// Draw rack rails and modules
	Widget::draw(args);

	// Draw translucent dark rectangle
	if (b < 1.f) {
		// Get zoom level
		float t[6];
		nvgCurrentTransform(args.vg, t);
		float zoom = t[3];
		float radius = 300.0 / zoom;
		float brightness = 0.2f;
		// Draw mouse spotlight
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0.0, 0.0, VEC_ARGS(box.size));
		nvgFillPaint(args.vg, nvgRadialGradient(args.vg, VEC_ARGS(internal->mousePos), 0.0, radius, nvgRGBAf(0, 0, 0, 1.f - b - brightness), nvgRGBAf(0, 0, 0, 1.f - b)));
		nvgFill(args.vg);
	}

	// Draw lights and halos
	Widget::drawLayer(args, 1);

	// Tint all draws after this point
	nvgGlobalTint(args.vg, nvgRGBAf(b, b, b, 1));

	// Draw cables
	Widget::drawLayer(args, 2);

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
}

void RackWidget::onButton(const ButtonEvent& e) {
	Widget::onButton(e);
	e.stopPropagating();
	if (e.isConsumed())
		return;

	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
		APP->scene->browser->show();
		e.consume(this);
	}
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
		e.consume(this);
	}
}

void RackWidget::onDragStart(const DragStartEvent& e) {
	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		// Deselect all modules
		updateSelectionFromRect();
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
		updateSelectionFromRect();
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

void RackWidget::mergeJson(json_t* rootJ) {
	// Get module offset so modules are aligned to (0, 0) when the patch is loaded.
	math::Vec moduleOffset = internal->moduleContainer->getChildrenBoundingBox().pos;
	if (!moduleOffset.isFinite()) {
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
		ModuleWidget* mw = getModule(id);
		if (!mw) {
			WARN("Cannot find ModuleWidget %lld", (long long) id);
			continue;
		}

		// pos
		math::Vec pos = mw->box.pos.minus(moduleOffset);
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
			WARN("Cannot find CableWidget %lld", (long long) id);
			continue;
		}

		cw->mergeJson(cableJ);
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
			WARN("Cannot find Module %lld", (long long) id);
			continue;
		}

		// Create ModuleWidget
		INFO("Creating module widget %s", module->model->getFullName().c_str());
		ModuleWidget* mw = module->model->createModuleWidget(module);

		// pos
		json_t* posJ = json_object_get(moduleJ, "pos");
		double x = 0.0, y = 0.0;
		json_unpack(posJ, "[F, F]", &x, &y);
		math::Vec pos = math::Vec(x, y);
		if (legacyV05) {
			// In <=v0.5, positions were in pixel units
		}
		else {
			pos = pos.mult(RACK_GRID_SIZE);
		}
		pos = pos.plus(RACK_OFFSET);
		setModulePosNearest(mw, pos);

		internal->moduleContainer->addChild(mw);
	}

	RackWidget_updateExpanders(this);

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
		// In <=v0.6, the cable ID was the index in the array.
		if (idJ)
			id = json_integer_value(idJ);
		else
			id = cableIndex;

		// Get Cable
		engine::Cable* cable = APP->engine->getCable(id);
		if (!cable) {
			WARN("Cannot find Cable %lld", (long long) id);
			continue;
		}

		// Create CableWidget
		CableWidget* cw = new CableWidget;
		try {
			cw->setCable(cable);
			cw->fromJson(cableJ);
			// In <=v1, cable colors were not serialized, so choose one from the available colors.
			if (cw->color.a == 0.f) {
				cw->color = getNextCableColor();
			}
		}
		catch (Exception& e) {
			delete cw;
			// If creating CableWidget fails, remove Cable from Engine.
			APP->engine->removeCable(cable);
			delete cable;
			continue;
		}
		addCable(cw);
	}
}

struct PasteJsonReturn {
	std::map<int64_t, int64_t> newModuleIds;
};
static PasteJsonReturn RackWidget_pasteJson(RackWidget* that, json_t* rootJ, history::ComplexAction* complexAction) {
	that->deselectAll();

	std::map<int64_t, int64_t> newModuleIds;

	// modules
	json_t* modulesJ = json_object_get(rootJ, "modules");
	if (!modulesJ)
		return {};

	size_t moduleIndex;
	json_t* moduleJ;
	json_array_foreach(modulesJ, moduleIndex, moduleJ) {
		json_t* idJ = json_object_get(moduleJ, "id");
		if (!idJ)
			continue;
		int64_t id = json_integer_value(idJ);

		engine::Module::jsonStripIds(moduleJ);

		ModuleWidget* mw;
		try {
			mw = moduleWidgetFromJson(moduleJ);
		}
		catch (Exception& e) {
			WARN("%s", e.what());
			continue;
		}
		assert(mw);
		assert(mw->module);

		APP->engine->addModule(mw->module);

		// pos
		json_t* posJ = json_object_get(moduleJ, "pos");
		double x = 0.0, y = 0.0;
		json_unpack(posJ, "[F, F]", &x, &y);
		math::Vec pos = math::Vec(x, y);
		pos = pos.mult(RACK_GRID_SIZE);
		mw->box.pos = pos.plus(RACK_OFFSET);

		that->internal->moduleContainer->addChild(mw);
		that->select(mw);

		newModuleIds[id] = mw->module->id;
	}

	// This calls RackWidget_updateExpanders()
	that->setSelectionPosNearest(math::Vec(0, 0));

	// Add positioned selected modules to history
	for (ModuleWidget* mw : that->getSelected()) {
		// history::ModuleAdd
		history::ModuleAdd* h = new history::ModuleAdd;
		h->setModule(mw);
		complexAction->push(h);
	}

	// cables
	json_t* cablesJ = json_object_get(rootJ, "cables");
	if (cablesJ) {
		size_t cableIndex;
		json_t* cableJ;
		json_array_foreach(cablesJ, cableIndex, cableJ) {
			engine::Cable::jsonStripIds(cableJ);

			// Remap old module IDs to new IDs
			json_t* inputModuleIdJ = json_object_get(cableJ, "inputModuleId");
			if (!inputModuleIdJ)
				continue;
			int64_t inputModuleId = json_integer_value(inputModuleIdJ);
			inputModuleId = get(newModuleIds, inputModuleId, -1);
			if (inputModuleId < 0)
				continue;
			json_object_set(cableJ, "inputModuleId", json_integer(inputModuleId));

			json_t* outputModuleIdJ = json_object_get(cableJ, "outputModuleId");
			if (!outputModuleIdJ)
				continue;
			int64_t outputModuleId = json_integer_value(outputModuleIdJ);
			outputModuleId = get(newModuleIds, outputModuleId, -1);
			if (outputModuleId < 0)
				continue;
			json_object_set(cableJ, "outputModuleId", json_integer(outputModuleId));

			// Create Cable
			engine::Cable* cable = new engine::Cable;
			try {
				cable->fromJson(cableJ);
				APP->engine->addCable(cable);
			}
			catch (Exception& e) {
				WARN("Cannot paste cable: %s", e.what());
				delete cable;
				continue;
			}

			// Create CableWidget
			app::CableWidget* cw = new app::CableWidget;
			cw->setCable(cable);
			cw->fromJson(cableJ);
			that->addCable(cw);

			// history::CableAdd
			history::CableAdd* h = new history::CableAdd;
			h->setCable(cw);
			complexAction->push(h);
		}
	}

	return {newModuleIds};
}

void RackWidget::pasteJsonAction(json_t* rootJ) {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "paste modules";
	DEFER({
		if (!complexAction->isEmpty())
			APP->history->push(complexAction);
		else
			delete complexAction;
	});

	RackWidget_pasteJson(this, rootJ, complexAction);
}

void RackWidget::pasteModuleJsonAction(json_t* moduleJ) {
	engine::Module::jsonStripIds(moduleJ);

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
	h->name = "paste module";
	h->setModule(mw);
	APP->history->push(h);
}

void RackWidget::pasteClipboardAction() {
	const char* json = glfwGetClipboardString(APP->window->win);
	if (!json) {
		WARN("Could not get text from clipboard.");
		return;
	}

	json_error_t error;
	json_t* rootJ = json_loads(json, 0, &error);
	if (!rootJ) {
		WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		return;
	}
	DEFER({json_decref(rootJ);});

	json_t* modulesJ = json_object_get(rootJ, "modules");
	if (modulesJ) {
		// JSON is a selection of modules
		pasteJsonAction(rootJ);
	}
	else {
		// JSON is a single module
		pasteModuleJsonAction(rootJ);
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

	// Deselect module if selected
	internal->selectedModules.erase(m);

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

static math::Vec eachNearestGridPos(math::Vec pos, std::function<bool(math::Vec pos)> f) {
	// Dijkstra's algorithm to generate a sorted list of Vecs closest to `pos`.

	// Comparison of distance of Vecs to `pos`
	auto cmpNearest = [&](const math::Vec& a, const math::Vec& b) {
		return a.minus(pos).square() > b.minus(pos).square();
	};
	// Comparison of dictionary order of Vecs
	auto cmp = [](const math::Vec& a, const math::Vec& b) {
		if (a.y != b.y)
			return a.y < b.y;
		return a.x < b.x;
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
			return testPos;
		// Move testPos to visited set
		queue.pop();
		visited.insert(testPos);

		// Add adjacent Vecs
		static const std::vector<math::Vec> deltas = {
			math::Vec(1, 0).mult(RACK_GRID_SIZE),
			math::Vec(0, 1).mult(RACK_GRID_SIZE),
			math::Vec(-1, 0).mult(RACK_GRID_SIZE),
			math::Vec(0, -1).mult(RACK_GRID_SIZE),
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
	return math::Vec();
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

std::vector<ModuleWidget*> RackWidget::getModules() {
	std::vector<ModuleWidget*> mws;
	mws.reserve(internal->moduleContainer->children.size());
	for (widget::Widget* w : internal->moduleContainer->children) {
		ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
		assert(mw);
		mws.push_back(mw);
	}
	mws.shrink_to_fit();
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
	h->name = "move modules";

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

void RackWidget::updateSelectionFromRect() {
	math::Rect selectionBox = math::Rect::fromCorners(internal->selectionStart, internal->selectionEnd);
	deselectAll();
	for (ModuleWidget* mw : getModules()) {
		bool selected = internal->selecting && selectionBox.intersects(mw->box);
		if (selected)
			select(mw);
	}
}

void RackWidget::selectAll() {
	internal->selectedModules.clear();
	for (widget::Widget* w : internal->moduleContainer->children) {
		ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
		assert(mw);
		internal->selectedModules.insert(mw);
	}
}

void RackWidget::deselectAll() {
	internal->selectedModules.clear();
}

void RackWidget::select(ModuleWidget* mw, bool selected) {
	if (selected) {
		internal->selectedModules.insert(mw);
	}
	else {
		auto it = internal->selectedModules.find(mw);
		if (it != internal->selectedModules.end())
			internal->selectedModules.erase(it);
	}
}

bool RackWidget::hasSelection() {
	return !internal->selectedModules.empty();
}

const std::set<ModuleWidget*>& RackWidget::getSelected() {
	return internal->selectedModules;
}

bool RackWidget::isSelected(ModuleWidget* mw) {
	auto it = internal->selectedModules.find(mw);
	return (it != internal->selectedModules.end());
}

json_t* RackWidget::selectionToJson(bool cables) {
	json_t* rootJ = json_object();

	std::set<engine::Module*> modules;

	// modules
	json_t* modulesJ = json_array();
	for (ModuleWidget* mw : getSelected()) {
		json_t* moduleJ = mw->toJson();

		// pos
		math::Vec pos = mw->box.pos.minus(RACK_OFFSET);
		pos = pos.div(RACK_GRID_SIZE).round();
		json_t* posJ = json_pack("[i, i]", (int) pos.x, (int) pos.y);
		json_object_set_new(moduleJ, "pos", posJ);

		json_array_append_new(modulesJ, moduleJ);
		modules.insert(mw->getModule());
	}
	json_object_set_new(rootJ, "modules", modulesJ);

	if (cables) {
		// cables
		json_t* cablesJ = json_array();
		for (CableWidget* cw : getCompleteCables()) {
			// Only add cables attached on both ends to selected modules
			engine::Cable* cable = cw->getCable();
			if (!cable || !cable->inputModule || !cable->outputModule)
				continue;
			const auto inputIt = modules.find(cable->inputModule);
			if (inputIt == modules.end())
				continue;
			const auto outputIt = modules.find(cable->outputModule);
			if (outputIt == modules.end())
				continue;

			json_t* cableJ = cable->toJson();
			cw->mergeJson(cableJ);

			json_array_append_new(cablesJ, cableJ);
		}
		json_object_set_new(rootJ, "cables", cablesJ);
	}

	return rootJ;
}

static const char SELECTION_FILTERS[] = "VCV Rack module selection (.vcvs):vcvs";

void RackWidget::loadSelection(std::string path) {
	FILE* file = std::fopen(path.c_str(), "r");
	if (!file)
		throw Exception("Could not load selection file %s", path.c_str());
	DEFER({std::fclose(file);});

	INFO("Loading selection %s", path.c_str());

	json_error_t error;
	json_t* rootJ = json_loadf(file, 0, &error);
	if (!rootJ)
		throw Exception("File is not a valid selection file. JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
	DEFER({json_decref(rootJ);});

	pasteJsonAction(rootJ);
}

void RackWidget::loadSelectionDialog() {
	std::string selectionDir = asset::user("selections");
	system::createDirectories(selectionDir);

	osdialog_filters* filters = osdialog_filters_parse(SELECTION_FILTERS);
	DEFER({osdialog_filters_free(filters);});

	char* pathC = osdialog_file(OSDIALOG_OPEN, selectionDir.c_str(), NULL, filters);
	if (!pathC) {
		// No path selected
		return;
	}
	DEFER({std::free(pathC);});

	try {
		loadSelection(pathC);
	}
	catch (Exception& e) {
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, e.what());
	}
}

void RackWidget::saveSelection(std::string path) {
	INFO("Saving selection %s", path.c_str());

	json_t* rootJ = selectionToJson();
	assert(rootJ);
	DEFER({json_decref(rootJ);});

	engine::Module::jsonStripIds(rootJ);

	FILE* file = std::fopen(path.c_str(), "w");
	if (!file) {
		std::string message = string::f("Could not save selection to file %s", path.c_str());
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
		return;
	}
	DEFER({std::fclose(file);});

	json_dumpf(rootJ, file, JSON_INDENT(2));
}

void RackWidget::saveSelectionDialog() {
	std::string selectionDir = asset::user("selections");
	system::createDirectories(selectionDir);

	osdialog_filters* filters = osdialog_filters_parse(SELECTION_FILTERS);
	DEFER({osdialog_filters_free(filters);});

	char* pathC = osdialog_file(OSDIALOG_SAVE, selectionDir.c_str(), "Untitled.vcvs", filters);
	if (!pathC) {
		// No path selected
		return;
	}
	DEFER({std::free(pathC);});

	std::string path = pathC;
	// Automatically append .vcvs extension
	if (system::getExtension(path) != ".vcvs")
		path += ".vcvs";

	saveSelection(path);
}

void RackWidget::copyClipboardSelection() {
	json_t* rootJ = selectionToJson();
	DEFER({json_decref(rootJ);});
	char* moduleJson = json_dumps(rootJ, JSON_INDENT(2));
	DEFER({std::free(moduleJson);});
	glfwSetClipboardString(APP->window->win, moduleJson);
}

void RackWidget::resetSelectionAction() {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "reset modules";

	for (ModuleWidget* mw : getSelected()) {
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

void RackWidget::randomizeSelectionAction() {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "randomize modules";

	for (ModuleWidget* mw : getSelected()) {
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

void RackWidget::disconnectSelectionAction() {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "disconnect cables";

	for (ModuleWidget* mw : getSelected()) {
		mw->appendDisconnectActions(complexAction);
	}

	if (!complexAction->isEmpty())
		APP->history->push(complexAction);
	else
		delete complexAction;
}

void RackWidget::cloneSelectionAction(bool cloneCables) {
	json_t* rootJ = selectionToJson(cloneCables);
	DEFER({json_decref(rootJ);});

	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "duplicate modules";
	DEFER({
		if (!complexAction->isEmpty())
			APP->history->push(complexAction);
		else
			delete complexAction;
	});

	auto p = RackWidget_pasteJson(this, rootJ, complexAction);

	// Clone cables attached to inputs of selected modules but outputs of non-selected modules
	if (cloneCables) {
		for (CableWidget* cw : getCompleteCables()) {
			auto inputIt = p.newModuleIds.find(cw->getCable()->inputModule->id);
			if (inputIt == p.newModuleIds.end())
				continue;

			auto outputIt = p.newModuleIds.find(cw->getCable()->outputModule->id);
			if (outputIt != p.newModuleIds.end())
				continue;

			int64_t clonedInputModuleId = inputIt->second;
			engine::Module* clonedInputModule = APP->engine->getModule(clonedInputModuleId);

			// Create cable attached to cloned ModuleWidget's input
			engine::Cable* clonedCable = new engine::Cable;
			clonedCable->inputModule = clonedInputModule;
			clonedCable->inputId = cw->cable->inputId;
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
			complexAction->push(hca);
		}
	}
}

void RackWidget::bypassSelectionAction(bool bypassed) {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = bypassed ? "bypass modules" : "un-bypass modules";

	for (ModuleWidget* mw : getSelected()) {
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

bool RackWidget::isSelectionBypassed() {
	for (ModuleWidget* mw : getSelected()) {
		if (!mw->getModule()->isBypassed())
			return false;
	}
	return true;
}

void RackWidget::deleteSelectionAction() {
	history::ComplexAction* complexAction = new history::ComplexAction;
	complexAction->name = "remove modules";

	// Copy selected set since removing ModuleWidgets modifies it.
	std::set<ModuleWidget*> selectedModules = getSelected();
	for (ModuleWidget* mw : selectedModules) {
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

bool RackWidget::requestSelectionPos(math::Vec delta) {
	// Calculate new positions
	std::map<widget::Widget*, math::Rect> mwBoxes;
	for (ModuleWidget* mw : getSelected()) {
		math::Rect mwBox = mw->box;
		mwBox.pos += delta;
		mwBoxes[mw] = mwBox;
	}

	// Check intersection with other modules
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

void RackWidget::setSelectionPosNearest(math::Vec delta) {
	eachNearestGridPos(delta, [&](math::Vec delta) -> bool {
		return requestSelectionPos(delta);
	});
}

void RackWidget::appendSelectionContextMenu(ui::Menu* menu) {
	int n = getSelected().size();
	menu->addChild(createMenuLabel(string::f("%d selected %s", n, n == 1 ? "module" : "modules")));

	// Enable alwaysConsume of menu items if the number of selected modules changes

	// Select all
	menu->addChild(createMenuItem("Select all", RACK_MOD_CTRL_NAME "+A", [=]() {
		selectAll();
	}, false, true));

	// Deselect
	menu->addChild(createMenuItem("Deselect", RACK_MOD_CTRL_NAME "+" RACK_MOD_SHIFT_NAME "+A", [=]() {
		deselectAll();
	}, n == 0, true));

	// Copy
	menu->addChild(createMenuItem("Copy", RACK_MOD_CTRL_NAME "+C", [=]() {
		copyClipboardSelection();
	}, n == 0));

	// Paste
	menu->addChild(createMenuItem("Paste", RACK_MOD_CTRL_NAME "+V", [=]() {
		pasteClipboardAction();
	}, false, true));

	// Save
	menu->addChild(createMenuItem("Save selection as", "", [=]() {
		saveSelectionDialog();
	}, n == 0));

	// Initialize
	menu->addChild(createMenuItem("Initialize", RACK_MOD_CTRL_NAME "+I", [=]() {
		resetSelectionAction();
	}, n == 0));

	// Randomize
	menu->addChild(createMenuItem("Randomize", RACK_MOD_CTRL_NAME "+R", [=]() {
		randomizeSelectionAction();
	}, n == 0));

	// Disconnect cables
	menu->addChild(createMenuItem("Disconnect cables", RACK_MOD_CTRL_NAME "+U", [=]() {
		disconnectSelectionAction();
	}, n == 0));

	// Bypass
	std::string bypassText = RACK_MOD_CTRL_NAME "+E";
	bool bypassed = (n > 0) && isSelectionBypassed();
	if (bypassed)
		bypassText += " " CHECKMARK_STRING;
	menu->addChild(createMenuItem("Bypass", bypassText, [=]() {
		bypassSelectionAction(!bypassed);
	}, n == 0, true));

	// Duplicate
	menu->addChild(createMenuItem("Duplicate", RACK_MOD_CTRL_NAME "+D", [=]() {
		cloneSelectionAction(false);
	}, n == 0));

	// Duplicate with cables
	menu->addChild(createMenuItem("└ with cables", RACK_MOD_SHIFT_NAME "+" RACK_MOD_CTRL_NAME "+D", [=]() {
		cloneSelectionAction(true);
	}, n == 0));

	// Delete
	menu->addChild(createMenuItem("Delete", "Backspace/Delete", [=]() {
		deleteSelectionAction();
	}, n == 0, true));
}

void RackWidget::clearCables() {
	internal->incompleteCable = NULL;
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
		if (cw == internal->incompleteCable) {
			internal->incompleteCable = NULL;
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

CableWidget* RackWidget::getIncompleteCable() {
	return internal->incompleteCable;
}

void RackWidget::setIncompleteCable(CableWidget* cw) {
	if (internal->incompleteCable) {
		internal->cableContainer->removeChild(internal->incompleteCable);
		delete internal->incompleteCable;
		internal->incompleteCable = NULL;
	}
	if (cw) {
		internal->cableContainer->addChild(cw);
		internal->incompleteCable = cw;
	}
}

CableWidget* RackWidget::releaseIncompleteCable() {
	if (!internal->incompleteCable)
		return NULL;

	CableWidget* cw = internal->incompleteCable;
	internal->cableContainer->removeChild(internal->incompleteCable);
	internal->incompleteCable = NULL;
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

std::vector<CableWidget*> RackWidget::getCompleteCables() {
	std::vector<CableWidget*> cws;
	cws.reserve(internal->cableContainer->children.size());
	for (widget::Widget* w : internal->cableContainer->children) {
		CableWidget* cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (cw->isComplete())
			cws.push_back(cw);
	}
	cws.shrink_to_fit();
	return cws;
}

std::vector<CableWidget*> RackWidget::getCablesOnPort(PortWidget* port) {
	assert(port);
	std::vector<CableWidget*> cws;
	for (widget::Widget* w : internal->cableContainer->children) {
		CableWidget* cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (cw->inputPort == port || cw->outputPort == port) {
			cws.push_back(cw);
		}
	}
	return cws;
}

std::vector<CableWidget*> RackWidget::getCompleteCablesOnPort(PortWidget* port) {
	assert(port);
	std::vector<CableWidget*> cws;
	for (widget::Widget* w : internal->cableContainer->children) {
		CableWidget* cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (!cw->isComplete())
			continue;
		if (cw->inputPort == port || cw->outputPort == port) {
			cws.push_back(cw);
		}
	}
	return cws;
}


int RackWidget::getNextCableColorId() {
	return internal->nextCableColorId;
}


void RackWidget::setNextCableColorId(int id) {
	internal->nextCableColorId = id;
}


NVGcolor RackWidget::getNextCableColor() {
	if (settings::cableColors.empty())
		return color::WHITE;

	int id = internal->nextCableColorId++;
	if (id >= (int) settings::cableColors.size())
		id = 0;
	if (internal->nextCableColorId >= (int) settings::cableColors.size())
		internal->nextCableColorId = 0;
	return settings::cableColors[id];
}


ParamWidget* RackWidget::getTouchedParam() {
	return touchedParam;
}


void RackWidget::setTouchedParam(ParamWidget* pw) {
	touchedParam = pw;
}


} // namespace app
} // namespace rack
