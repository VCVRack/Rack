#include "app/CableContainer.hpp"
#include "app.hpp"
#include "engine/Engine.hpp"


namespace rack {


CableContainer::~CableContainer() {
	clear();
}

void CableContainer::clear() {
	for (Widget *w : children) {
		CableWidget *cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (cw != incompleteCable)
			app()->engine->removeCable(cw->cable);
	}
	incompleteCable = NULL;
	clearChildren();
}

void CableContainer::clearPort(PortWidget *port) {
	assert(port);
	std::list<Widget*> childrenCopy = children;
	for (Widget *w : childrenCopy) {
		CableWidget *cw = dynamic_cast<CableWidget*>(w);
		assert(cw);

		// Check if cable is connected to port
		if (cw->inputPort == port || cw->outputPort == port) {
			if (cw == incompleteCable) {
				incompleteCable = NULL;
				removeChild(cw);
			}
			else {
				removeCable(cw);
			}
			delete cw;
		}
	}
}

void CableContainer::addCable(CableWidget *w) {
	assert(w->isComplete());
	app()->engine->addCable(w->cable);
	addChild(w);
}

void CableContainer::removeCable(CableWidget *w) {
	assert(w->isComplete());
	app()->engine->removeCable(w->cable);
	removeChild(w);
}

void CableContainer::setIncompleteCable(CableWidget *w) {
	if (incompleteCable) {
		removeChild(incompleteCable);
		delete incompleteCable;
		incompleteCable = NULL;
	}
	if (w) {
		addChild(w);
		incompleteCable = w;
	}
}

CableWidget *CableContainer::releaseIncompleteCable() {
	CableWidget *cw = incompleteCable;
	removeChild(incompleteCable);
	incompleteCable = NULL;
	return cw;
}

CableWidget *CableContainer::getTopCable(PortWidget *port) {
	for (auto it = children.rbegin(); it != children.rend(); it++) {
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

CableWidget *CableContainer::getCable(int cableId) {
	for (Widget *w : children) {
		CableWidget *cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		if (cw->cable->id == cableId)
			return cw;
	}
	return NULL;
}

json_t *CableContainer::toJson() {
	json_t *rootJ = json_array();
	for (Widget *w : children) {
		CableWidget *cw = dynamic_cast<CableWidget*>(w);
		assert(cw);

		// Only serialize complete cables
		if (!cw->isComplete())
			continue;

		json_array_append_new(rootJ, cw->toJson());
	}
	return rootJ;
}

void CableContainer::fromJson(json_t *rootJ, const std::map<int, ModuleWidget*> &moduleWidgets) {
	size_t cableIndex;
	json_t *cableJ;
	json_array_foreach(rootJ, cableIndex, cableJ) {
		// Create a unserialize cable
		CableWidget *cw = new CableWidget;
		cw->fromJson(cableJ, moduleWidgets);
		if (!cw->isComplete()) {
			delete cw;
			continue;
		}
		addCable(cw);
	}
}

void CableContainer::draw(NVGcontext *vg) {
	Widget::draw(vg);

	// Cable plugs
	for (Widget *w : children) {
		CableWidget *cw = dynamic_cast<CableWidget*>(w);
		assert(cw);
		cw->drawPlugs(vg);
	}
}


} // namespace rack
