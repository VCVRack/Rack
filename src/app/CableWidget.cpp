#include <app/CableWidget.hpp>
#include <widget/SvgWidget.hpp>
#include <widget/TransformWidget.hpp>
#include <app/Scene.hpp>
#include <app/RackWidget.hpp>
#include <app/ModuleWidget.hpp>
#include <context.hpp>
#include <asset.hpp>
#include <settings.hpp>
#include <engine/Engine.hpp>
#include <engine/Port.hpp>
#include <app/MultiLightWidget.hpp>
#include <componentlibrary.hpp>


namespace rack {
namespace app {


struct TintWidget : widget::Widget {
	NVGcolor color = color::WHITE;
	void draw(const DrawArgs& args) override {
		nvgTint(args.vg, color);
		Widget::draw(args);
	}
};


struct PlugLight : componentlibrary::TRedGreenBlueLight<app::MultiLightWidget> {
	PlugLight() {
		box.size = math::Vec(9, 9);
	}
};


struct PlugWidget::Internal {
	CableWidget* cableWidget;
	engine::Port::Type type;

	/** Initially pointing upward. */
	float angle = 0.5f * M_PI;

	widget::FramebufferWidget* fb;
	widget::TransformWidget* plugTransform;
	TintWidget* plugTint;
	widget::SvgWidget* plug;
	widget::SvgWidget* plugPort;
	app::MultiLightWidget* plugLight;
};


PlugWidget::PlugWidget() {
	internal = new Internal;

	internal->fb = new widget::FramebufferWidget;
	addChild(internal->fb);

	internal->plugTransform = new widget::TransformWidget;
	internal->fb->addChild(internal->plugTransform);

	internal->plugTint = new TintWidget;
	internal->plugTransform->addChild(internal->plugTint);

	internal->plug = new widget::SvgWidget;
	internal->plug->setSvg(window::Svg::load(asset::system("res/ComponentLibrary/Plug.svg")));
	internal->plugTint->addChild(internal->plug);
	internal->plugTransform->setSize(internal->plug->getSize());
	internal->plugTransform->setPosition(internal->plug->getSize().mult(-0.5));
	internal->plugTint->setSize(internal->plug->getSize());

	internal->plugPort = new widget::SvgWidget;
	internal->plugPort->setSvg(window::Svg::load(asset::system("res/ComponentLibrary/PlugPort.svg")));
	internal->plugPort->setPosition(internal->plugPort->getSize().mult(-0.5));
	internal->fb->addChild(internal->plugPort);

	internal->plugLight = new PlugLight;
	internal->plugLight->setPosition(internal->plugLight->getSize().mult(-0.5));
	addChild(internal->plugLight);

	setSize(internal->plug->getSize());
}


PlugWidget::~PlugWidget() {
	delete internal;
}

void PlugWidget::step() {
	std::vector<float> values(3);

	PortWidget* pw = internal->cableWidget->getPort(internal->type);
	if (pw && internal->plugLight->isVisible()) {
		engine::Port* port = pw->getPort();
		if (port) {
			for (int i = 0; i < 3; i++) {
				values[i] = port->plugLights[i].getBrightness();
			}
		}
	}
	internal->plugLight->setBrightnesses(values);

	Widget::step();
}

void PlugWidget::setColor(NVGcolor color) {
	if (color::isEqual(color, internal->plugTint->color))
		return;
	internal->plugTint->color = color;
	internal->fb->setDirty();
}

void PlugWidget::setAngle(float angle) {
	if (angle == internal->angle)
		return;
	internal->angle = angle;
	internal->plugTransform->identity();
	internal->plugTransform->rotate(angle - 0.5f * M_PI, internal->plug->getSize().div(2));
	internal->fb->setDirty();
}

void PlugWidget::setTop(bool top) {
	internal->plugLight->setVisible(top);
}

CableWidget* PlugWidget::getCable() {
	return internal->cableWidget;
}

engine::Port::Type PlugWidget::getType() {
	return internal->type;
}


struct CableWidget::Internal {
	/** For making history consistent when disconnecting and reconnecting cable. */
	int64_t cableId = -1;
};


CableWidget::CableWidget() {
	internal = new Internal;
	color = color::BLACK_TRANSPARENT;

	outputPlug = new PlugWidget;
	outputPlug->internal->cableWidget = this;
	outputPlug->internal->type = engine::Port::OUTPUT;

	inputPlug = new PlugWidget;
	inputPlug->internal->cableWidget = this;
	inputPlug->internal->type = engine::Port::INPUT;
}


CableWidget::~CableWidget() {
	delete outputPlug;
	delete inputPlug;

	setCable(NULL);
	delete internal;
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
		cable->id = internal->cableId;
		cable->inputModule = inputPort->module;
		cable->inputId = inputPort->portId;
		cable->outputModule = outputPort->module;
		cable->outputId = outputPort->portId;
		APP->engine->addCable(cable);
		internal->cableId = cable->id;
	}
}


void CableWidget::setCable(engine::Cable* cable) {
	if (this->cable) {
		APP->engine->removeCable(this->cable);
		delete this->cable;
		this->cable = NULL;
		internal->cableId = -1;
	}
	if (cable) {
		app::ModuleWidget* outputMw = APP->scene->rack->getModule(cable->outputModule->id);
		if (!outputMw)
			throw Exception("Cable cannot find output ModuleWidget %lld", (long long) cable->outputModule->id);
		outputPort = outputMw->getOutput(cable->outputId);
		if (!outputPort)
			throw Exception("Cable cannot find output port %d", cable->outputId);

		app::ModuleWidget* inputMw = APP->scene->rack->getModule(cable->inputModule->id);
		if (!inputMw)
			throw Exception("Cable cannot find input ModuleWidget %lld", (long long) cable->inputModule->id);
		inputPort = inputMw->getInput(cable->inputId);
		if (!inputPort)
			throw Exception("Cable cannot find input port %d", cable->inputId);

		this->cable = cable;
		internal->cableId = cable->id;
	}
	else {
		outputPort = NULL;
		inputPort = NULL;
	}
}


engine::Cable* CableWidget::getCable() {
	return cable;
}


math::Vec CableWidget::getInputPos() {
	if (inputPort) {
		return inputPort->getRelativeOffset(inputPort->box.zeroPos().getCenter(), APP->scene->rack);
	}
	else if (hoveredInputPort) {
		return hoveredInputPort->getRelativeOffset(hoveredInputPort->box.zeroPos().getCenter(), APP->scene->rack);
	}
	else {
		return APP->scene->rack->getMousePos();
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
		return APP->scene->rack->getMousePos();
	}
}


void CableWidget::mergeJson(json_t* rootJ) {
	std::string s = color::toHexString(color);
	json_object_set_new(rootJ, "color", json_string(s.c_str()));
}


void CableWidget::fromJson(json_t* rootJ) {
	json_t* colorJ = json_object_get(rootJ, "color");
	if (colorJ && json_is_string(colorJ)) {
		color = color::fromHexString(json_string_value(colorJ));
	}
	else {
		// In <v0.6.0, cables used JSON objects instead of hex strings. Just ignore them if so and use the existing cable color.
		// In <=v1, cable colors were not serialized.
		color = APP->scene->rack->getNextCableColor();
	}
}


static math::Vec getSlumpPos(math::Vec pos1, math::Vec pos2) {
	float dist = pos1.minus(pos2).norm();
	math::Vec avg = pos1.plus(pos2).div(2);
	// Lower average point as distance increases
	avg.y += (1.0 - settings::cableTension) * (150.0 + 1.0 * dist);
	return avg;
}


void CableWidget::step() {
	math::Vec outputPos = getOutputPos();
	math::Vec inputPos = getInputPos();
	math::Vec slump = getSlumpPos(outputPos, inputPos);

	NVGcolor colorOpaque = color;
	colorOpaque.a = 1.f;

	// Draw output plug
	outputPlug->setPosition(outputPos);
	bool outputTop = outputPort && (APP->scene->rack->getTopPlug(outputPort) == outputPlug);
	outputPlug->setTop(outputTop);
	outputPlug->setAngle(slump.minus(outputPos).arg());
	outputPlug->setColor(colorOpaque);

	// Draw input plug
	inputPlug->setPosition(inputPos);
	bool inputTop = inputPort && (APP->scene->rack->getTopPlug(inputPort) == inputPlug);
	inputPlug->setTop(inputTop);
	inputPlug->setAngle(slump.minus(inputPos).arg());
	inputPlug->setColor(colorOpaque);

	Widget::step();
}


void CableWidget::draw(const DrawArgs& args) {
	CableWidget::drawLayer(args, 0);
}


void CableWidget::drawLayer(const DrawArgs& args, int layer) {
	// Cable shadow and cable
	float opacity = settings::cableOpacity;
	bool thick = false;

	if (isComplete()) {
		engine::Output* output = &cable->outputModule->outputs[cable->outputId];
		// Increase thickness if output port is polyphonic
		if (output->isPolyphonic()) {
			thick = true;
		}

		// Draw opaque if mouse is hovering over a connected port
		Widget* hoveredWidget = APP->event->hoveredWidget;
		if (outputPort == hoveredWidget || inputPort == hoveredWidget) {
			opacity = 1.0;
		}
		// Draw translucent cable if not active (i.e. 0 channels)
		else if (output->getChannels() == 0) {
			opacity *= 0.5;
		}
	}
	else {
		// Draw opaque if the cable is incomplete
		opacity = 1.0;
	}

	if (opacity <= 0.0)
		return;
	nvgAlpha(args.vg, std::pow(opacity, 1.5));

	math::Vec outputPos = getOutputPos();
	math::Vec inputPos = getInputPos();

	float thickness = thick ? 9.0 : 6.0;

	// The endpoints are off-center
	math::Vec slump = getSlumpPos(outputPos, inputPos);
	float dist = 14.f;
	outputPos = outputPos.plus(slump.minus(outputPos).normalize().mult(dist));
	inputPos = inputPos.plus(slump.minus(inputPos).normalize().mult(dist));

	nvgLineCap(args.vg, NVG_ROUND);
	// Avoids glitches when cable is bent
	nvgLineJoin(args.vg, NVG_ROUND);

	if (layer == -1) {
		// Draw cable shadow
		// math::Vec shadowSlump = slump.plus(math::Vec(0, 30));
		// nvgBeginPath(args.vg);
		// nvgMoveTo(args.vg, VEC_ARGS(outputPos));
		// nvgQuadTo(args.vg, VEC_ARGS(shadowSlump), VEC_ARGS(inputPos));
		// NVGcolor shadowColor = nvgRGBAf(0, 0, 0, 0.10);
		// nvgStrokeColor(args.vg, shadowColor);
		// nvgStrokeWidth(args.vg, thickness - 1.0);
		// nvgStroke(args.vg);
	}
	else if (layer == 0) {
		// Draw cable outline
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, VEC_ARGS(outputPos));
		nvgQuadTo(args.vg, VEC_ARGS(slump), VEC_ARGS(inputPos));
		// nvgStrokePaint(args.vg, nvgLinearGradient(args.vg, VEC_ARGS(outputPos), VEC_ARGS(inputPos), color::mult(color, 0.5), color));
		nvgStrokeColor(args.vg, color::mult(color, 0.8));
		nvgStrokeWidth(args.vg, thickness);
		nvgStroke(args.vg);

		// Draw cable
		nvgStrokeColor(args.vg, color::mult(color, 0.95));
		nvgStrokeWidth(args.vg, thickness - 1.0);
		nvgStroke(args.vg);
	}

	Widget::drawLayer(args, layer);
}


engine::Cable* CableWidget::releaseCable() {
	engine::Cable* cable = this->cable;
	this->cable = NULL;
	internal->cableId = -1;
	return cable;
}


void CableWidget::onAdd(const AddEvent& e) {
	Widget* plugContainer = APP->scene->rack->getPlugContainer();
	plugContainer->addChild(outputPlug);
	plugContainer->addChild(inputPlug);
	Widget::onAdd(e);
}


void CableWidget::onRemove(const RemoveEvent& e) {
	Widget* plugContainer = APP->scene->rack->getPlugContainer();
	plugContainer->removeChild(outputPlug);
	plugContainer->removeChild(inputPlug);
	Widget::onRemove(e);
}


} // namespace app
} // namespace rack
