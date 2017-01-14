#include "Rack.hpp"


namespace rack {

Port::Port() {
	box.size = Vec(20, 20);
	spriteOffset = Vec(-18, -18);
	spriteSize = Vec(56, 56);
	spriteFilename = "res/port.png";

	index = randomu32() % 5;
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

void Port::drawGlow(NVGcontext *vg) {
	Vec c = box.getCenter();
	NVGcolor icol = nvgRGBAf(1, 1, 1, 0.5);
	NVGcolor ocol = nvgRGBAf(1, 1, 1, 0);
	NVGpaint paint = nvgRadialGradient(vg, c.x, c.y, 0, 20, icol, ocol);
	nvgFillPaint(vg, paint);
	nvgBeginPath(vg);
	nvgRect(vg, box.pos.x - 10, box.pos.y - 10, box.size.x + 20, box.size.y + 20);
	nvgFill(vg);
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
