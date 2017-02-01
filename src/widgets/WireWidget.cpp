#include "scene.hpp"
#include "engine.hpp"


namespace rack {

void drawWire(NVGcontext *vg, Vec pos1, Vec pos2, float tension, NVGcolor color, float opacity) {
	NVGcolor colorShadow = nvgRGBAf(0, 0, 0, 0.08);
	NVGcolor colorOutline = nvgLerpRGBA(color, nvgRGBf(0.0, 0.0, 0.0), 0.5);


	// Wire
	if (opacity > 0.0) {
		nvgSave(vg);
		nvgGlobalAlpha(vg, opacity);

		float dist = pos1.minus(pos2).norm();
		Vec slump;
		slump.y = (1.0 - tension) * (150.0 + 1.0*dist);
		Vec pos3 = pos1.plus(pos2).div(2).plus(slump);

		nvgLineJoin(vg, NVG_ROUND);

		// Shadow
		Vec pos4 = pos3.plus(slump.mult(0.08));
		nvgBeginPath(vg);
		nvgMoveTo(vg, pos1.x, pos1.y);
		nvgQuadTo(vg, pos4.x, pos4.y, pos2.x, pos2.y);
		nvgStrokeColor(vg, colorShadow);
		nvgStrokeWidth(vg, 5);
		nvgStroke(vg);

		// Wire outline
		nvgBeginPath(vg);
		nvgMoveTo(vg, pos1.x, pos1.y);
		nvgQuadTo(vg, pos3.x, pos3.y, pos2.x, pos2.y);
		nvgStrokeColor(vg, colorOutline);
		nvgStrokeWidth(vg, 6);
		nvgStroke(vg);

		// Wire solid
		nvgStrokeColor(vg, color);
		nvgStrokeWidth(vg, 4);
		nvgStroke(vg);

		nvgRestore(vg);
	}

	// First plug
	nvgBeginPath(vg);
	nvgCircle(vg, pos1.x, pos1.y, 21/2.0);
	nvgFillColor(vg, colorOutline);
	nvgFill(vg);

	nvgBeginPath(vg);
	nvgCircle(vg, pos1.x, pos1.y, 19/2.0);
	nvgFillColor(vg, color);
	nvgFill(vg);

	nvgBeginPath(vg);
	nvgCircle(vg, pos1.x, pos1.y, 11/2.0);
	nvgFillColor(vg, nvgRGBf(0.0, 0.0, 0.0));
	nvgFill(vg);

	// Second plug
	nvgBeginPath(vg);
	nvgCircle(vg, pos2.x, pos2.y, 21/2.0);
	nvgFillColor(vg, colorOutline);
	nvgFill(vg);

	nvgBeginPath(vg);
	nvgCircle(vg, pos2.x, pos2.y, 19/2.0);
	nvgFillColor(vg, color);
	nvgFill(vg);

	nvgBeginPath(vg);
	nvgCircle(vg, pos2.x, pos2.y, 11/2.0);
	nvgFillColor(vg, nvgRGBf(0.0, 0.0, 0.0));
	nvgFill(vg);
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
		engineRemoveWire(wire);
		delete wire;
		wire = NULL;
	}
	if (inputPort && outputPort) {
		wire = new Wire();
		wire->outputModule = outputPort->module;
		wire->outputId = outputPort->outputId;
		wire->inputModule = inputPort->module;
		wire->inputId = inputPort->inputId;
		engineAddWire(wire);
	}
}

void WireWidget::draw(NVGcontext *vg) {
	Vec outputPos, inputPos;
	Vec absolutePos = getAbsolutePos();
	float opacity = dynamic_cast<RackScene*>(gScene)->toolbar->wireOpacitySlider->value / 100.0;
	float tension = dynamic_cast<RackScene*>(gScene)->toolbar->wireTensionSlider->value;

	// Compute location of pos1 and pos2
	if (outputPort) {
		outputPos = Rect(outputPort->getAbsolutePos(), outputPort->box.size).getCenter();
	}
	else {
		outputPos = gMousePos;
		opacity = 1.0;
	}
	if (inputPort) {
		inputPos = Rect(inputPort->getAbsolutePos(), inputPort->box.size).getCenter();
	}
	else {
		inputPos = gMousePos;
		opacity = 1.0;
	}

	outputPos = outputPos.minus(absolutePos);
	inputPos = inputPos.minus(absolutePos);

	drawWire(vg, outputPos, inputPos, tension, color, opacity);
}


} // namespace rack
