#include "../5V.hpp"


void drawWire(NVGcontext *vg, Vec pos1, Vec pos2, NVGcolor color) {
	float dist = pos1.minus(pos2).norm();
	Vec slump = Vec(0, 100.0 + 0.5*dist);
	Vec pos3 = pos1.plus(pos2).div(2).plus(slump);

	NVGcolor colorOutline = nvgRGBf(0, 0, 0);

	nvgLineJoin(vg, NVG_ROUND);
	nvgStrokeWidth(vg, 4);
	// Shadow
	// Vec pos4 = pos3.plus(slump.mult(0.1));
	// NVGcolor colorShadow = nvgRGBAf(0, 0, 0, 0.2);
	// nvgBeginPath(vg);
	// nvgMoveTo(vg, pos1.x, pos1.y);
	// nvgQuadTo(vg, pos4.x, pos4.y, pos2.x, pos2.y);
	// nvgStrokeColor(vg, colorShadow);
	// nvgStroke(vg);

	// Wire outline
	nvgBeginPath(vg);
	nvgMoveTo(vg, pos1.x, pos1.y);
	nvgQuadTo(vg, pos3.x, pos3.y, pos2.x, pos2.y);
	nvgStrokeColor(vg, colorOutline);
	nvgStroke(vg);

	// Wire solid
	nvgStrokeWidth(vg, 2);
	nvgStrokeColor(vg, color);
	nvgStroke(vg);
}


static NVGcolor wireColors[8] = {
	nvgRGB(0x50, 0x50, 0x50),
	nvgRGB(0xac, 0x41, 0x42),
	nvgRGB(0x90, 0xa9, 0x59),
	nvgRGB(0xf4, 0xbf, 0x75),
	nvgRGB(0x6a, 0x9f, 0xb5),
	nvgRGB(0xaa, 0x75, 0x9f),
	nvgRGB(0x75, 0xb5, 0xaa),
	nvgRGB(0xf5, 0xf5, 0xf5),
};
static int wireColorId = 1;



WireWidget::WireWidget() {
	wireColorId = (wireColorId + 1) % 8;
	color = wireColors[wireColorId];
}

WireWidget::~WireWidget() {
	if (outputPort) {
		outputPort->connectedWire = NULL;
		outputPort = NULL;
	}
	if (inputPort) {
		inputPort->connectedWire = NULL;
		inputPort = NULL;
	}
	updateWire();
}

void WireWidget::updateWire() {
	if (wire) {
		rackDisconnectWire(wire);
		delete wire;
		wire = NULL;
	}
	if (inputPort && outputPort) {
		wire = new Wire();
		wire->outputModule = outputPort->moduleWidget->module;
		wire->outputId = outputPort->outputId;
		wire->inputModule = inputPort->moduleWidget->module;
		wire->inputId = inputPort->inputId;
		rackConnectWire(wire);
	}
}

void WireWidget::draw(NVGcontext *vg) {
	Vec outputPos, inputPos;
	Vec absolutePos = getAbsolutePos();

	if (outputPort) {
		outputPos = Rect(outputPort->getAbsolutePos(), outputPort->box.size).getCenter();
	}
	else {
		outputPos = gMousePos;
	}
	if (inputPort) {
		inputPos = Rect(inputPort->getAbsolutePos(), inputPort->box.size).getCenter();
	}
	else {
		inputPos = gMousePos;
	}

	outputPos = outputPos.minus(absolutePos);
	inputPos = inputPos.minus(absolutePos);

	bndNodePort(vg, outputPos.x, outputPos.y, BND_DEFAULT, color);
	bndNodePort(vg, inputPos.x, inputPos.y, BND_DEFAULT, color);
	nvgSave(vg);
	float wireOpacity = gScene->toolbar->wireOpacitySlider->value / 100.0;
	if (wireOpacity > 0.0) {
		nvgGlobalAlpha(vg, wireOpacity);
		drawWire(vg, outputPos, inputPos, color);
	}
	nvgRestore(vg);
}
