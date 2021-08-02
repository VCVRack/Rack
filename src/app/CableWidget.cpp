#include <app/CableWidget.hpp>
#include <app/Scene.hpp>
#include <app/RackWidget.hpp>
#include <Window.hpp>
#include <context.hpp>
#include <patch.hpp>
#include <asset.hpp>
#include <settings.hpp>
#include <engine/Engine.hpp>
#include <engine/Port.hpp>


namespace rack {
namespace app {


struct CableWidget::Internal {
	std::shared_ptr<Svg> plugSvg;
	std::shared_ptr<Svg> plugPortSvg;
};


CableWidget::CableWidget() {
	internal = new Internal;
	color = color::BLACK_TRANSPARENT;
	internal->plugSvg = Svg::load(asset::system("res/ComponentLibrary/Plug.svg"));
	internal->plugPortSvg = Svg::load(asset::system("res/ComponentLibrary/PlugPort.svg"));
}

CableWidget::~CableWidget() {
	setCable(NULL);
	delete internal;
}

void CableWidget::setNextCableColor() {
	if (!settings::cableColors.empty()) {
		int id = APP->scene->rack->nextCableColorId++;
		APP->scene->rack->nextCableColorId %= settings::cableColors.size();
		color = settings::cableColors[id];
	}
}

bool CableWidget::isComplete() {
	return outputPort && inputPort;
}

void CableWidget::updateCable() {
	if (cable) {
		APP->engine->removeCable(cable);
		delete cable;
		cable = NULL;
	}
	if (inputPort && outputPort) {
		cable = new engine::Cable;
		cable->inputModule = inputPort->module;
		cable->inputId = inputPort->portId;
		cable->outputModule = outputPort->module;
		cable->outputId = outputPort->portId;
		APP->engine->addCable(cable);
	}
}

void CableWidget::setCable(engine::Cable* cable) {
	if (this->cable) {
		APP->engine->removeCable(this->cable);
		delete this->cable;
		this->cable = NULL;
	}
	this->cable = cable;
	if (cable) {
		app::ModuleWidget* outputModule = APP->scene->rack->getModule(cable->outputModule->id);
		assert(outputModule);
		outputPort = outputModule->getOutput(cable->outputId);
		assert(outputPort);

		app::ModuleWidget* inputModule = APP->scene->rack->getModule(cable->inputModule->id);
		assert(inputModule);
		inputPort = inputModule->getInput(cable->inputId);
		assert(inputPort);
	}
	else {
		outputPort = NULL;
		inputPort = NULL;
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

json_t* CableWidget::toJson() {
	json_t* rootJ = json_object();

	std::string s = color::toHexString(color);
	json_object_set_new(rootJ, "color", json_string(s.c_str()));

	return rootJ;
}

void CableWidget::fromJson(json_t* rootJ) {
	json_t* colorJ = json_object_get(rootJ, "color");
	if (colorJ) {
		// In <v0.6.0, cables used JSON objects instead of hex strings. Just ignore them if so and use the existing cable color.
		if (json_is_string(colorJ))
			color = color::fromHexString(json_string_value(colorJ));
	}
}

static math::Vec getCableSlump(math::Vec pos1, math::Vec pos2) {
	float dist = pos1.minus(pos2).norm();
	math::Vec avg = pos1.plus(pos2).div(2);
	// Lower average point as distance increases
	avg.y += (1.0 - settings::cableTension) * (150.0 + 1.0 * dist);
	return avg;
}

static void CableWidget_drawPlug(CableWidget* that, const widget::Widget::DrawArgs& args, math::Vec pos, math::Vec slump, NVGcolor color, bool top) {
	if (!top)
		return;

	nvgSave(args.vg);
	nvgTranslate(args.vg, pos.x, pos.y);

	// Plug
	nvgSave(args.vg);
	nvgTint(args.vg, color);
	std::shared_ptr<Svg> plugSvg = that->internal->plugSvg;
	math::Vec plugSize = plugSvg->getSize();
	float angle = slump.minus(pos).arg() - 0.5f * M_PI;
	nvgRotate(args.vg, angle);
	nvgTranslate(args.vg, VEC_ARGS(plugSize.div(2).neg()));
	plugSvg->draw(args.vg);
	nvgRestore(args.vg);

	// Port
	nvgSave(args.vg);
	std::shared_ptr<Svg> plugPortSvg = that->internal->plugPortSvg;
	math::Vec plugPortSize = plugPortSvg->getSize();
	nvgTranslate(args.vg, VEC_ARGS(plugPortSize.div(2).neg()));
	plugPortSvg->draw(args.vg);
	nvgRestore(args.vg);

	nvgRestore(args.vg);
}

static void CableWidget_drawCable(CableWidget* that, const widget::Widget::DrawArgs& args, math::Vec pos1, math::Vec pos2, NVGcolor color, bool thick, float opacity) {
	if (opacity <= 0.0)
		return;

	float thickness = thick ? 10.0 : 6.0;

	// The endpoints are off-center
	math::Vec slump = getCableSlump(pos1, pos2);
	pos1 = pos1.plus(slump.minus(pos1).normalize().mult(13.0));
	pos2 = pos2.plus(slump.minus(pos2).normalize().mult(13.0));

	nvgSave(args.vg);
	nvgAlpha(args.vg, std::pow(opacity, 1.5));

	nvgLineCap(args.vg, NVG_ROUND);
	// Avoids glitches when cable is bent
	nvgLineJoin(args.vg, NVG_ROUND);

	// Shadow
	math::Vec shadowSlump = slump.plus(math::Vec(0, 30));
	nvgBeginPath(args.vg);
	nvgMoveTo(args.vg, VEC_ARGS(pos1));
	nvgQuadTo(args.vg, VEC_ARGS(shadowSlump), VEC_ARGS(pos2));
	NVGcolor shadowColor = nvgRGBAf(0, 0, 0, 0.10);
	nvgStrokeColor(args.vg, shadowColor);
	nvgStrokeWidth(args.vg, thickness - 1.0);
	nvgStroke(args.vg);

	// Cable solid
	nvgBeginPath(args.vg);
	nvgMoveTo(args.vg, VEC_ARGS(pos1));
	nvgQuadTo(args.vg, VEC_ARGS(slump), VEC_ARGS(pos2));
	// nvgStrokePaint(args.vg, nvgLinearGradient(args.vg, VEC_ARGS(pos1), VEC_ARGS(pos2), color::mult(color, 0.5), color));
	nvgStrokeColor(args.vg, color::mult(color, 0.75));
	nvgStrokeWidth(args.vg, thickness);
	nvgStroke(args.vg);

	nvgStrokeColor(args.vg, color);
	nvgStrokeWidth(args.vg, thickness - 1.0);
	nvgStroke(args.vg);

	nvgRestore(args.vg);
}

void CableWidget::draw(const DrawArgs& args) {
	float opacity = settings::cableOpacity;
	bool thick = false;

	if (isComplete()) {
		engine::Output* output = &cable->outputModule->outputs[cable->outputId];
		// Increase thickness if output port is polyphonic
		if (output->channels > 1) {
			thick = true;
		}

		// Draw opaque if mouse is hovering over a connected port
		Widget* hoveredWidget = APP->event->hoveredWidget;
		if (outputPort == hoveredWidget || inputPort == hoveredWidget) {
			opacity = 1.0;
		}
		// Draw translucent cable if not active (i.e. 0 channels)
		else if (output->channels == 0) {
			opacity *= 0.5;
		}
	}
	else {
		// Draw opaque if the cable is incomplete
		opacity = 1.0;
	}

	math::Vec outputPos = getOutputPos();
	math::Vec inputPos = getInputPos();
	CableWidget_drawCable(this, args, outputPos, inputPos, color, thick, opacity);
}

void CableWidget::drawPlugs(const DrawArgs& args) {
	math::Vec outputPos = getOutputPos();
	math::Vec inputPos = getInputPos();
	math::Vec slump = getCableSlump(outputPos, inputPos);

	// Draw output plug
	bool outputTop = !isComplete() || APP->scene->rack->getTopCable(outputPort) == this;
	CableWidget_drawPlug(this, args, outputPos, slump, color, outputTop);
	if (outputTop && isComplete()) {
		// Draw output plug light
		nvgSave(args.vg);
		LightWidget* plugLight = outputPort->getPlugLight();
		math::Vec plugPos = outputPos.minus(plugLight->getSize().div(2));
		nvgTranslate(args.vg, VEC_ARGS(plugPos));
		plugLight->draw(args);
		nvgRestore(args.vg);
	}

	// Draw input plug
	bool inputTop = !isComplete() || APP->scene->rack->getTopCable(inputPort) == this;
	CableWidget_drawPlug(this, args, inputPos, slump, color, inputTop);
	if (inputTop && isComplete()) {
		// Draw input plug light
		nvgSave(args.vg);
		LightWidget* plugLight = inputPort->getPlugLight();
		math::Vec plugPos = inputPos.minus(plugLight->getSize().div(2));
		nvgTranslate(args.vg, VEC_ARGS(plugPos));
		plugLight->draw(args);
		nvgRestore(args.vg);
	}
}


engine::Cable* CableWidget::releaseCable() {
	engine::Cable* cable = this->cable;
	this->cable = NULL;
	return cable;
}


} // namespace app
} // namespace rack
