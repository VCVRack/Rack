#include "app/RackWidget.hpp"
#include "widget/TransparentWidget.hpp"
#include "app/RackRail.hpp"
#include "app/Scene.hpp"
#include "app/ModuleBrowser.hpp"
#include "settings.hpp"
#include "plugin.hpp"
#include "engine/Engine.hpp"
#include "app.hpp"
#include "asset.hpp"
#include "patch.hpp"
#include "osdialog.h"
#include <map>
#include <algorithm>


namespace rack {
namespace app {


static ModuleWidget *moduleFromJson(json_t *moduleJ) {
	// Get slugs
	json_t *pluginSlugJ = json_object_get(moduleJ, "plugin");
	if (!pluginSlugJ)
		return NULL;
	json_t *modelSlugJ = json_object_get(moduleJ, "model");
	if (!modelSlugJ)
		return NULL;
	std::string pluginSlug = json_string_value(pluginSlugJ);
	std::string modelSlug = json_string_value(modelSlugJ);

	// Get Model
	plugin::Model *model = plugin::getModel(pluginSlug, modelSlug);
	if (!model)
		return NULL;

	// Create ModuleWidget
	ModuleWidget *moduleWidget = model->createModuleWidget();
	assert(moduleWidget);
	moduleWidget->fromJson(moduleJ);
	return moduleWidget;
}


struct ModuleContainer : widget::Widget {
	void draw(const DrawArgs &args) override {
		// Draw shadows behind each ModuleWidget first, so the shadow doesn't overlap the front of other ModuleWidgets.
		for (widget::Widget *child : children) {
			ModuleWidget *w = dynamic_cast<ModuleWidget*>(child);
			assert(w);

			nvgSave(args.vg);
			nvgTranslate(args.vg, child->box.pos.x, child->box.pos.y);
			w->drawShadow(args);
			nvgRestore(args.vg);
		}

		widget::Widget::draw(args);
	}
};


struct CableContainer : widget::TransparentWidget {
	void draw(const DrawArgs &args) override {
		widget::Widget::draw(args);

		// Draw cable plugs
		for (widget::Widget *w : children) {
			CableWidget *cw = dynamic_cast<CableWidget*>(w);
			assert(cw);
			cw->drawPlugs(args);
		}
	}
};


RackWidget::RackWidget() {
	rails = new widget::FramebufferWidget;
	rails->box.size = math::Vec();
	rails->oversample = 1.0;
	{
		RackRail *rail = new RackRail;
		rail->box.size = math::Vec();
		rails->addChild(rail);
	}
	addChild(rails);

	moduleContainer = new ModuleContainer;
	addChild(moduleContainer);

	cableContainer = new CableContainer;
	addChild(cableContainer);
}

RackWidget::~RackWidget() {
	clear();
}

void RackWidget::step() {
	// Expand size to fit modules
	math::Vec moduleSize = moduleContainer->getChildrenBoundingBox().getBottomRight();
	// We assume that the size is reset by a parent before calling step(). Otherwise it will grow unbounded.
	box.size = box.size.max(moduleSize);

	// Adjust size and position of rails
	widget::Widget *rail = rails->children.front();
	math::Rect bound = getViewport(math::Rect(math::Vec(), box.size));
	if (!rails->box.isContaining(bound)) {
		math::Vec cellMargin = math::Vec(20, 1);
		rails->box.pos = bound.pos.div(RACK_GRID_SIZE).floor().minus(cellMargin).mult(RACK_GRID_SIZE);
		rails->box.size = bound.size.plus(cellMargin.mult(RACK_GRID_SIZE).mult(2));
		rails->dirty = true;

		rail->box.size = rails->box.size;
	}

	widget::Widget::step();
}

void RackWidget::draw(const DrawArgs &args) {
	widget::Widget::draw(args);
}

void RackWidget::onHover(const event::Hover &e) {
	if (!APP->event->selectedWidget) {
		// Scroll with arrow keys
		float arrowSpeed = 30.0;
		if ((APP->window->getMods() & WINDOW_MOD_MASK) == (WINDOW_MOD_CTRL |GLFW_MOD_SHIFT))
			arrowSpeed /= 16.0;
		else if ((APP->window->getMods() & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL)
			arrowSpeed *= 4.0;
		else if ((APP->window->getMods() & WINDOW_MOD_MASK) == GLFW_MOD_SHIFT)
			arrowSpeed /= 4.0;

		ui::ScrollWidget *scrollWidget = APP->scene->scrollWidget;
		if (glfwGetKey(APP->window->win, GLFW_KEY_LEFT) == GLFW_PRESS) {
			scrollWidget->offset.x -= arrowSpeed;
		}
		if (glfwGetKey(APP->window->win, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			scrollWidget->offset.x += arrowSpeed;
		}
		if (glfwGetKey(APP->window->win, GLFW_KEY_UP) == GLFW_PRESS) {
			scrollWidget->offset.y -= arrowSpeed;
		}
		if (glfwGetKey(APP->window->win, GLFW_KEY_DOWN) == GLFW_PRESS) {
			scrollWidget->offset.y += arrowSpeed;
		}
	}

	widget::OpaqueWidget::onHover(e);
	mousePos = e.pos;
}

void RackWidget::onHoverKey(const event::HoverKey &e) {
	widget::OpaqueWidget::onHoverKey(e);
	if (e.getConsumed() != this)
		return;

	if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		switch (e.key) {
			case GLFW_KEY_V: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					pastePresetClipboardAction();
				}
			} break;
		}
	}
}

void RackWidget::onDragHover(const event::DragHover &e) {
	widget::OpaqueWidget::onDragHover(e);
	mousePos = e.pos;
}

void RackWidget::onButton(const event::Button &e) {
	widget::OpaqueWidget::onButton(e);
	if (e.getConsumed() == this) {
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
			APP->scene->moduleBrowser->visible = true;
		}
	}
}

void RackWidget::onZoom(const event::Zoom &e) {
	rails->box.size = math::Vec();
	widget::OpaqueWidget::onZoom(e);
}

void RackWidget::clear() {
	// This isn't required because removing all ModuleWidgets should remove all cables, but do it just in case.
	clearCables();
	// Remove ModuleWidgets
	std::list<widget::Widget*> widgets = moduleContainer->children;
	for (widget::Widget *w : widgets) {
		ModuleWidget *moduleWidget = dynamic_cast<ModuleWidget*>(w);
		assert(moduleWidget);
		removeModule(moduleWidget);
		delete moduleWidget;
	}
}

json_t *RackWidget::toJson() {
	// root
	json_t *rootJ = json_object();

	// modules
	json_t *modulesJ = json_array();
	for (widget::Widget *w : moduleContainer->children) {
		ModuleWidget *moduleWidget = dynamic_cast<ModuleWidget*>(w);
		assert(moduleWidget);
		// module
		json_t *moduleJ = moduleWidget->toJson();
		{
			// id
			json_object_set_new(moduleJ, "id", json_integer(moduleWidget->module->id));
			// pos
			math::Vec pos = moduleWidget->box.pos.div(RACK_GRID_SIZE).round();
			json_t *posJ = json_pack("[i, i]", (int) pos.x, (int) pos.y);
			json_object_set_new(moduleJ, "pos", posJ);
		}
		json_array_append_new(modulesJ, moduleJ);
	}
	json_object_set_new(rootJ, "modules", modulesJ);

	// cables
	json_t *cablesJ = json_array();
	for (widget::Widget *w : cableContainer->children) {
		CableWidget *cw = dynamic_cast<CableWidget*>(w);
		assert(cw);

		// Only serialize complete cables
		if (!cw->isComplete())
			continue;

		json_t *cableJ = cw->toJson();
		{
			// id
			json_object_set_new(rootJ, "id", json_integer(cw->cable->id));
		}
		json_array_append_new(cablesJ, cableJ);

	}
	json_object_set_new(rootJ, "cables", cablesJ);

	return rootJ;
}

void RackWidget::fromJson(json_t *rootJ) {
	// modules
	json_t *modulesJ = json_object_get(rootJ, "modules");
	if (!modulesJ)
		return;
	size_t moduleIndex;
	json_t *moduleJ;
	json_array_foreach(modulesJ, moduleIndex, moduleJ) {
		ModuleWidget *moduleWidget = moduleFromJson(moduleJ);

		if (moduleWidget) {
			// Before 1.0, the module ID was the index in the "modules" array
			if (APP->patch->isLegacy(2)) {
				moduleWidget->module->id = moduleIndex;
			}
			// id
			json_t *idJ = json_object_get(moduleJ, "id");
			if (idJ)
				moduleWidget->module->id = json_integer_value(idJ);

			// pos
			json_t *posJ = json_object_get(moduleJ, "pos");
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

			addModule(moduleWidget);
		}
		else {
			json_t *pluginSlugJ = json_object_get(moduleJ, "plugin");
			json_t *modelSlugJ = json_object_get(moduleJ, "model");
			std::string pluginSlug = json_string_value(pluginSlugJ);
			std::string modelSlug = json_string_value(modelSlugJ);
			APP->patch->warningLog += string::f("Could not find module \"%s\" of plugin \"%s\"\n", modelSlug.c_str(), pluginSlug.c_str());
		}
	}

	// cables
	json_t *cablesJ = json_object_get(rootJ, "cables");
	// Before 1.0, cables were called wires
	if (!cablesJ)
		cablesJ = json_object_get(rootJ, "wires");
	assert(cablesJ);
	size_t cableIndex;
	json_t *cableJ;
	json_array_foreach(cablesJ, cableIndex, cableJ) {
		// Create a unserialize cable
		CableWidget *cw = new CableWidget;
		cw->fromJson(cableJ);
		if (!cw->isComplete()) {
			delete cw;
			continue;
		}

		// Before 1.0, cables IDs were not used, so just use the index of the "cables" array.
		if (APP->patch->isLegacy(2)) {
			cw->cable->id = cableIndex;
		}
		// id
		json_t *idJ = json_object_get(cableJ, "id");
		if (idJ)
			cw->cable->id = json_integer_value(idJ);
		addCable(cw);
	}
}

void RackWidget::pastePresetClipboardAction() {
	const char *moduleJson = glfwGetClipboardString(APP->window->win);
	if (!moduleJson) {
		WARN("Could not get text from clipboard.");
		return;
	}

	json_error_t error;
	json_t *moduleJ = json_loads(moduleJson, 0, &error);
	if (moduleJ) {
		ModuleWidget *mw = moduleFromJson(moduleJ);
		json_decref(moduleJ);
		addModuleAtMouse(mw);

		// history::ModuleAdd
		history::ModuleAdd *h = new history::ModuleAdd;
		h->setModule(mw);
		APP->history->push(h);
	}
	else {
		WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
	}
}

void RackWidget::addModule(ModuleWidget *m) {
	// Add module to ModuleContainer
	assert(m);
	moduleContainer->addChild(m);

	if (m->module) {
		// Add module to Engine
		APP->engine->addModule(m->module);
	}
}

void RackWidget::addModuleAtMouse(ModuleWidget *m) {
	assert(m);
	// Move module nearest to the mouse position
	m->box.pos = mousePos.minus(m->box.size.div(2));
	requestModuleBoxNearest(m, m->box);
	addModule(m);
}

void RackWidget::removeModule(ModuleWidget *m) {
	// Unset touchedParamWidget
	if (touchedParam) {
		ModuleWidget *touchedModule = touchedParam->getAncestorOfType<ModuleWidget>();
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

bool RackWidget::requestModuleBox(ModuleWidget *m, math::Rect requestedBox) {
	// Check bounds
	if (requestedBox.pos.x < 0 || requestedBox.pos.y < 0)
		return false;

	// Check intersection with other modules
	for (widget::Widget *m2 : moduleContainer->children) {
		// Don't intersect with self
		if (m == m2)
			continue;
		if (requestedBox.isIntersecting(m2->box)) {
			return false;
		}
	}

	// Accept requested position
	m->box = requestedBox;
	return true;
}

bool RackWidget::requestModuleBoxNearest(ModuleWidget *m, math::Rect requestedBox) {
	// Create possible positions
	int x0 = std::round(requestedBox.pos.x / RACK_GRID_WIDTH);
	int y0 = std::round(requestedBox.pos.y / RACK_GRID_HEIGHT);
	std::vector<math::Vec> positions;
	for (int y = std::max(0, y0 - 8); y < y0 + 8; y++) {
		for (int x = std::max(0, x0 - 400); x < x0 + 400; x++) {
			positions.push_back(math::Vec(x * RACK_GRID_WIDTH, y * RACK_GRID_HEIGHT));
		}
	}

	// Sort possible positions by distance to the requested position
	std::sort(positions.begin(), positions.end(), [requestedBox](math::Vec a, math::Vec b) {
		return a.minus(requestedBox.pos).norm() < b.minus(requestedBox.pos).norm();
	});

	// Find a position that does not collide
	for (math::Vec position : positions) {
		math::Rect newBox = requestedBox;
		newBox.pos = position;
		if (requestModuleBox(m, newBox))
			return true;
	}
	// We failed to find a box with this brute force algorithm.
	return false;
}

ModuleWidget *RackWidget::getModule(int moduleId) {
	for (widget::Widget *w : moduleContainer->children) {
		ModuleWidget *moduleWidget = dynamic_cast<ModuleWidget*>(w);
		assert(moduleWidget);
		if (moduleWidget->module->id == moduleId)
			return moduleWidget;
	}
	return NULL;
}

bool RackWidget::isEmpty() {
	return moduleContainer->children.empty();
}

void RackWidget::clearCables() {
	for (widget::Widget *w : cableContainer->children) {
		CableWidget *cw = dynamic_cast<CableWidget*>(w);
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
	history::ComplexAction *complexAction = new history::ComplexAction;
	complexAction->name = "clear cables";

	for (widget::Widget *w : cableContainer->children) {
		CableWidget *cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (!cw->isComplete())
			continue;

		// history::CableRemove
		history::CableRemove *h = new history::CableRemove;
		h->setCable(cw);
		complexAction->push(h);
	}

	APP->history->push(complexAction);
	clearCables();
}

void RackWidget::clearCablesOnPort(PortWidget *port) {
	for (CableWidget *cw : getCablesOnPort(port)) {
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

void RackWidget::addCable(CableWidget *w) {
	assert(w->isComplete());
	APP->engine->addCable(w->cable);
	cableContainer->addChild(w);
}

void RackWidget::removeCable(CableWidget *w) {
	assert(w->isComplete());
	APP->engine->removeCable(w->cable);
	cableContainer->removeChild(w);
}

void RackWidget::setIncompleteCable(CableWidget *w) {
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

CableWidget *RackWidget::releaseIncompleteCable() {
	CableWidget *cw = incompleteCable;
	cableContainer->removeChild(incompleteCable);
	incompleteCable = NULL;
	return cw;
}

CableWidget *RackWidget::getTopCable(PortWidget *port) {
	for (auto it = cableContainer->children.rbegin(); it != cableContainer->children.rend(); it++) {
		CableWidget *cw = dynamic_cast<CableWidget*>(*it);
		assert(cw);
		// Ignore incomplete cables
		if (!cw->isComplete())
			continue;
		if (cw->inputPort == port || cw->outputPort == port)
			return cw;
	}
	return NULL;
}

CableWidget *RackWidget::getCable(int cableId) {
	for (widget::Widget *w : cableContainer->children) {
		CableWidget *cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (cw->cable->id == cableId)
			return cw;
	}
	return NULL;
}

std::list<CableWidget*> RackWidget::getCablesOnPort(PortWidget *port) {
	assert(port);
	std::list<CableWidget*> cables;
	for (widget::Widget *w : cableContainer->children) {
		CableWidget *cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (cw->inputPort == port || cw->outputPort == port) {
			cables.push_back(cw);
		}
	}
	return cables;
}


} // namespace app
} // namespace rack
