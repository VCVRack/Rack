#include "scene.hpp"


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


} // namespace rack
