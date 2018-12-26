#include "event.hpp"
#include "widgets/Widget.hpp"


namespace rack {
namespace event {


void Context::setHovered(Widget *w) {
	if (w == hoveredWidget)
		return;

	if (hoveredWidget) {
		// event::Leave
		event::Leave eLeave;
		hoveredWidget->onLeave(eLeave);
	}

	hoveredWidget = w;

	if (hoveredWidget) {
		// event::Enter
		event::Enter eEnter;
		hoveredWidget->onEnter(eEnter);
	}
}

void Context::setDragged(Widget *w) {
	if (w == draggedWidget)
		return;

	if (draggedWidget) {
		// event::DragEnd
		event::DragEnd eDragEnd;
		draggedWidget->onDragEnd(eDragEnd);
	}

	draggedWidget = w;

	if (draggedWidget) {
		// event::DragStart
		event::DragStart eDragStart;
		draggedWidget->onDragStart(eDragStart);
	}
}

void Context::setDragHovered(Widget *w) {
	if (w == dragHoveredWidget)
		return;

	if (dragHoveredWidget) {
		// event::DragLeave
		event::DragLeave eDragLeave;
		dragHoveredWidget->onDragLeave(eDragLeave);
	}

	dragHoveredWidget = w;

	if (dragHoveredWidget) {
		// event::DragEnter
		event::DragEnter eDragEnter;
		dragHoveredWidget->onDragEnter(eDragEnter);
	}
}

void Context::setSelected(Widget *w) {
	if (w == selectedWidget)
		return;

	if (selectedWidget) {
		// event::Deselect
		event::Deselect eDeselect;
		selectedWidget->onDeselect(eDeselect);
	}

	selectedWidget = w;

	if (selectedWidget) {
		// event::Select
		event::Select eSelect;
		selectedWidget->onSelect(eSelect);
	}
}

void Context::finalizeWidget(Widget *w) {
	if (hoveredWidget == w) setHovered(NULL);
	if (draggedWidget == w) setDragged(NULL);
	if (dragHoveredWidget == w) setDragHovered(NULL);
	if (selectedWidget == w) setSelected(NULL);
	if (scrollWidget == w) scrollWidget = NULL;
}

void Context::handleButton(math::Vec pos, int button, int action, int mods) {
	// event::Button
	event::Button eButton;
	eButton.pos = pos;
	eButton.button = button;
	eButton.action = action;
	eButton.mods = mods;
	rootWidget->onButton(eButton);
	Widget *clickedWidget = eButton.target;

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			setDragged(clickedWidget);
		}

		if (action == GLFW_RELEASE) {
			setDragHovered(NULL);

			if (clickedWidget && draggedWidget) {
				// event::DragDrop
				event::DragDrop eDragDrop;
				eDragDrop.origin = draggedWidget;
				clickedWidget->onDragDrop(eDragDrop);
			}

			setDragged(NULL);
		}

		if (action == GLFW_PRESS) {
			setSelected(clickedWidget);
		}
	}

	// if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
	// 	if (action == GLFW_PRESS) {
	// 		scrollWidget = clickedWidget;
	// 	}
	// 	if (action == GLFW_RELEASE) {
	// 		scrollWidget = NULL;
	// 	}
	// }
}

void Context::handleHover(math::Vec pos, math::Vec mouseDelta) {
	if (draggedWidget) {
		// event::DragMove
		event::DragMove eDragMove;
		eDragMove.mouseDelta = mouseDelta;
		draggedWidget->onDragMove(eDragMove);

		// event::DragHover
		event::DragHover eDragHover;
		eDragHover.pos = pos;
		eDragHover.mouseDelta = mouseDelta;
		rootWidget->onDragHover(eDragHover);

		setDragHovered(eDragHover.target);

		return;
	}

	// if (scrollWidget) {
	// event::HoverScroll
	// 	event::HoverScroll eHoverScroll;
	// 	eHoverScroll.pos = pos;
	// 	eHoverScroll.scrollDelta = scrollDelta;
	// 	rootWidget->onHoverScroll(eHoverScroll);
	// }

	// event::Hover
	event::Hover eHover;
	eHover.pos = pos;
	eHover.mouseDelta = mouseDelta;
	rootWidget->onHover(eHover);

	setHovered(eHover.target);
}

void Context::handleLeave() {
	setDragHovered(NULL);
	setHovered(NULL);
}

void Context::handleScroll(math::Vec pos, math::Vec scrollDelta) {
	// event::HoverScroll
	event::HoverScroll eHoverScroll;
	eHoverScroll.pos = pos;
	eHoverScroll.scrollDelta = scrollDelta;
	rootWidget->onHoverScroll(eHoverScroll);
}

void Context::handleDrop(math::Vec pos, std::vector<std::string> paths) {
	// event::PathDrop
	event::PathDrop ePathDrop;
	ePathDrop.pos = pos;
	ePathDrop.paths = paths;
	rootWidget->onPathDrop(ePathDrop);
}

void Context::handleText(math::Vec pos, int codepoint) {
	if (selectedWidget) {
		// event::SelectText
		event::SelectText eSelectText;
		eSelectText.codepoint = codepoint;
		selectedWidget->onSelectText(eSelectText);
		if (eSelectText.target)
			return;
	}

	// event::HoverText
	event::HoverText eHoverText;
	eHoverText.pos = pos;
	eHoverText.codepoint = codepoint;
	rootWidget->onHoverText(eHoverText);
}

void Context::handleKey(math::Vec pos, int key, int scancode, int action, int mods) {
	if (selectedWidget) {
		// event::SelectKey
		event::SelectKey eSelectKey;
		eSelectKey.key = key;
		eSelectKey.scancode = scancode;
		eSelectKey.action = action;
		eSelectKey.mods = mods;
		selectedWidget->onSelectKey(eSelectKey);
		if (eSelectKey.target)
			return;
	}

	// event::HoverKey
	event::HoverKey eHoverKey;
	eHoverKey.pos = pos;
	eHoverKey.key = key;
	eHoverKey.scancode = scancode;
	eHoverKey.action = action;
	eHoverKey.mods = mods;
	rootWidget->onHoverKey(eHoverKey);
}

void Context::handleZoom() {
	// event::Zoom
	event::Zoom eZoom;
	rootWidget->onZoom(eZoom);
}


} // namespace event
} // namespace rack
