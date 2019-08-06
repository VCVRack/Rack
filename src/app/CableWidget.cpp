#include <app/CableWidget.hpp>
#include <app/Scene.hpp>
#include <app/RackWidget.hpp>
#include <window.hpp>
#include <app.hpp>
#include <patch.hpp>
#include <settings.hpp>
#include <engine/Port.hpp>


namespace rack {
namespace app {

static void drawPlug(NVGcontext* vg, math::Vec pos, NVGcolor color) {
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

static void drawCable(NVGcontext* vg, math::Vec pos1, math::Vec pos2, NVGcolor color, float thickness, float tension, float opacity) {
	NVGcolor colorShadow = nvgRGBAf(0, 0, 0, 0.10);
	NVGcolor colorOutline = nvgLerpRGBA(color, nvgRGBf(0.0, 0.0, 0.0), 0.5);

	// Cable
	if (opacity > 0.0) {
		nvgSave(vg);
		// This power scaling looks more linear than actual linear scaling
		nvgGlobalAlpha(vg, std::pow(opacity, 1.5));

		float dist = pos1.minus(pos2).norm();
		math::Vec slump;
		slump.y = (1.0 - tension) * (150.0 + 1.0 * dist);
		math::Vec pos3 = pos1.plus(pos2).div(2).plus(slump);

		// Adjust pos1 and pos2 to not draw over the plug
		pos1 = pos1.plus(pos3.minus(pos1).normalize().mult(9));
		pos2 = pos2.plus(pos3.minus(pos2).normalize().mult(9));

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


CableWidget::CableWidget() {
	cable = new engine::Cable;

	color = color::BLACK_TRANSPARENT;
	if (!settings::cableColors.empty()) {
		int id = APP->scene->rack->nextCableColorId++;
		APP->scene->rack->nextCableColorId %= settings::cableColors.size();
		color = settings::cableColors[id];
	}
}

CableWidget::~CableWidget() {
	delete cable;
}

bool CableWidget::isComplete() {
	return outputPort && inputPort;
}

void CableWidget::setOutput(PortWidget* outputPort) {
	this->outputPort = outputPort;
	if (outputPort) {
		assert(outputPort->type == PortWidget::OUTPUT);
		cable->outputModule = outputPort->module;
		cable->outputId = outputPort->portId;
	}
}

void CableWidget::setInput(PortWidget* inputPort) {
	this->inputPort = inputPort;
	if (inputPort) {
		assert(inputPort->type == PortWidget::INPUT);
		cable->inputModule = inputPort->module;
		cable->inputId = inputPort->portId;
	}
}

math::Vec CableWidget::getOutputPos() {
	if (outputPort) {
		return outputPort->getRelativeOffset(outputPort->box.zeroPos().getCenter(), APP->scene->rack);
	}
	else if (hoveredOutputPort) {
		return hoveredOutputPort->getRelativeOffset(hoveredOutputPort->box.zeroPos().getCenter(), APP->scene->rack);
	}
	else {
		return APP->scene->rack->mousePos;
	}
}

math::Vec CableWidget::getInputPos() {
	if (inputPort) {
		return inputPort->getRelativeOffset(inputPort->box.zeroPos().getCenter(), APP->scene->rack);
	}
	else if (hoveredInputPort) {
		return hoveredInputPort->getRelativeOffset(hoveredInputPort->box.zeroPos().getCenter(), APP->scene->rack);
	}
	else {
		return APP->scene->rack->mousePos;
	}
}

json_t* CableWidget::toJson() {
	assert(isComplete());
	json_t* rootJ = json_object();

	json_object_set_new(rootJ, "id", json_integer(cable->id));
	json_object_set_new(rootJ, "outputModuleId", json_integer(cable->outputModule->id));
	json_object_set_new(rootJ, "outputId", json_integer(cable->outputId));
	json_object_set_new(rootJ, "inputModuleId", json_integer(cable->inputModule->id));
	json_object_set_new(rootJ, "inputId", json_integer(cable->inputId));

	std::string s = color::toHexString(color);
	json_object_set_new(rootJ, "color", json_string(s.c_str()));

	return rootJ;
}

void CableWidget::fromJson(json_t* rootJ) {
	// outputModuleId
	json_t* outputModuleIdJ = json_object_get(rootJ, "outputModuleId");
	if (!outputModuleIdJ)
		return;
	int outputModuleId = json_integer_value(outputModuleIdJ);
	ModuleWidget* outputModule = APP->scene->rack->getModule(outputModuleId);
	if (!outputModule)
		return;

	// inputModuleId
	json_t* inputModuleIdJ = json_object_get(rootJ, "inputModuleId");
	if (!inputModuleIdJ)
		return;
	int inputModuleId = json_integer_value(inputModuleIdJ);
	ModuleWidget* inputModule = APP->scene->rack->getModule(inputModuleId);
	if (!inputModule)
		return;

	// outputId
	json_t* outputIdJ = json_object_get(rootJ, "outputId");
	if (!outputIdJ)
		return;
	int outputId = json_integer_value(outputIdJ);

	// inputId
	json_t* inputIdJ = json_object_get(rootJ, "inputId");
	if (!inputIdJ)
		return;
	int inputId = json_integer_value(inputIdJ);

	// Only set ID if unset
	if (cable->id < 0) {
		// id
		json_t* idJ = json_object_get(rootJ, "id");
		// Before 1.0, cables IDs were not used, so just leave it as default and Engine will assign one automatically.
		if (idJ) {
			cable->id = json_integer_value(idJ);
		}
	}

	// Set ports
	if (APP->patch->isLegacy(1)) {
		// Before 0.6, the index of the "ports" array was the index of the PortWidget in the `outputs` and `inputs` vector.
		setOutput(outputModule->outputs[outputId]);
		setInput(inputModule->inputs[inputId]);
	}
	else {
		for (PortWidget* port : outputModule->outputs) {
			if (port->portId == outputId) {
				setOutput(port);
				break;
			}
		}
		for (PortWidget* port : inputModule->inputs) {
			if (port->portId == inputId) {
				setInput(port);
				break;
			}
		}
	}
	if (!isComplete())
		return;

	json_t* colorJ = json_object_get(rootJ, "color");
	if (colorJ) {
		// v0.6.0 and earlier patches use JSON objects. Just ignore them if so and use the existing cable color.
		if (json_is_string(colorJ))
			color = color::fromHexString(json_string_value(colorJ));
	}
}

void CableWidget::draw(const DrawArgs& args) {
	float opacity = settings::cableOpacity;
	float tension = settings::cableTension;
	float thickness = 5;

	if (isComplete()) {
		engine::Output* output = &cable->outputModule->outputs[cable->outputId];
		// Draw opaque if mouse is hovering over a connected port
		if (output->channels > 1) {
			// Increase thickness if output port is polyphonic
			thickness = 9;
		}

		if (outputPort->hovered || inputPort->hovered) {
			opacity = 1.0;
		}
		else if (output->channels == 0) {
			// Draw translucent cable if not active (i.e. 0 channels)
			opacity *= 0.5;
		}
	}
	else {
		// Draw opaque if the cable is incomplete
		opacity = 1.0;
	}

	math::Vec outputPos = getOutputPos();
	math::Vec inputPos = getInputPos();
	drawCable(args.vg, outputPos, inputPos, color, thickness, tension, opacity);
}

void CableWidget::drawPlugs(const DrawArgs& args) {
	math::Vec outputPos = getOutputPos();
	math::Vec inputPos = getInputPos();

	// Draw plug if the cable is on top, or if the cable is incomplete
	if (!isComplete() || APP->scene->rack->getTopCable(outputPort) == this) {
		drawPlug(args.vg, outputPos, color);
		if (isComplete()) {
			// Draw plug light
			nvgSave(args.vg);
			nvgTranslate(args.vg, outputPos.x - 4, outputPos.y - 4);
			outputPort->plugLight->draw(args);
			nvgRestore(args.vg);
		}
	}

	if (!isComplete() || APP->scene->rack->getTopCable(inputPort) == this) {
		drawPlug(args.vg, inputPos, color);
		if (isComplete()) {
			nvgSave(args.vg);
			nvgTranslate(args.vg, inputPos.x - 4, inputPos.y - 4);
			inputPort->plugLight->draw(args);
			nvgRestore(args.vg);
		}
	}
}


} // namespace app
} // namespace rack
