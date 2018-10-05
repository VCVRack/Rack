#include "WidgetState.hpp"
#include "logger.hpp"

namespace rack {


void WidgetState::handleButton(math::Vec pos, int button, int action, int mods) {
	// Button event
	event::Button eButton;
	eButton.pos = pos;
	eButton.button = button;
	eButton.action = action;
	eButton.mods = mods;
	rootWidget->onButton(eButton);
	Widget *clickedWidget = eButton.target;

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		// Drag events
		if (action == GLFW_PRESS && !draggedWidget && clickedWidget) {
			event::DragStart eDragStart;
			clickedWidget->onDragStart(eDragStart);
			draggedWidget = clickedWidget;
		}

		if (action == GLFW_RELEASE && draggedWidget) {
			if (clickedWidget) {
				event::DragDrop eDragDrop;
				eDragDrop.origin = draggedWidget;
				clickedWidget->onDragDrop(eDragDrop);
			}

			event::DragEnd eDragEnd;
			draggedWidget->onDragEnd(eDragEnd);
			draggedWidget = NULL;
		}

		// Select events
		if (action == GLFW_PRESS && clickedWidget != selectedWidget) {
			if (selectedWidget) {
				event::Deselect eDeselect;
				selectedWidget->onDeselect(eDeselect);
			}

			selectedWidget = clickedWidget;

			if (selectedWidget) {
				event::Select eSelect;
				selectedWidget->onSelect(eSelect);
			}
		}
	}

	if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		if (action == GLFW_PRESS) {
			scrollWidget = clickedWidget;
		}
		if (action == GLFW_RELEASE) {
			scrollWidget = NULL;
		}
	}
}


void WidgetState::handleHover(math::Vec pos, math::Vec mouseDelta) {
	// Drag events
	if (draggedWidget) {
		event::DragMove eDragMove;
		eDragMove.mouseDelta = mouseDelta;
		draggedWidget->onDragMove(eDragMove);
		return;
	}

	// if (scrollWidget) {
	// 	event::HoverScroll eHoverScroll;
	// 	eHoverScroll.pos = pos;
	// 	eHoverScroll.scrollDelta = scrollDelta;
	// 	rootWidget->onHoverScroll(eHoverScroll);
	// }

	// Hover event
	event::Hover eHover;
	eHover.pos = pos;
	eHover.mouseDelta = mouseDelta;
	rootWidget->onHover(eHover);
	Widget *newHoveredWidget = eHover.target;

	if (newHoveredWidget != hoveredWidget) {
		if (hoveredWidget) {
			event::Leave eLeave;
			hoveredWidget->onLeave(eLeave);
		}

		hoveredWidget = newHoveredWidget;

		if (hoveredWidget) {
			event::Enter eEnter;
			hoveredWidget->onEnter(eEnter);
		}
	}
}

void WidgetState::handleLeave() {
	if (hoveredWidget) {
		// Leave event
		event::Leave eLeave;
		hoveredWidget->onLeave(eLeave);
	}
	hoveredWidget = NULL;
}

void WidgetState::handleScroll(math::Vec pos, math::Vec scrollDelta) {
	// HoverScroll event
	event::HoverScroll eHoverScroll;
	eHoverScroll.pos = pos;
	eHoverScroll.scrollDelta = scrollDelta;
	rootWidget->onHoverScroll(eHoverScroll);
}

void WidgetState::handleDrop(math::Vec pos, std::vector<std::string> paths) {
	// PathDrop event
	event::PathDrop ePathDrop;
	ePathDrop.pos = pos;
	ePathDrop.paths = paths;
	rootWidget->onPathDrop(ePathDrop);
}

void WidgetState::handleText(math::Vec pos, int codepoint) {
	if (selectedWidget) {
		// SelectText event
		event::SelectText eSelectText;
		eSelectText.codepoint = codepoint;
		selectedWidget->onSelectText(eSelectText);
		if (eSelectText.target)
			return;
	}

	// HoverText event
	event::HoverText eHoverText;
	eHoverText.pos = pos;
	eHoverText.codepoint = codepoint;
	rootWidget->onHoverText(eHoverText);
}

void WidgetState::handleKey(math::Vec pos, int key, int scancode, int action, int mods) {
	if (selectedWidget) {
		event::SelectKey eSelectKey;
		eSelectKey.key = key;
		eSelectKey.scancode = scancode;
		eSelectKey.action = action;
		eSelectKey.mods = mods;
		selectedWidget->onSelectKey(eSelectKey);
		if (eSelectKey.target)
			return;
	}

	event::HoverKey eHoverKey;
	eHoverKey.pos = pos;
	eHoverKey.key = key;
	eHoverKey.scancode = scancode;
	eHoverKey.action = action;
	eHoverKey.mods = mods;
	rootWidget->onHoverKey(eHoverKey);
}

void WidgetState::finalizeWidget(Widget *w) {
	if (hoveredWidget == w) hoveredWidget = NULL;
	if (draggedWidget == w) draggedWidget = NULL;
	if (dragHoveredWidget == w) dragHoveredWidget = NULL;
	if (selectedWidget == w) selectedWidget = NULL;
	if (scrollWidget == w) scrollWidget = NULL;
}

void WidgetState::handleZoom() {
	event::Zoom eZoom;
	rootWidget->onZoom(eZoom);
}


// TODO Move this elsewhere
WidgetState *gWidgetState = NULL;


} // namespace rack
