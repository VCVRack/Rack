#include "../5V.hpp"


void InputPort::draw(NVGcontext *vg) {
	Port::draw(vg);
	if (gRackWidget->activeWire && gRackWidget->activeWire->inputPort) {
		Port::drawGlow(vg);
	}
}

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
		if (gRackWidget->activeWire->inputPort) return;
		gRackWidget->activeWire->inputPort = this;
		connectedWire = gRackWidget->activeWire;
	}
}
