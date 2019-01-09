#include "app/CableWidget.hpp"
#include "app/Scene.hpp"
#include "engine/Engine.hpp"
#include "componentlibrary.hpp"
#include "window.hpp"
#include "event.hpp"
#include "app.hpp"
#include "settings.hpp"


namespace rack {

static void drawPlug(NVGcontext *vg, math::Vec pos, NVGcolor color) {
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

static void drawCable(NVGcontext *vg, math::Vec pos1, math::Vec pos2, NVGcolor color, float thickness, float tension, float opacity) {
	NVGcolor colorShadow = nvgRGBAf(0, 0, 0, 0.10);
	NVGcolor colorOutline = nvgLerpRGBA(color, nvgRGBf(0.0, 0.0, 0.0), 0.5);

	// Cable
	if (opacity > 0.0) {
		nvgSave(vg);
		// This power scaling looks more linear than actual linear scaling
		nvgGlobalAlpha(vg, std::pow(opacity, 1.5));

		float dist = pos1.minus(pos2).norm();
		math::Vec slump;
		slump.y = (1.0 - tension) * (150.0 + 1.0*dist);
		math::Vec pos3 = pos1.plus(pos2).div(2).plus(slump);

		nvgLineJoin(vg, NVG_ROUND);

		// Shadow
		math::Vec pos4 = pos3.plus(slump.mult(0.08));
		nvgBeginPath(vg);
		nvgMoveTo(vg, pos1.x, pos1.y);
		nvgQuadTo(vg, pos4.x, pos4.y, pos2.x, pos2.y);
		nvgStrokeColor(vg, colorShadow);
		nvgStrokeWidth(vg, thickness);
		nvgStroke(vg);

		// Cable outline
		nvgBeginPath(vg);
		nvgMoveTo(vg, pos1.x, pos1.y);
		nvgQuadTo(vg, pos3.x, pos3.y, pos2.x, pos2.y);
		nvgStrokeColor(vg, colorOutline);
		nvgStrokeWidth(vg, thickness);
		nvgStroke(vg);

		// Cable solid
		nvgStrokeColor(vg, color);
		nvgStrokeWidth(vg, thickness - 2);
		nvgStroke(vg);

		nvgRestore(vg);
	}
}


static const NVGcolor cableColors[] = {
	nvgRGB(0xc9, 0xb7, 0x0e), // yellow
	nvgRGB(0xc9, 0x18, 0x47), // red
	nvgRGB(0x0c, 0x8e, 0x15), // green
	nvgRGB(0x09, 0x86, 0xad), // blue
	// nvgRGB(0x44, 0x44, 0x44), // black
	// nvgRGB(0x66, 0x66, 0x66), // gray
	// nvgRGB(0x88, 0x88, 0x88), // light gray
	// nvgRGB(0xaa, 0xaa, 0xaa), // white
};
static int lastCableColorId = -1;


CableWidget::CableWidget() {
	lastCableColorId = (lastCableColorId + 1) % LENGTHOF(cableColors);
	color = cableColors[lastCableColorId];
}

CableWidget::~CableWidget() {
	outputPort = NULL;
	inputPort = NULL;
	updateCable();
}

void CableWidget::updateCable() {
	if (inputPort && outputPort) {
		// Check correct types
		assert(inputPort->type == PortWidget::INPUT);
		assert(outputPort->type == PortWidget::OUTPUT);

		if (!cable) {
			cable = new Cable;
			cable->outputModule = outputPort->module;
			cable->outputId = outputPort->portId;
			cable->inputModule = inputPort->module;
			cable->inputId = inputPort->portId;
			app()->engine->addCable(cable);
		}
	}
	else {
		if (cable) {
			app()->engine->removeCable(cable);
			delete cable;
			cable = NULL;
		}
	}
}

math::Vec CableWidget::getOutputPos() {
	if (outputPort) {
		return outputPort->getRelativeOffset(outputPort->box.zeroPos().getCenter(), app()->scene->rackWidget);
	}
	else if (hoveredOutputPort) {
		return hoveredOutputPort->getRelativeOffset(hoveredOutputPort->box.zeroPos().getCenter(), app()->scene->rackWidget);
	}
	else {
		return app()->scene->rackWidget->lastMousePos;
	}
}

math::Vec CableWidget::getInputPos() {
	if (inputPort) {
		return inputPort->getRelativeOffset(inputPort->box.zeroPos().getCenter(), app()->scene->rackWidget);
	}
	else if (hoveredInputPort) {
		return hoveredInputPort->getRelativeOffset(hoveredInputPort->box.zeroPos().getCenter(), app()->scene->rackWidget);
	}
	else {
		return app()->scene->rackWidget->lastMousePos;
	}
}

json_t *CableWidget::toJson() {
	json_t *rootJ = json_object();
	std::string s = color::toHexString(color);
	json_object_set_new(rootJ, "color", json_string(s.c_str()));
	return rootJ;
}

void CableWidget::fromJson(json_t *rootJ) {
	json_t *colorJ = json_object_get(rootJ, "color");
	if (colorJ) {
		// v0.6.0 and earlier patches use JSON objects. Just ignore them if so and use the existing cable color.
		if (json_is_string(colorJ))
			color = color::fromHexString(json_string_value(colorJ));
	}
}

void CableWidget::draw(NVGcontext *vg) {
	float opacity = settings::cableOpacity;
	float tension = settings::cableTension;

	CableWidget *activeCable = app()->scene->rackWidget->cableContainer->activeCable;
	if (activeCable) {
		// Draw as opaque if the cable is active
		if (activeCable == this)
			opacity = 1.0;
	}
	else {
		PortWidget *hoveredPort = dynamic_cast<PortWidget*>(app()->event->hoveredWidget);
		if (hoveredPort && (hoveredPort == outputPort || hoveredPort == inputPort))
			opacity = 1.0;
	}

	float thickness = 5;
	if (cable && cable->outputModule) {
		Output *output = &cable->outputModule->outputs[cable->outputId];
		if (output->numChannels != 1) {
			thickness = 9;
		}
	}

	math::Vec outputPos = getOutputPos();
	math::Vec inputPos = getInputPos();
	drawCable(vg, outputPos, inputPos, color, thickness, tension, opacity);
}

void CableWidget::drawPlugs(NVGcontext *vg) {
	// TODO Figure out a way to draw plugs first and cables last, and cut the plug portion of the cable off.
	math::Vec outputPos = getOutputPos();
	math::Vec inputPos = getInputPos();
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
