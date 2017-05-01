#include "app.hpp"


namespace rack {

Port::Port() {
	box.size = Vec(20, 20);
}

Port::~Port() {
	disconnect();
}

void Port::disconnect() {
	if (connectedWire) {
		gRackWidget->wireContainer->removeChild(connectedWire);
		// On destruction, Wire automatically sets connectedWire to NULL
		delete connectedWire;
	}
}

void Port::draw(NVGcontext *vg) {
	if (gRackWidget->activeWire) {
		// Dim the Port if the active wire cannot plug into this Port
		if (type == INPUT ? gRackWidget->activeWire->inputPort : gRackWidget->activeWire->outputPort)
			nvgGlobalAlpha(vg, 0.5);
	}
}

void Port::onMouseDown(int button) {
	if (button == 1) {
		disconnect();
	}
}

void Port::onDragEnd() {
	WireWidget *w = gRackWidget->activeWire;
	assert(w);
	w->updateWire();
	if (!w->wire) {
		gRackWidget->wireContainer->removeChild(w);
		delete w;
	}
	gRackWidget->activeWire = NULL;
}

void Port::onDragStart() {
	if (connectedWire) {
		// Disconnect wire from this port, but set it as the active wire
		if (type == INPUT)
			connectedWire->inputPort = NULL;
		else
			connectedWire->outputPort = NULL;
		connectedWire->updateWire();
		gRackWidget->activeWire = connectedWire;
		connectedWire = NULL;
	}
	else {
		connectedWire = new WireWidget();
		if (type == INPUT)
			connectedWire->inputPort = this;
		else
			connectedWire->outputPort = this;
		gRackWidget->wireContainer->addChild(connectedWire);
		gRackWidget->activeWire = connectedWire;
	}
}

void Port::onDragDrop(Widget *origin) {
	if (connectedWire) return;
	if (gRackWidget->activeWire) {
		if (type == INPUT) {
			gRackWidget->activeWire->hoveredInputPort = NULL;
			if (gRackWidget->activeWire->inputPort) return;
			gRackWidget->activeWire->inputPort = this;
		}
		else {
			gRackWidget->activeWire->hoveredOutputPort = NULL;
			if (gRackWidget->activeWire->outputPort) return;
			gRackWidget->activeWire->outputPort = this;
		}
		connectedWire = gRackWidget->activeWire;
	}
}

void Port::onDragEnter(Widget *origin) {
	if (connectedWire) return;
	if (gRackWidget->activeWire) {
		if (type == INPUT)
			gRackWidget->activeWire->hoveredInputPort = this;
		else
			gRackWidget->activeWire->hoveredOutputPort = this;
	}
}

void Port::onDragLeave(Widget *origin) {
	if (gRackWidget->activeWire) {
		if (type == INPUT)
			gRackWidget->activeWire->hoveredInputPort = NULL;
		else
			gRackWidget->activeWire->hoveredOutputPort = NULL;
	}
}


} // namespace rack
