#include "app/CableContainer.hpp"

namespace rack {


void CableContainer::setActiveCable(CableWidget *w) {
	if (activeCable) {
		removeChild(activeCable);
		delete activeCable;
		activeCable = NULL;
	}
	if (w) {
		if (w->parent == NULL)
			addChild(w);
		activeCable = w;
	}
}

void CableContainer::commitActiveCable() {
	if (!activeCable)
		return;

	if (activeCable->hoveredOutputPort) {
		activeCable->outputPort = activeCable->hoveredOutputPort;
		activeCable->hoveredOutputPort = NULL;
	}
	if (activeCable->hoveredInputPort) {
		activeCable->inputPort = activeCable->hoveredInputPort;
		activeCable->hoveredInputPort = NULL;
	}
	activeCable->updateCable();

	// Did it successfully connect?
	if (activeCable->cable) {
		// Make it permanent
		activeCable = NULL;
	}
	else {
		// Remove it
		setActiveCable(NULL);
	}
}

void CableContainer::removeTopCable(PortWidget *port) {
	CableWidget *cable = getTopCable(port);
	if (cable) {
		removeChild(cable);
		delete cable;
	}
}

void CableContainer::removeAllCables(PortWidget *port) {
	// As a convenience, de-hover the active cable so we don't attach them once it is dropped.
	if (activeCable) {
		if (activeCable->hoveredInputPort == port)
			activeCable->hoveredInputPort = NULL;
		if (activeCable->hoveredOutputPort == port)
			activeCable->hoveredOutputPort = NULL;
	}

	// Build a list of CableWidgets to delete
	std::list<CableWidget*> cables;

	for (Widget *child : children) {
		CableWidget *cable = dynamic_cast<CableWidget*>(child);
		assert(cable);
		if (!cable || cable->inputPort == port || cable->outputPort == port) {
			if (activeCable == cable) {
				activeCable = NULL;
			}
			// We can't delete from this list while we're iterating it, so add it to the deletion list.
			cables.push_back(cable);
		}
	}

	// Once we're done building the list, actually delete them
	for (CableWidget *cable : cables) {
		removeChild(cable);
		delete cable;
	}
}

CableWidget *CableContainer::getTopCable(PortWidget *port) {
	for (auto it = children.rbegin(); it != children.rend(); it++) {
		CableWidget *cable = dynamic_cast<CableWidget*>(*it);
		assert(cable);
		// Ignore incomplete cables
		if (!(cable->inputPort && cable->outputPort))
			continue;
		if (cable->inputPort == port || cable->outputPort == port)
			return cable;
	}
	return NULL;
}

void CableContainer::draw(NVGcontext *vg) {
	Widget::draw(vg);

	// Cable plugs
	for (Widget *child : children) {
		CableWidget *cable = dynamic_cast<CableWidget*>(child);
		assert(cable);
		cable->drawPlugs(vg);
	}
}


} // namespace rack
