#include "app.hpp"
#include "gui.hpp"


namespace rack {

Port::~Port() {
	gRackWidget->wireContainer->removeAllWires(this);
}

void Port::draw(NVGcontext *vg) {
	WireWidget *activeWire = gRackWidget->wireContainer->activeWire;
	if (activeWire) {
		// Dim the Port if the active wire cannot plug into this Port
		if (type == INPUT ? activeWire->inputPort : activeWire->outputPort)
			nvgGlobalAlpha(vg, 0.5);
	}
}

void Port::onMouseDownOpaque(int button) {
	if (button == 1) {
		gRackWidget->wireContainer->removeTopWire(this);

		// HACK
		// Update hovered*Port of active wire if applicable
		onDragEnter(NULL);
	}
}

void Port::onDragEnd() {
	gRackWidget->wireContainer->commitActiveWire();
}

void Port::onDragStart() {
	// Try to grab wire on top of stack
	WireWidget *wire = gRackWidget->wireContainer->getTopWire(this);

	if (wire && !guiIsModPressed()) {
		// Disconnect existing wire
		if (type == INPUT)
			wire->inputPort = NULL;
		else
			wire->outputPort = NULL;
		wire->updateWire();
	}
	else {
		// Create a new wire
		wire = new WireWidget();
		if (type == INPUT)
			wire->inputPort = this;
		else
			wire->outputPort = this;
	}
	gRackWidget->wireContainer->setActiveWire(wire);
}

void Port::onDragDrop(Widget *origin) {
}

void Port::onDragEnter(Widget *origin) {
	// Reject ports if this is an input port and something is already plugged into it
	if (type == INPUT) {
		WireWidget *topWire = gRackWidget->wireContainer->getTopWire(this);
		if (topWire)
			return;
	}

	WireWidget *activeWire = gRackWidget->wireContainer->activeWire;
	if (activeWire) {
		if (type == INPUT)
			activeWire->hoveredInputPort = this;
		else
			activeWire->hoveredOutputPort = this;
	}
}

void Port::onDragLeave(Widget *origin) {
	WireWidget *activeWire = gRackWidget->wireContainer->activeWire;
	if (activeWire) {
		if (type == INPUT)
			activeWire->hoveredInputPort = NULL;
		else
			activeWire->hoveredOutputPort = NULL;
	}
}


} // namespace rack
