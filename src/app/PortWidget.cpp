#include <app/PortWidget.hpp>
#include <app/Scene.hpp>
#include <Window.hpp>
#include <context.hpp>
#include <history.hpp>
#include <engine/Engine.hpp>
#include <settings.hpp>
#include <componentlibrary.hpp>


namespace rack {
namespace app {


struct PortTooltip : ui::Tooltip {
	PortWidget* portWidget;

	void step() override {
		if (portWidget->module) {
			engine::Port* port = portWidget->getPort();
			engine::PortInfo* portInfo = portWidget->getPortInfo();
			// Label
			text = portInfo->getName();
			text += " ";
			text += (portWidget->type == engine::Port::INPUT) ? "input" : "output";
			// Description
			std::string description = portInfo->getDescription();
			if (description != "") {
				text += "\n";
				text += description;
			}
			// Voltage, number of channels
			int channels = port->getChannels();
			for (int i = 0; i < channels; i++) {
				float v = port->getVoltage(i);
				// Add newline or comma
				text += "\n";
				if (channels > 1)
					text += string::f("%d: ", i + 1);
				text += string::f("% .3fV", math::normalizeZero(v));
			}
			// Connected to
			std::list<CableWidget*> cables = APP->scene->rack->getCablesOnPort(portWidget);
			for (CableWidget* cable : cables) {
				PortWidget* otherPw = (portWidget->type == engine::Port::INPUT) ? cable->outputPort : cable->inputPort;
				if (!otherPw)
					continue;
				text += "\n";
				text += "Connected to ";
				text += otherPw->module->model->getFullName();
				text += ": ";
				text += otherPw->getPortInfo()->getName();
				text += " ";
				text += (otherPw->type == engine::Port::INPUT) ? "input" : "output";
			}
		}
		Tooltip::step();
		// Position at bottom-right of parameter
		box.pos = portWidget->getAbsoluteOffset(portWidget->box.size).round();
		// Fit inside parent (copied from Tooltip.cpp)
		assert(parent);
		box = box.nudge(parent->box.zeroPos());
	}
};


struct PortWidget::Internal {
	ui::Tooltip* tooltip = NULL;
	app::MultiLightWidget* plugLight;
};


PortWidget::PortWidget() {
	internal = new Internal;

	using namespace componentlibrary;
	struct PlugLight : TRedGreenBlueLight<TGrayModuleLightWidget<MediumLight<app::MultiLightWidget>>> {
		PlugLight() {
			// fb->bypassed = true;
			fb->oversample = 1.0;
		}
	};
	internal->plugLight = new PlugLight;
}

PortWidget::~PortWidget() {
	// The port shouldn't have any cables when destroyed, but just to make sure.
	if (module)
		APP->scene->rack->clearCablesOnPort(this);
	// HACK: In case onDragDrop() is called but not onLeave() afterwards...
	destroyTooltip();
	// plugLight is not a child but owned by the PortWidget, so we need to delete it here
	delete internal->plugLight;
	delete internal;
}

engine::Port* PortWidget::getPort() {
	if (!module)
		return NULL;
	if (type == engine::Port::INPUT)
		return &module->inputs[portId];
	else
		return &module->outputs[portId];
}

engine::PortInfo* PortWidget::getPortInfo() {
	if (!module)
		return NULL;
	if (type == engine::Port::INPUT)
		return module->inputInfos[portId];
	else
		return module->outputInfos[portId];
}

LightWidget* PortWidget::getPlugLight() {
	return internal->plugLight;
}

void PortWidget::createTooltip() {
	if (!settings::tooltips)
		return;
	if (internal->tooltip)
		return;
	if (!module)
		return;
	PortTooltip* tooltip = new PortTooltip;
	tooltip->portWidget = this;
	APP->scene->addChild(tooltip);
	internal->tooltip = tooltip;
}

void PortWidget::destroyTooltip() {
	if (!internal->tooltip)
		return;
	APP->scene->removeChild(internal->tooltip);
	delete internal->tooltip;
	internal->tooltip = NULL;
}

void PortWidget::step() {
	if (!module)
		return;

	std::vector<float> values(3);
	for (int i = 0; i < 3; i++) {
		if (type == engine::Port::OUTPUT)
			values[i] = module->outputs[portId].plugLights[i].getBrightness();
		else
			values[i] = module->inputs[portId].plugLights[i].getBrightness();
	}
	internal->plugLight->setBrightnesses(values);
	internal->plugLight->step();

	Widget::step();
}

void PortWidget::draw(const DrawArgs& args) {
	CableWidget* cw = APP->scene->rack->incompleteCable;
	if (cw) {
		// Dim the PortWidget if the active cable cannot plug into this PortWidget
		if (type == engine::Port::OUTPUT ? cw->outputPort : cw->inputPort) {
			nvgTint(args.vg, nvgRGBf(0.33, 0.33, 0.33));
		}
	}
	Widget::draw(args);
}

void PortWidget::onButton(const ButtonEvent& e) {
	OpaqueWidget::onButton(e);

	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
		CableWidget* cw = APP->scene->rack->getTopCable(this);
		if (cw) {
			// history::CableRemove
			history::CableRemove* h = new history::CableRemove;
			h->setCable(cw);
			APP->history->push(h);

			APP->scene->rack->removeCable(cw);
			delete cw;
		}

		e.consume(this);
	}
}

void PortWidget::onEnter(const EnterEvent& e) {
	createTooltip();
}

void PortWidget::onLeave(const LeaveEvent& e) {
	destroyTooltip();
}

void PortWidget::onDragStart(const DragStartEvent& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	CableWidget* cw = NULL;
	if ((APP->window->getMods() & RACK_MOD_MASK) == RACK_MOD_CTRL) {
		if (type == engine::Port::OUTPUT) {
			// Ctrl-clicking an output creates a new cable.
			// Keep cable NULL. Will be created below
		}
		else {
			// Ctrl-clicking an input clones the cable already patched to it.
			CableWidget* topCw = APP->scene->rack->getTopCable(this);
			if (topCw) {
				cw = new CableWidget;
				cw->color = topCw->color;
				cw->outputPort = topCw->outputPort;
				cw->updateCable();
			}
		}
	}
	else {
		// Grab cable on top of stack
		cw = APP->scene->rack->getTopCable(this);

		if (cw) {
			// history::CableRemove
			history::CableRemove* h = new history::CableRemove;
			h->setCable(cw);
			APP->history->push(h);

			// Disconnect and reuse existing cable
			APP->scene->rack->removeCable(cw);
			if (type == engine::Port::OUTPUT)
				cw->outputPort = NULL;
			else
				cw->inputPort = NULL;
			cw->updateCable();
		}
	}

	if (!cw) {
		// Create a new cable
		cw = new CableWidget;
		cw->setNextCableColor();
		if (type == engine::Port::OUTPUT)
			cw->outputPort = this;
		else
			cw->inputPort = this;
		cw->updateCable();
	}

	APP->scene->rack->setIncompleteCable(cw);
}

void PortWidget::onDragEnd(const DragEndEvent& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	CableWidget* cw = APP->scene->rack->releaseIncompleteCable();
	if (!cw)
		return;

	if (cw->isComplete()) {
		APP->scene->rack->addCable(cw);

		// history::CableAdd
		history::CableAdd* h = new history::CableAdd;
		h->setCable(cw);
		APP->history->push(h);
	}
	else {
		delete cw;
	}
}

void PortWidget::onDragDrop(const DragDropEvent& e) {
	// HACK: Only delete tooltip if we're not (normal) dragging it.
	if (e.origin == this)
		createTooltip();

	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	// Reject ports if this is an input port and something is already plugged into it
	if (type == engine::Port::INPUT) {
		if (APP->scene->rack->getTopCable(this))
			return;
	}

	CableWidget* cw = APP->scene->rack->incompleteCable;
	if (cw) {
		cw->hoveredOutputPort = cw->hoveredInputPort = NULL;
		if (type == engine::Port::OUTPUT)
			cw->outputPort = this;
		else
			cw->inputPort = this;
		cw->updateCable();
	}
}

void PortWidget::onDragEnter(const DragEnterEvent& e) {
	PortWidget* pw = dynamic_cast<PortWidget*>(e.origin);
	if (pw) {
		createTooltip();
	}

	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	// Reject ports if this is an input port and something is already plugged into it
	if (type == engine::Port::INPUT) {
		if (APP->scene->rack->getTopCable(this))
			return;
	}

	CableWidget* cw = APP->scene->rack->incompleteCable;
	if (cw) {
		if (type == engine::Port::OUTPUT)
			cw->hoveredOutputPort = this;
		else
			cw->hoveredInputPort = this;
	}
}

void PortWidget::onDragLeave(const DragLeaveEvent& e) {
	destroyTooltip();

	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	PortWidget* originPort = dynamic_cast<PortWidget*>(e.origin);
	if (!originPort)
		return;

	CableWidget* cw = APP->scene->rack->incompleteCable;
	if (cw) {
		if (type == engine::Port::OUTPUT)
			cw->hoveredOutputPort = NULL;
		else
			cw->hoveredInputPort = NULL;
	}
}

void PortWidget::onContextDestroy(const ContextDestroyEvent& e) {
	internal->plugLight->onContextDestroy(e);
	Widget::onContextDestroy(e);
}


} // namespace app
} // namespace rack
