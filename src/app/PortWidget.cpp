#include <app/PortWidget.hpp>
#include <app/Scene.hpp>
#include <window.hpp>
#include <app.hpp>
#include <history.hpp>
#include <componentlibrary.hpp>
#include <settings.hpp> 


namespace rack {
namespace app {


struct PlugLight : MultiLightWidget {
	PlugLight() {
		addBaseColor(componentlibrary::SCHEME_GREEN);
		addBaseColor(componentlibrary::SCHEME_RED);
		addBaseColor(componentlibrary::SCHEME_BLUE);
		box.size = math::Vec(8, 8);
		bgColor = componentlibrary::SCHEME_BLACK_TRANSPARENT;
	}
};


PortWidget::PortWidget() {
	plugLight = new PlugLight;
}

PortWidget::~PortWidget() {
	// plugLight is not a child and is thus owned by the PortWidget, so we need to delete it here
	delete plugLight;
	// HACK
	if (module)
		APP->scene->rack->clearCablesOnPort(this);
}

void PortWidget::step() {
	if (!module)
		return;

	std::vector<float> values(3);
	for (int i = 0; i < 3; i++) {
		if (type == OUTPUT)
			values[i] = module->outputs[portId].plugLights[i].getBrightness();
		else
			values[i] = module->inputs[portId].plugLights[i].getBrightness();
	}
	plugLight->setBrightnesses(values);

	Widget::step();
}

void PortWidget::draw(const DrawArgs& args) {
	CableWidget* cw = APP->scene->rack->incompleteCable;
	if (cw) {
		// Dim the PortWidget if the active cable cannot plug into this PortWidget
		if (type == OUTPUT ? cw->outputPort : cw->inputPort)
			nvgGlobalAlpha(args.vg, 0.5);
	}
	Widget::draw(args);
}

void PortWidget::onButton(const event::Button& e) {
	OpaqueWidget::onButton(e);

	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
		CableWidget* cw = APP->scene->rack->getTopCable(this);
		if (cw) {
			if ((APP->window->getMods() & RACK_MOD_MASK) == RACK_MOD_CTRL) {
				int id = APP->scene->rack->nextCableColorId++;
				APP->scene->rack->nextCableColorId %= settings::cableColors.size();
				cw->color = settings::cableColors[id];
			} else 
			{
				// history::CableRemove
				history::CableRemove *h = new history::CableRemove;
				h->setCable(cw);
				APP->history->push(h);

				APP->scene->rack->removeCable(cw);
				delete cw;
			}
		}

		e.consume(this);
	}
}

void PortWidget::onEnter(const event::Enter& e) {
	hovered = true;
}

void PortWidget::onLeave(const event::Leave& e) {
	hovered = false;
}

void PortWidget::onDragStart(const event::DragStart& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	CableWidget* cw = NULL;
	if ((APP->window->getMods() & RACK_MOD_MASK) == RACK_MOD_CTRL) {
		if (type == OUTPUT) {
			// Keep cable NULL. Will be created below
		}
		else {
			CableWidget* topCw = APP->scene->rack->getTopCable(this);
			if (topCw) {
				cw = new CableWidget;
				cw->setOutput(topCw->outputPort);
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
			if (type == OUTPUT)
				cw->setOutput(NULL);
			else
				cw->setInput(NULL);
		}
	}

	if (!cw) {
		// Create a new cable
		cw = new CableWidget;
		if (type == OUTPUT)
			cw->setOutput(this);
		else
			cw->setInput(this);
	}

	APP->scene->rack->setIncompleteCable(cw);
}

void PortWidget::onDragEnd(const event::DragEnd& e) {
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

void PortWidget::onDragDrop(const event::DragDrop& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	// Reject ports if this is an input port and something is already plugged into it
	if (type == INPUT) {
		if (APP->scene->rack->getTopCable(this))
			return;
	}

	CableWidget* cw = APP->scene->rack->incompleteCable;
	if (cw) {
		cw->hoveredOutputPort = cw->hoveredInputPort = NULL;
		if (type == OUTPUT)
			cw->setOutput(this);
		else
			cw->setInput(this);
	}
}

void PortWidget::onDragEnter(const event::DragEnter& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	// Reject ports if this is an input port and something is already plugged into it
	if (type == INPUT) {
		if (APP->scene->rack->getTopCable(this))
			return;
	}

	CableWidget* cw = APP->scene->rack->incompleteCable;
	if (cw) {
		if (type == OUTPUT)
			cw->hoveredOutputPort = this;
		else
			cw->hoveredInputPort = this;
	}
}

void PortWidget::onDragLeave(const event::DragLeave& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	PortWidget* originPort = dynamic_cast<PortWidget*>(e.origin);
	if (!originPort)
		return;

	CableWidget* cw = APP->scene->rack->incompleteCable;
	if (cw) {
		if (type == OUTPUT)
			cw->hoveredOutputPort = NULL;
		else
			cw->hoveredInputPort = NULL;
	}
}


} // namespace app
} // namespace rack
