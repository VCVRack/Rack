#include "app.hpp"
#include "engine.hpp"
#include "componentlibrary.hpp"
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
	if (outputPort) {
		return outputPort->getRelativeOffset(outputPort->box.zeroPos().getCenter(), gRackWidget);
	}
	else if (hoveredOutputPort) {
		return hoveredOutputPort->getRelativeOffset(hoveredOutputPort->box.zeroPos().getCenter(), gRackWidget);
	}
	else {
		return gRackWidget->lastMousePos;
	}
}

Vec WireWidget::getInputPos() {
	if (inputPort) {
		return inputPort->getRelativeOffset(inputPort->box.zeroPos().getCenter(), gRackWidget);
	}
	else if (hoveredInputPort) {
		return hoveredInputPort->getRelativeOffset(hoveredInputPort->box.zeroPos().getCenter(), gRackWidget);
	}
	else {
		return gRackWidget->lastMousePos;
	}
}

json_t *WireWidget::toJson() {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "color", colorToJson(color));
	return rootJ;
}

void WireWidget::fromJson(json_t *rootJ) {
	json_t *colorJ = json_object_get(rootJ, "color");
	if (colorJ)
		color = jsonToColor(colorJ);
}

void WireWidget::draw(NVGcontext *vg) {
	float opacity = gToolbar->wireOpacitySlider->value / 100.0;
	float tension = gToolbar->wireTensionSlider->value;

	WireWidget *activeWire = gRackWidget->wireContainer->activeWire;
	if (activeWire) {
		// Draw as opaque if the wire is active
		if (activeWire == this)
			opacity = 1.0;
	}
	else {
		Port *hoveredPort = dynamic_cast<Port*>(gHoveredWidget);
		if (hoveredPort && (hoveredPort == outputPort || hoveredPort == inputPort))
			opacity = 1.0;
	}

	Vec outputPos = getOutputPos();
	Vec inputPos = getInputPos();
	drawWire(vg, outputPos, inputPos, color, tension, opacity);
}

void WireWidget::drawPlugs(NVGcontext *vg) {
	// TODO Figure out a way to draw plugs first and wires last, and cut the plug portion of the wire off.
	Vec outputPos = getOutputPos();
	Vec inputPos = getInputPos();
	drawPlug(vg, outputPos, color);
	drawPlug(vg, inputPos, color);

	// Draw plug light
	// TODO
	// Only draw this when light is on top of the plug stack
	if (outputPort) {
		nvgSave(vg);
		nvgTranslate(vg, outputPos.x - 4, outputPos.y - 4);
		outputPort->plugLight->draw(vg);
		nvgRestore(vg);
	}
	if (inputPort) {
		nvgSave(vg);
		nvgTranslate(vg, inputPos.x - 4, inputPos.y - 4);
		inputPort->plugLight->draw(vg);
		nvgRestore(vg);
	}
}


} // namespace rack
