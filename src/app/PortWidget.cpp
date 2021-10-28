#include <app/PortWidget.hpp>
#include <app/Scene.hpp>
#include <ui/MenuItem.hpp>
#include <ui/MenuSeparator.hpp>
#include <window/Window.hpp>
#include <context.hpp>
#include <history.hpp>
#include <engine/Engine.hpp>
#include <settings.hpp>
#include <helpers.hpp>


namespace rack {
namespace app {


struct PortTooltip : ui::Tooltip {
	PortWidget* portWidget;

	void step() override {
		if (portWidget->module) {
			engine::Port* port = portWidget->getPort();
			engine::PortInfo* portInfo = portWidget->getPortInfo();
			// Label
			text = portInfo->getFullName();
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
			for (CableWidget* cable : APP->scene->rack->getCablesOnPort(portWidget)) {
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


struct ColorMenuItem : ui::MenuItem {
	NVGcolor color;

	void draw(const DrawArgs& args) override {
		MenuItem::draw(args);

		// Color circle
		nvgBeginPath(args.vg);
		float radius = 6.0;
		nvgCircle(args.vg, 8.0 + radius, box.size.y / 2, radius);
		nvgFillColor(args.vg, color);
		nvgFill(args.vg);
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, color::mult(color, 0.5));
		nvgStroke(args.vg);
	}
};


struct PortCableItem : ColorMenuItem {
};


struct PortCreateCableItem : ColorMenuItem {
};


struct PortWidget::Internal {
	ui::Tooltip* tooltip = NULL;
};


PortWidget::PortWidget() {
	internal = new Internal;
}


PortWidget::~PortWidget() {
	// The port shouldn't have any cables when destroyed, but just to make sure.
	if (module)
		APP->scene->rack->clearCablesOnPort(this);
	// HACK: In case onDragDrop() is called but not onLeave() afterwards...
	destroyTooltip();
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


void PortWidget::createContextMenu() {
	ui::Menu* menu = createMenu();
	WeakPtr<PortWidget> weakThis = this;

	engine::PortInfo* portInfo = getPortInfo();
	assert(portInfo);
	menu->addChild(createMenuLabel(portInfo->getFullName()));

	std::vector<CableWidget*> cws = APP->scene->rack->getCablesOnPort(this);
	CableWidget* topCw = cws.empty() ? NULL : cws.back();

	menu->addChild(createMenuItem("Delete top cable", RACK_MOD_SHIFT_NAME "+click",
		[=]() {
			if (!weakThis)
				return;
			weakThis->deleteTopCableAction();
		},
		!topCw
	));

	// TODO
	if (type == engine::Port::INPUT) {
		menu->addChild(createMenuItem("Duplicate top cable", RACK_MOD_CTRL_NAME "+drag", NULL, true));
	}
	else {
	}

	// Create cable items
	bool createCableDisabled = (type == engine::Port::INPUT) && topCw;
	for (NVGcolor color : settings::cableColors) {
		// Include extra leading spaces for the color circle
		PortCreateCableItem* item = createMenuItem<PortCreateCableItem>("     New cable", "Click+drag");
		item->disabled = createCableDisabled;
		item->color = color;
		menu->addChild(item);
	}

	// Cable items
	if (!cws.empty()) {
		menu->addChild(new ui::MenuSeparator);

		for (CableWidget* cw : cws) {
			PortCableItem* item = createMenuItem<PortCableItem>("     XXXX", "Click+drag");
			item->color = nvgRGBf(1, 0, 0);
			menu->addChild(item);
		}
	}
}


void PortWidget::deleteTopCableAction() {
	CableWidget* cw = APP->scene->rack->getTopCable(this);
	if (!cw)
		return;

	// history::CableRemove
	history::CableRemove* h = new history::CableRemove;
	h->setCable(cw);
	APP->history->push(h);

	APP->scene->rack->removeCable(cw);
	delete cw;
}


void PortWidget::step() {
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
		createContextMenu();
		e.consume(this);
		return;
	}

	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
		deleteTopCableAction();
		// Consume null so onDragStart isn't triggered
		e.consume(NULL);
		return;
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


} // namespace app
} // namespace rack
