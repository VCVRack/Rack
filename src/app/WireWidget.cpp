#include "app.hpp"
#include "engine.hpp"
#include "components.hpp"
#include "gui.hpp"


namespace rack {

static void drawPlug(NVGcontext *vg, Vec pos, NVGcolor color) {
	NVGcolor colorOutline = nvgLerpRGBA(color, nvgRGBf(0.0, 0.0, 0.0), 0.5);

	// Plug solid
	nvgBeginPath(vg);
	nvgCircle(vg, pos.x, pos.y, 9);
	nvgFillColor(vg, color);
	nvgFill(vg);

	// Border
	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, colorOutline);
	nvgStroke(vg);

	// Hole
	nvgBeginPath(vg);
	nvgCircle(vg, pos.x, pos.y, 5);
	nvgFillColor(vg, nvgRGBf(0.0, 0.0, 0.0));
	nvgFill(vg);
}

static void drawWire(NVGcontext *vg, Vec pos1, Vec pos2, NVGcolor color, float tension, float opacity) {
	NVGcolor colorShadow = nvgRGBAf(0, 0, 0, 0.08);
	NVGcolor colorOutline = nvgLerpRGBA(color, nvgRGBf(0.0, 0.0, 0.0), 0.5);

	// Wire
	if (opacity > 0.0) {
		nvgSave(vg);
		nvgGlobalAlpha(vg, powf(opacity, 1.5));

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


static const NVGcolor wireColors[] = {
	nvgRGB(0xc9, 0xb7, 0x0e), // yellow
	nvgRGB(0xc9, 0x18, 0x47), // red
	nvgRGB(0x0c, 0x8e, 0x15), // green
	nvgRGB(0x09, 0x86, 0xad), // blue
	nvgRGB(0x44, 0x44, 0x44), // black
	// nvgRGB(0x66, 0x66, 0x66), // gray
	// nvgRGB(0x88, 0x88, 0x88), // light gray
	// nvgRGB(0xaa, 0xaa, 0xaa), // white
};
static int lastWireColorId = -1;


WireWidget::WireWidget() {
	lastWireColorId = (lastWireColorId + 1) % LENGTHOF(wireColors);
	color = wireColors[lastWireColorId];

	inputLight = construct<PolarityLight>(&PolarityLight::posColor, COLOR_GREEN, &PolarityLight::negColor, COLOR_RED);
	outputLight = construct<PolarityLight>(&PolarityLight::posColor, COLOR_GREEN, &PolarityLight::negColor, COLOR_RED);
	addChild(inputLight);
	addChild(outputLight);
}

WireWidget::~WireWidget() {
	outputPort = NULL;
	inputPort = NULL;
	updateWire();
}

void WireWidget::updateWire() {
	if (inputPort && outputPort) {
		// Check correct types
		assert(inputPort->type == Port::INPUT);
		assert(outputPort->type == Port::OUTPUT);

		if (!wire) {
			wire = new Wire();
			wire->outputModule = outputPort->module;
			wire->outputId = outputPort->portId;
			wire->inputModule = inputPort->module;
			wire->inputId = inputPort->portId;
			engineAddWire(wire);
		}
	}
	else {
		if (wire) {
			engineRemoveWire(wire);
			delete wire;
			wire = NULL;
		}
	}
}

Vec WireWidget::getOutputPos() {
	Vec pos;
	if (outputPort) {
		pos = Rect(outputPort->getAbsolutePos(), outputPort->box.size).getCenter();
	}
	else if (hoveredOutputPort) {
		pos = Rect(hoveredOutputPort->getAbsolutePos(), hoveredOutputPort->box.size).getCenter();
	}
	else {
		pos = gMousePos;
	}
	return pos.minus(getAbsolutePos().minus(box.pos));
}

Vec WireWidget::getInputPos() {
	Vec pos;
	if (inputPort) {
		pos = Rect(inputPort->getAbsolutePos(), inputPort->box.size).getCenter();
	}
	else if (hoveredInputPort) {
		pos = Rect(hoveredInputPort->getAbsolutePos(), hoveredInputPort->box.size).getCenter();
	}
	else {
		pos = gMousePos;
	}
	return pos.minus(getAbsolutePos().minus(box.pos));
}

void WireWidget::draw(NVGcontext *vg) {
	float opacity = gToolbar->wireOpacitySlider->value / 100.0;
	float tension = gToolbar->wireTensionSlider->value;

	// Draw as opaque if an "incomplete" wire
	if (!(inputPort && outputPort))
		opacity = 1.0;

	drawWire(vg, getOutputPos(), getInputPos(), color, tension, opacity);
}

void WireWidget::drawPlugs(NVGcontext *vg) {
	// TODO Figure out a way to draw plugs first and wires last, and cut the plug portion of the wire off.
	Vec outputPos = getOutputPos();
	Vec inputPos = getInputPos();
	drawPlug(vg, outputPos, color);
	drawPlug(vg, inputPos, color);

	// Draw plug light
	if (gToolbar->plugLightButton->value > 0.0) {
		if (wire) {
			Output &output = wire->outputModule->outputs[wire->outputId];
			float value = output.value / 8.0;
			outputLight->box.size = Vec(10, 10);
			inputLight->box.size = Vec(10, 10);
			outputLight->box.pos = outputPos.minus(Vec(5, 5));
			inputLight->box.pos = inputPos.minus(Vec(5, 5));
			outputLight->setValue(value);
			inputLight->setValue(value);
		}
		else {
			outputLight->setValue(0.0);
			inputLight->setValue(0.0);
		}
		outputLight->visible = true;
		inputLight->visible = true;
	}
	else {
		outputLight->visible = false;
		inputLight->visible = false;
	}
	Widget::draw(vg);
}


} // namespace rack
