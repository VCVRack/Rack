#include "app/CableWidget.hpp"
#include "app/Scene.hpp"
#include "componentlibrary.hpp"
#include "window.hpp"
#include "event.hpp"
#include "app.hpp"
#include "patch.hpp"
#include "settings.hpp"


namespace rack {

static void drawPlug(const DrawContext &ctx, math::Vec pos, NVGcolor color) {
	NVGcolor colorOutline = nvgLerpRGBA(color, nvgRGBf(0.0, 0.0, 0.0), 0.5);

	// Plug solid
	nvgBeginPath(ctx.vg);
	nvgCircle(ctx.vg, pos.x, pos.y, 9);
	nvgFillColor(ctx.vg, color);
	nvgFill(ctx.vg);

	// Border
	nvgStrokeWidth(ctx.vg, 1.0);
	nvgStrokeColor(ctx.vg, colorOutline);
	nvgStroke(ctx.vg);

	// Hole
	nvgBeginPath(ctx.vg);
	nvgCircle(ctx.vg, pos.x, pos.y, 5);
	nvgFillColor(ctx.vg, nvgRGBf(0.0, 0.0, 0.0));
	nvgFill(ctx.vg);
}

static void drawCable(const DrawContext &ctx, math::Vec pos1, math::Vec pos2, NVGcolor color, float thickness, float tension, float opacity) {
	NVGcolor colorShadow = nvgRGBAf(0, 0, 0, 0.10);
	NVGcolor colorOutline = nvgLerpRGBA(color, nvgRGBf(0.0, 0.0, 0.0), 0.5);

	// Cable
	if (opacity > 0.0) {
		nvgSave(ctx.vg);
		// This power scaling looks more linear than actual linear scaling
		nvgGlobalAlpha(ctx.vg, std::pow(opacity, 1.5));

		float dist = pos1.minus(pos2).norm();
		math::Vec slump;
		slump.y = (1.0 - tension) * (150.0 + 1.0*dist);
		math::Vec pos3 = pos1.plus(pos2).div(2).plus(slump);

		nvgLineJoin(ctx.vg, NVG_ROUND);

		// Shadow
		math::Vec pos4 = pos3.plus(slump.mult(0.08));
		nvgBeginPath(ctx.vg);
		nvgMoveTo(ctx.vg, pos1.x, pos1.y);
		nvgQuadTo(ctx.vg, pos4.x, pos4.y, pos2.x, pos2.y);
		nvgStrokeColor(ctx.vg, colorShadow);
		nvgStrokeWidth(ctx.vg, thickness);
		nvgStroke(ctx.vg);

		// Cable outline
		nvgBeginPath(ctx.vg);
		nvgMoveTo(ctx.vg, pos1.x, pos1.y);
		nvgQuadTo(ctx.vg, pos3.x, pos3.y, pos2.x, pos2.y);
		nvgStrokeColor(ctx.vg, colorOutline);
		nvgStrokeWidth(ctx.vg, thickness);
		nvgStroke(ctx.vg);

		// Cable solid
		nvgStrokeColor(ctx.vg, color);
		nvgStrokeWidth(ctx.vg, thickness - 2);
		nvgStroke(ctx.vg);

		nvgRestore(ctx.vg);
	}
}


static const NVGcolor cableColors[] = {
	nvgRGB(0xc9, 0xb7, 0x0e), // yellow
	nvgRGB(0xc9, 0x18, 0x47), // red
	nvgRGB(0x0c, 0x8e, 0x15), // green
	nvgRGB(0x09, 0x86, 0xad), // blue
};
static int lastCableColorId = -1;


CableWidget::CableWidget() {
	lastCableColorId = (lastCableColorId + 1) % LENGTHOF(cableColors);
	color = cableColors[lastCableColorId];

	cable = new Cable;
}

CableWidget::~CableWidget() {
	delete cable;
}

bool CableWidget::isComplete() {
	return outputPort && inputPort;
}

void CableWidget::setOutput(PortWidget *outputPort) {
	this->outputPort = outputPort;
	if (outputPort) {
		assert(outputPort->type == PortWidget::OUTPUT);
		cable->outputModule = outputPort->module;
		cable->outputId = outputPort->portId;
	}
}

void CableWidget::setInput(PortWidget *inputPort) {
	this->inputPort = inputPort;
	if (inputPort) {
		assert(inputPort->type == PortWidget::INPUT);
		cable->inputModule = inputPort->module;
		cable->inputId = inputPort->portId;
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
		return app()->scene->rackWidget->mousePos;
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
		return app()->scene->rackWidget->mousePos;
	}
}

json_t *CableWidget::toJson() {
	assert(isComplete());
	json_t *rootJ = json_object();

	// This is just here for fun. It is not used in fromJson(), since cableIds are not preserved across multiple launches of Rack.
	json_object_set_new(rootJ, "id", json_integer(cable->id));

	json_object_set_new(rootJ, "outputModuleId", json_integer(cable->outputModule->id));
	json_object_set_new(rootJ, "outputId", json_integer(cable->outputId));
	json_object_set_new(rootJ, "inputModuleId", json_integer(cable->inputModule->id));
	json_object_set_new(rootJ, "inputId", json_integer(cable->inputId));

	std::string s = color::toHexString(color);
	json_object_set_new(rootJ, "color", json_string(s.c_str()));

	return rootJ;
}

void CableWidget::fromJson(json_t *rootJ, const std::map<int, ModuleWidget*> &moduleWidgets) {
	int outputModuleId = json_integer_value(json_object_get(rootJ, "outputModuleId"));
	int outputId = json_integer_value(json_object_get(rootJ, "outputId"));
	int inputModuleId = json_integer_value(json_object_get(rootJ, "inputModuleId"));
	int inputId = json_integer_value(json_object_get(rootJ, "inputId"));

	// Get module widgets
	auto outputModuleIt = moduleWidgets.find(outputModuleId);
	auto inputModuleIt = moduleWidgets.find(inputModuleId);
	if (outputModuleIt == moduleWidgets.end() || inputModuleIt == moduleWidgets.end())
		return;

	ModuleWidget *outputModule = outputModuleIt->second;
	ModuleWidget *inputModule = inputModuleIt->second;

	// Set ports
	if (app()->patch->isLegacy(1)) {
		// Before 0.6, the index of the "ports" array was the index of the PortWidget in the `outputs` and `inputs` vector.
		setOutput(outputModule->outputs[outputId]);
		setInput(inputModule->inputs[inputId]);
	}
	else {
		for (PortWidget *port : outputModule->outputs) {
			if (port->portId == outputId) {
				setOutput(port);
				break;
			}
		}
		for (PortWidget *port : inputModule->inputs) {
			if (port->portId == inputId) {
				setInput(port);
				break;
			}
		}
	}
	if (!isComplete())
		return;

	json_t *colorJ = json_object_get(rootJ, "color");
	if (colorJ) {
		// v0.6.0 and earlier patches use JSON objects. Just ignore them if so and use the existing cable color.
		if (json_is_string(colorJ))
			color = color::fromHexString(json_string_value(colorJ));
	}
}

void CableWidget::draw(const DrawContext &ctx) {
	float opacity = settings::cableOpacity;
	float tension = settings::cableTension;

	if (!isComplete()) {
		// Draw opaque if the cable is incomplete
		opacity = 1.0;
	}
	else {
		// Draw opaque if mouse is hovering over a connected port
		PortWidget *hoveredPort = dynamic_cast<PortWidget*>(app()->event->hoveredWidget);
		if (hoveredPort && (hoveredPort == outputPort || hoveredPort == inputPort))
			opacity = 1.0;
	}

	float thickness = 5;
	if (isComplete()) {
		Output *output = &cable->outputModule->outputs[cable->outputId];
		if (output->channels > 1) {
			// Increase thickness if output port is polyphonic
			thickness = 8;
		}
		// else if (output->channels == 0) {
		// 	// Draw translucent cable if not active (i.e. 0 channels)
		// 	opacity *= 0.5;
		// }
	}

	math::Vec outputPos = getOutputPos();
	math::Vec inputPos = getInputPos();
	drawCable(ctx, outputPos, inputPos, color, thickness, tension, opacity);
}

void CableWidget::drawPlugs(const DrawContext &ctx) {
	// TODO Figure out a way to draw plugs first and cables last, and cut the plug portion of the cable off.
	math::Vec outputPos = getOutputPos();
	math::Vec inputPos = getInputPos();

	// Draw plug if the cable is on top, or if the cable is incomplete
	if (!isComplete() || app()->scene->rackWidget->getTopCable(outputPort) == this) {
		drawPlug(ctx, outputPos, color);
		if (isComplete()) {
			// Draw plug light
			nvgSave(ctx.vg);
			nvgTranslate(ctx.vg, outputPos.x - 4, outputPos.y - 4);
			outputPort->plugLight->draw(ctx);
			nvgRestore(ctx.vg);
		}
	}

	if (!isComplete() || app()->scene->rackWidget->getTopCable(inputPort) == this) {
		drawPlug(ctx, inputPos, color);
		if (isComplete()) {
			nvgSave(ctx.vg);
			nvgTranslate(ctx.vg, inputPos.x - 4, inputPos.y - 4);
			inputPort->plugLight->draw(ctx);
			nvgRestore(ctx.vg);
		}
	}
}


} // namespace rack
