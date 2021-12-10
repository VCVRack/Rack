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


struct PlugWidget : widget::Widget {
	float angle = 0.f;
	PortWidget* portWidget = NULL;

	widget::FramebufferWidget* fb;
	widget::TransformWidget* plugTransform;
	TintWidget* plugTint;
	widget::SvgWidget* plug;

	widget::SvgWidget* plugPort;

	app::MultiLightWidget* plugLight;

	PlugWidget() {
		fb = new widget::FramebufferWidget;
		addChild(fb);

		plugTransform = new widget::TransformWidget;
		fb->addChild(plugTransform);

		plugTint = new TintWidget;
		plugTransform->addChild(plugTint);

		plug = new widget::SvgWidget;
		plug->setSvg(window::Svg::load(asset::system("res/ComponentLibrary/Plug.svg")));
		plugTint->addChild(plug);
		plugTransform->setSize(plug->getSize());
		plugTransform->setPosition(plug->getSize().mult(-0.5));
		plugTint->setSize(plug->getSize());

		plugPort = new widget::SvgWidget;
		plugPort->setSvg(window::Svg::load(asset::system("res/ComponentLibrary/PlugPort.svg")));
		plugPort->setPosition(plugPort->getSize().mult(-0.5));
		fb->addChild(plugPort);

		plugLight = new PlugLight;
		plugLight->setPosition(plugLight->getSize().mult(-0.5));
		addChild(plugLight);

		setSize(plug->getSize());
	}

	void step() override {
		std::vector<float> values(3);
		if (portWidget && plugLight->isVisible()) {
			engine::Port* port = portWidget->getPort();
			if (port) {
				for (int i = 0; i < 3; i++) {
					values[i] = port->plugLights[i].getBrightness();
				}
			}
		}
		plugLight->setBrightnesses(values);

		Widget::step();
	}

	void setColor(NVGcolor color) {
		if (color::isEqual(color, plugTint->color))
			return;
		plugTint->color = color;
		fb->setDirty();
	}

	void setAngle(float angle) {
		if (angle == this->angle)
			return;
		this->angle = angle;
		plugTransform->identity();
		plugTransform->rotate(angle - 0.5f * M_PI, plug->getSize().div(2));
		fb->setDirty();
	}

	void setPortWidget(PortWidget* portWidget) {
		this->portWidget = portWidget;
	}

	void setTop(bool top) {
		plugLight->setVisible(top);
	}
};


struct CableWidget::Internal {
};


CableWidget::CableWidget() {
	internal = new Internal;
	color = color::BLACK_TRANSPARENT;

	inputPlug = new PlugWidget;
	addChild(inputPlug);

	outputPlug = new PlugWidget;
	addChild(outputPlug);
}


CableWidget::~CableWidget() {
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
	if (cable) {
		app::ModuleWidget* outputMw = APP->scene->rack->getModule(cable->outputModule->id);
		if (!outputMw)
			throw Exception("Cable cannot find output ModuleWidget %ld", cable->outputModule->id);
		outputPort = outputMw->getOutput(cable->outputId);
		if (!outputPort)
			throw Exception("Cable cannot find output port %d", cable->outputId);

		app::ModuleWidget* inputMw = APP->scene->rack->getModule(cable->inputModule->id);
		if (!inputMw)
			throw Exception("Cable cannot find input ModuleWidget %ld", cable->inputModule->id);
		inputPort = inputMw->getInput(cable->inputId);
		if (!inputPort)
			throw Exception("Cable cannot find input port %d", cable->inputId);

		this->cable = cable;
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
	if (colorJ) {
		// In <v0.6.0, cables used JSON objects instead of hex strings. Just ignore them if so and use the existing cable color.
		if (json_is_string(colorJ))
			color = color::fromHexString(json_string_value(colorJ));
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

	// Draw output plug
	bool outputTop = !isComplete() || APP->scene->rack->getTopCable(outputPort) == this;
	outputPlug->setPosition(outputPos);
	outputPlug->setTop(outputTop);
	outputPlug->setAngle(slump.minus(outputPos).arg());
	outputPlug->setColor(color);
	outputPlug->setPortWidget(outputPort);

	// Draw input plug
	bool inputTop = !isComplete() || APP->scene->rack->getTopCable(inputPort) == this;
	inputPlug->setPosition(inputPos);
	inputPlug->setTop(inputTop);
	inputPlug->setAngle(slump.minus(inputPos).arg());
	inputPlug->setColor(color);
	inputPlug->setPortWidget(inputPort);

	Widget::step();
}


void CableWidget::draw(const DrawArgs& args) {
	// Draw plugs
	Widget::draw(args);
}


void CableWidget::drawLayer(const DrawArgs& args, int layer) {
	// Cable shadow and cable
	if (layer == 2 || layer == 3) {
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
		outputPos = outputPos.plus(slump.minus(outputPos).normalize().mult(13.0));
		inputPos = inputPos.plus(slump.minus(inputPos).normalize().mult(13.0));

		nvgLineCap(args.vg, NVG_ROUND);
		// Avoids glitches when cable is bent
		nvgLineJoin(args.vg, NVG_ROUND);

		if (layer == 2) {
			// Draw cable shadow
			math::Vec shadowSlump = slump.plus(math::Vec(0, 30));
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, VEC_ARGS(outputPos));
			nvgQuadTo(args.vg, VEC_ARGS(shadowSlump), VEC_ARGS(inputPos));
			NVGcolor shadowColor = nvgRGBAf(0, 0, 0, 0.10);
			nvgStrokeColor(args.vg, shadowColor);
			nvgStrokeWidth(args.vg, thickness - 1.0);
			nvgStroke(args.vg);
		}
		else if (layer == 3) {
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
	}

	Widget::drawLayer(args, layer);
}


engine::Cable* CableWidget::releaseCable() {
	engine::Cable* cable = this->cable;
	this->cable = NULL;
	return cable;
}


} // namespace app
} // namespace rack
