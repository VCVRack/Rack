#include "scene.hpp"


namespace rack {

void OutputPort::onDragStart() {
	if (connectedWire) {
		// Disconnect wire from this port, but set it as the active wire
		connectedWire->outputPort = NULL;
		connectedWire->updateWire();
		gRackWidget->activeWire = connectedWire;
		connectedWire = NULL;
	}
	else {
		connectedWire = new WireWidget();
		connectedWire->outputPort = this;
		gRackWidget->wireContainer->addChild(connectedWire);
		gRackWidget->activeWire = connectedWire;
	}
}

void OutputPort::onDragDrop(Widget *origin) {
	if (connectedWire) return;
	if (gRackWidget->activeWire) {
		gRackWidget->activeWire->hoveredOutputPort = NULL;
		if (gRackWidget->activeWire->outputPort) return;
		gRackWidget->activeWire->outputPort = this;
		connectedWire = gRackWidget->activeWire;
	}
}

void OutputPort::onDragEnter(Widget *origin) {
	if (connectedWire) return;
	if (gRackWidget->activeWire) {
		gRackWidget->activeWire->hoveredOutputPort = this;
	}
}

void OutputPort::onDragLeave(Widget *origin) {
	if (gRackWidget->activeWire) {
		gRackWidget->activeWire->hoveredOutputPort = NULL;
	}
}

void OutputPort::draw(NVGcontext *vg) {
	if (gRackWidget->activeWire) {
		if (gRackWidget->activeWire->outputPort)
			nvgGlobalAlpha(vg, 0.5);
	}
	SpriteWidget::draw(vg);
}

} // namespace rack
