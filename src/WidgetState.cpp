#include "WidgetState.hpp"

namespace rack {


WidgetState::WidgetState() {
	rootWidget = NULL;
	hoveredWidget = NULL;
	draggedWidget = NULL;
	dragHoveredWidget = NULL;
	selectedWidget = NULL;
}


void WidgetState::handleButton(math::Vec pos, int button, int action, int mods) {
	// Button event
	event::Button eButton;
	eButton.button = button;
	eButton.action = action;
	eButton.mods = mods;
	rootWidget->handleEvent(eButton);
	Widget *clickedWidget = eButton.target;

	if (button == GLFW_MOUSE_BUTTON_LEFT && clickedWidget) {
		// Drag events
		if (action == GLFW_PRESS && !draggedWidget) {
			event::DragStart eDragStart;
			clickedWidget->handleEvent(eDragStart);
			draggedWidget = eDragStart.target;
		}

		if (action == GLFW_RELEASE && draggedWidget) {
			event::DragDrop eDragDrop;
			eDragDrop.origin = draggedWidget;
			clickedWidget->handleEvent(eDragDrop);

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
}


// TODO Move this elsewhere
WidgetState *gWidgetState = NULL;


} // namespace rack
