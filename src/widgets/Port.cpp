#include "../5V.hpp"


Port::Port() {
	box.size = Vec(20, 20);
	type = rand() % 5;
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
	Vec pos = box.pos.plus(Vec(-18, -18));
	int width, height;
	int imageId = loadImage("res/port.png");
	nvgImageSize(vg, imageId, &width, &height);
	float offsetY = type * width;
	NVGpaint paint = nvgImagePattern(vg, pos.x, pos.y - offsetY, width, height, 0.0, imageId, 1.0);
	nvgFillPaint(vg, paint);
	nvgBeginPath(vg);
	nvgRect(vg, pos.x, pos.y, width, width);
	nvgFill(vg);
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
