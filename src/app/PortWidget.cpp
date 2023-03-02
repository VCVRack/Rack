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


struct PortWidget::Internal {
	ui::Tooltip* tooltip = NULL;
	/** For overriding onDragStart behavior by menu items. */
	CableWidget* overrideCw = NULL;
	CableWidget* overrideCloneCw = NULL;
	bool overrideCreateCable = false;
	NVGcolor overrideColor = color::BLACK_TRANSPARENT;
};


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
			std::vector<CableWidget*> cables = APP->scene->rack->getCompleteCablesOnPort(portWidget);
			for (auto it = cables.rbegin(); it != cables.rend(); it++) {
				CableWidget* cable = *it;
				PortWidget* otherPw = (portWidget->type == engine::Port::INPUT) ? cable->outputPort : cable->inputPort;
				if (!otherPw)
					continue;
				text += "\n";
				if (portWidget->type == engine::Port::INPUT)
					text += "From ";
				else
					text += "To ";
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


struct PortCloneCableItem : ui::MenuItem {
	PortWidget* pw;
	CableWidget* cw;

	void onButton(const ButtonEvent& e) override {
		OpaqueWidget::onButton(e);
		if (disabled)
			return;
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
			// Set PortWidget::onDragStart overrides
			pw->internal->overrideCloneCw = cw;

			// Pretend the PortWidget was clicked
			e.consume(pw);
			// Deletes `this`
			doAction();
		}
	}
};


struct CableColorItem : ColorMenuItem {
	CableWidget* cw;

	void onAction(const ActionEvent& e) override {
		// history::CableColorChange
		history::CableColorChange* h = new history::CableColorChange;
		h->setCable(cw);
		h->newColor = color;
		h->oldColor = cw->color;
		APP->history->push(h);

		cw->color = color;
	}
};


struct PortCableItem : ColorMenuItem {
	PortWidget* pw;
	CableWidget* cw;

	void onButton(const ButtonEvent& e) override {
		OpaqueWidget::onButton(e);
		if (disabled)
			return;
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
			// Set PortWidget::onDragStart overrides
			pw->internal->overrideCw = cw;

			// Pretend the PortWidget was clicked
			e.consume(pw);
			// Deletes `this`
			doAction();
		}
	}

	ui::Menu* createChildMenu() override {
		ui::Menu* menu = new ui::Menu;

		for (NVGcolor color : settings::cableColors) {
			// Include extra leading spaces for the color circle
			CableColorItem* item = createMenuItem<CableColorItem>("     Set color");
			item->disabled = color::isEqual(color, cw->color);
			item->cw = cw;
			item->color = color;
			menu->addChild(item);
		}

		return menu;
	}
};


struct PortCreateCableItem : ColorMenuItem {
	PortWidget* pw;

	void onButton(const ButtonEvent& e) override {
		OpaqueWidget::onButton(e);
		if (disabled)
			return;
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
			// Set PortWidget::onDragStart overrides
			pw->internal->overrideCreateCable = true;
			pw->internal->overrideColor = color;

			// Pretend the PortWidget was clicked
			e.consume(pw);
			// Deletes `this`
			doAction();
		}
	}
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

	std::vector<CableWidget*> cws = APP->scene->rack->getCompleteCablesOnPort(this);
	CableWidget* topCw = cws.empty() ? NULL : cws.back();

	menu->addChild(createMenuItem("Delete top cable", RACK_MOD_SHIFT_NAME "+click",
		[=]() {
			if (!weakThis)
				return;
			weakThis->deleteTopCableAction();
		},
		!topCw
	));

	if (type == engine::Port::INPUT) {
		PortCloneCableItem* item = createMenuItem<PortCloneCableItem>("Duplicate top cable", RACK_MOD_CTRL_NAME "+drag");
		item->disabled = !topCw;
		item->pw = this;
		item->cw = topCw;
		menu->addChild(item);
	}

	menu->addChild(new ui::MenuSeparator);

	// New cable items
	bool createCableDisabled = (type == engine::Port::INPUT) && topCw;
	for (NVGcolor color : settings::cableColors) {
		// Include extra leading spaces for the color circle
		PortCreateCableItem* item = createMenuItem<PortCreateCableItem>("     New cable", "Click+drag");
		item->disabled = createCableDisabled;
		item->pw = this;
		item->color = color;
		menu->addChild(item);
	}

	if (!cws.empty()) {
		menu->addChild(new ui::MenuSeparator);
		menu->addChild(createMenuLabel("Click+drag to grab cable"));

		// Cable items
		for (auto it = cws.rbegin(); it != cws.rend(); it++) {
			CableWidget* cw = *it;
			PortWidget* pw = (type == engine::Port::INPUT) ? cw->outputPort : cw->inputPort;
			engine::PortInfo* portInfo = pw->getPortInfo();

			PortCableItem* item = createMenuItem<PortCableItem>("     " + portInfo->module->model->name + ": " + portInfo->getName(), RIGHT_ARROW);
			item->color = cw->color;
			item->pw = this;
			item->cw = cw;
			menu->addChild(item);
		}
	}

	appendContextMenu(menu);
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
	CableWidget* cw = APP->scene->rack->getIncompleteCable();
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

	DEFER({
		// Reset overrides
		internal->overrideCw = NULL;
		internal->overrideCloneCw = NULL;
		internal->overrideCreateCable = false;
		internal->overrideColor = color::BLACK_TRANSPARENT;
	});

	CableWidget* cw = NULL;
	if (internal->overrideCreateCable) {
		// Keep cable NULL. Will be created below
	}
	else if (internal->overrideCloneCw || (APP->window->getMods() & RACK_MOD_MASK) == RACK_MOD_CTRL) {
		if (type == engine::Port::OUTPUT) {
			// Ctrl-clicking an output creates a new cable.
			// Keep cable NULL. Will be created below
		}
		else {
			// Ctrl-clicking an input clones the cable already patched to it.
			CableWidget* cloneCw;
			if (internal->overrideCloneCw)
				cloneCw = internal->overrideCloneCw;
			else
				cloneCw = APP->scene->rack->getTopCable(this);

			if (cloneCw) {
				cw = new CableWidget;
				cw->color = cloneCw->color;
				cw->outputPort = cloneCw->outputPort;
				cw->updateCable();
			}
		}
	}
	else {
		// Grab cable on top of stack
		if (internal->overrideCw)
			cw = internal->overrideCw;
		else
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
		// Check that inputs don't already have a cable
		if (type == engine::Port::INPUT) {
			CableWidget* topCw = APP->scene->rack->getTopCable(this);
			if (topCw)
				return;
		}

		// Create a new cable
		cw = new CableWidget;

		// Set color
		if (internal->overrideCreateCable)
			cw->color = internal->overrideColor;
		else
			cw->color = APP->scene->rack->getNextCableColor();

		// Set port
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

	CableWidget* cw = APP->scene->rack->getIncompleteCable();
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

	CableWidget* cw = APP->scene->rack->getIncompleteCable();
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

	CableWidget* cw = APP->scene->rack->getIncompleteCable();
	if (cw) {
		if (type == engine::Port::OUTPUT)
			cw->hoveredOutputPort = NULL;
		else
			cw->hoveredInputPort = NULL;
	}
}


} // namespace app
} // namespace rack
