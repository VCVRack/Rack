#include "scene.hpp"
#include "engine.hpp"


namespace rack {

static void drawPlug(NVGcontext *vg, Vec pos, NVGcolor color) {
	NVGcolor colorOutline = nvgLerpRGBA(color, nvgRGBf(0.0, 0.0, 0.0), 0.5);

	// Plug solid
	nvgBeginPath(vg);
	nvgCircle(vg, pos.x, pos.y, 9.5);
	nvgFillColor(vg, color);
	nvgFill(vg);

	// Border
	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, colorOutline);
	nvgStroke(vg);

	// Hole
	nvgBeginPath(vg);
	nvgCircle(vg, pos.x, pos.y, 5.5);
	nvgFillColor(vg, nvgRGBf(0.0, 0.0, 0.0));
	nvgFill(vg);
}

static void drawWire(NVGcontext *vg, Vec pos1, Vec pos2, NVGcolor color, float tension, float opacity) {
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
		nvgStrokeWidth(vg, 5);
		nvgStroke(vg);

		// Wire solid
		nvgStrokeColor(vg, color);
		nvgStrokeWidth(vg, 3);
		nvgStroke(vg);

		nvgRestore(vg);
	}
}


static NVGcolor wireColors[8] = {
	nvgRGB(0xc9, 0xb7, 0x0e), // yellow
	nvgRGB(0xc9, 0x18, 0x47), // red
	nvgRGB(0x0c, 0x8e, 0x15), // green
	nvgRGB(0x09, 0x86, 0xad), // blue
	nvgRGB(0x44, 0x44, 0x44), // black
	nvgRGB(0x66, 0x66, 0x66), // gray
	nvgRGB(0x88, 0x88, 0x88), // light gray
	nvgRGB(0xaa, 0xaa, 0xaa), // white
};
static int lastWireColorId = -1;


WireWidget::WireWidget() {
	int wireColorId;
	do {
		wireColorId = randomu32() % 8;
	} while (wireColorId == lastWireColorId);
	lastWireColorId = wireColorId;

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
	Vec absolutePos = getAbsolutePos();
	float opacity = dynamic_cast<RackScene*>(gScene)->toolbar->wireOpacitySlider->value / 100.0;
	float tension = dynamic_cast<RackScene*>(gScene)->toolbar->wireTensionSlider->value;

	// Display the actively dragged wire as opaque
	if (gRackWidget->activeWire == this)
		opacity = 1.0;

	// Compute location of outputPos and inputPos
	Vec outputPos;
	if (outputPort) {
		outputPos = Rect(outputPort->getAbsolutePos(), outputPort->box.size).getCenter();
	}
	else if (hoveredOutputPort) {
		outputPos = Rect(hoveredOutputPort->getAbsolutePos(), hoveredOutputPort->box.size).getCenter();
	}
	else {
		outputPos = gMousePos;
	}

	Vec inputPos;
	if (inputPort) {
		inputPos = Rect(inputPort->getAbsolutePos(), inputPort->box.size).getCenter();
	}
	else if (hoveredInputPort) {
		inputPos = Rect(hoveredInputPort->getAbsolutePos(), hoveredInputPort->box.size).getCenter();
	}
	else {
		inputPos = gMousePos;
	}

	outputPos = outputPos.minus(absolutePos);
	inputPos = inputPos.minus(absolutePos);

	drawWire(vg, outputPos, inputPos, color, tension, opacity);
	drawPlug(vg, outputPos, color);
	drawPlug(vg, inputPos, color);
}



} // namespace rack
