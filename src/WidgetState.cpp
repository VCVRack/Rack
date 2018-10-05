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
	rootWidget->handleEvent(eButton);
	Widget *clickedWidget = eButton.target;

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		// Drag events
		if (action == GLFW_PRESS && !draggedWidget && clickedWidget) {
			event::DragStart eDragStart;
			clickedWidget->handleEvent(eDragStart);
			draggedWidget = clickedWidget;
		}

		if (action == GLFW_RELEASE && draggedWidget) {
			if (clickedWidget) {
				event::DragDrop eDragDrop;
				eDragDrop.origin = draggedWidget;
				clickedWidget->handleEvent(eDragDrop);
			}

			event::DragEnd eDragEnd;
			draggedWidget->handleEvent(eDragEnd);
			draggedWidget = NULL;
		}

		// Select events
		if (action == GLFW_PRESS && clickedWidget != selectedWidget) {
			if (selectedWidget) {
				event::Deselect eDeselect;
				selectedWidget->handleEvent(eDeselect);
			}

			selectedWidget = clickedWidget;

			if (selectedWidget) {
				event::Select eSelect;
				selectedWidget->handleEvent(eSelect);
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
		draggedWidget->handleEvent(eDragMove);
		return;
	}

	// if (scrollWidget) {
	// 	event::HoverScroll eHoverScroll;
	// 	eHoverScroll.pos = pos;
	// 	eHoverScroll.scrollDelta = scrollDelta;
	// 	rootWidget->handleEvent(eHoverScroll);
	// }

	// Hover event
	event::Hover eHover;
	eHover.pos = pos;
	eHover.mouseDelta = mouseDelta;
	rootWidget->handleEvent(eHover);
	Widget *newHoveredWidget = eHover.target;

	if (newHoveredWidget != hoveredWidget) {
		if (hoveredWidget) {
			event::Leave eLeave;
			hoveredWidget->handleEvent(eLeave);
		}

		hoveredWidget = newHoveredWidget;

		if (hoveredWidget) {
			event::Enter eEnter;
			hoveredWidget->handleEvent(eEnter);
		}
	}
}

void WidgetState::handleLeave() {
	if (hoveredWidget) {
		// Leave event
		event::Leave eLeave;
		hoveredWidget->handleEvent(eLeave);
	}
	hoveredWidget = NULL;
}

void WidgetState::handleScroll(math::Vec pos, math::Vec scrollDelta) {
	// HoverScroll event
	event::HoverScroll eHoverScroll;
	eHoverScroll.pos = pos;
	eHoverScroll.scrollDelta = scrollDelta;
	rootWidget->handleEvent(eHoverScroll);
}

void WidgetState::handleDrop(math::Vec pos, std::vector<std::string> paths) {
	// PathDrop event
	event::PathDrop ePathDrop;
	ePathDrop.pos = pos;
	ePathDrop.paths = paths;
	rootWidget->handleEvent(ePathDrop);
}

void WidgetState::handleText(math::Vec pos, int codepoint) {
	if (selectedWidget) {
		// SelectText event
		event::SelectText eSelectText;
		eSelectText.codepoint = codepoint;
		selectedWidget->handleEvent(eSelectText);
		if (eSelectText.target)
			return;
	}

	// HoverText event
	event::HoverText eHoverText;
	eHoverText.pos = pos;
	eHoverText.codepoint = codepoint;
	rootWidget->handleEvent(eHoverText);
}

void WidgetState::handleKey(math::Vec pos, int key, int scancode, int action, int mods) {
	if (selectedWidget) {
		event::SelectKey eSelectKey;
		eSelectKey.key = key;
		eSelectKey.scancode = scancode;
		eSelectKey.action = action;
		eSelectKey.mods = mods;
		selectedWidget->handleEvent(eSelectKey);
		if (eSelectKey.target)
			return;
	}

	event::HoverKey eHoverKey;
	eHoverKey.pos = pos;
	eHoverKey.key = key;
	eHoverKey.scancode = scancode;
	eHoverKey.action = action;
	eHoverKey.mods = mods;
	rootWidget->handleEvent(eHoverKey);
}

void WidgetState::finalizeWidget(Widget *w) {
	if (hoveredWidget == w) hoveredWidget = NULL;
	if (draggedWidget == w) draggedWidget = NULL;
	if (dragHoveredWidget == w) dragHoveredWidget = NULL;
	if (selectedWidget == w) selectedWidget = NULL;
	if (scrollWidget == w) scrollWidget = NULL;
}



// TODO Move this elsewhere
WidgetState *gWidgetState = NULL;


} // namespace rack
