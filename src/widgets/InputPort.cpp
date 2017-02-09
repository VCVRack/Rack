#include "scene.hpp"


namespace rack {

void InputPort::onDragStart() {
	if (connectedWire) {
		// Disconnect wire from this port, but set it as the active wire
		connectedWire->inputPort = NULL;
		connectedWire->updateWire();
		gRackWidget->activeWire = connectedWire;
		connectedWire = NULL;
	}
	else {
		connectedWire = new WireWidget();
		connectedWire->inputPort = this;
		gRackWidget->wireContainer->addChild(connectedWire);
		gRackWidget->activeWire = connectedWire;
	}
}

void InputPort::onDragDrop(Widget *origin) {
	if (connectedWire) return;
	if (gRackWidget->activeWire) {
		gRackWidget->activeWire->hoveredInputPort = NULL;
		if (gRackWidget->activeWire->inputPort) return;
		gRackWidget->activeWire->inputPort = this;
		connectedWire = gRackWidget->activeWire;
	}
}

void InputPort::onDragEnter(Widget *origin) {
	if (connectedWire) return;
	if (gRackWidget->activeWire) {
		gRackWidget->activeWire->hoveredInputPort = this;
	}
}

void InputPort::onDragLeave(Widget *origin) {
	if (gRackWidget->activeWire) {
		gRackWidget->activeWire->hoveredInputPort = NULL;
	}
}

} // namespace rack
