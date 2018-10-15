#include "WidgetState.hpp"
#include "event.hpp"
#include "widgets.hpp"
#include "logger.hpp"


namespace rack {


void WidgetState::handleButton(math::Vec pos, int button, int action, int mods) {
	// event::Button
	event::Button eButton;
	eButton.pos = pos;
	eButton.button = button;
	eButton.action = action;
	eButton.mods = mods;
	rootWidget->onButton(eButton);
	Widget *clickedWidget = eButton.target;

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS && !draggedWidget && clickedWidget) {
			// event::DragStart
			event::DragStart eDragStart;
			clickedWidget->onDragStart(eDragStart);
			draggedWidget = clickedWidget;
		}

		if (action == GLFW_RELEASE && draggedWidget) {
			if (dragHoveredWidget) {
				// event::DragLeave
				event::DragLeave eDragLeave;
				dragHoveredWidget->onDragLeave(eDragLeave);
			}

			if (clickedWidget) {
				// event::DragDrop
				event::DragDrop eDragDrop;
				eDragDrop.origin = draggedWidget;
				clickedWidget->onDragDrop(eDragDrop);
			}

			// event::DragEnd
			event::DragEnd eDragEnd;
			draggedWidget->onDragEnd(eDragEnd);
			draggedWidget = NULL;
			dragHoveredWidget = NULL;
		}

		if (action == GLFW_PRESS && clickedWidget != selectedWidget) {
			if (selectedWidget) {
				// event::Deselect
				event::Deselect eDeselect;
				selectedWidget->onDeselect(eDeselect);
			}

			selectedWidget = clickedWidget;

			if (selectedWidget) {
				// event::Select
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
		Widget *newDragHoveredWidget = eDragHover.target;

		if (newDragHoveredWidget != dragHoveredWidget) {
			if (dragHoveredWidget) {
				// event::DragLeave
				event::DragLeave eDragLeave;
				dragHoveredWidget->onDragLeave(eDragLeave);
			}

			dragHoveredWidget = newDragHoveredWidget;

			if (dragHoveredWidget) {
				// event::DragEnter
				event::DragEnter eDragEnter;
				dragHoveredWidget->onDragEnter(eDragEnter);
			}
		}

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
	Widget *newHoveredWidget = eHover.target;

	if (newHoveredWidget != hoveredWidget) {
		if (hoveredWidget) {
			// event::Leave
			event::Leave eLeave;
			hoveredWidget->onLeave(eLeave);
		}

		hoveredWidget = newHoveredWidget;

		if (hoveredWidget) {
			// event::Enter
			event::Enter eEnter;
			hoveredWidget->onEnter(eEnter);
		}
	}
}

void WidgetState::handleLeave() {
	if (hoveredWidget) {
		// event::Leave
		event::Leave eLeave;
		hoveredWidget->onLeave(eLeave);
	}
	hoveredWidget = NULL;
}

void WidgetState::handleScroll(math::Vec pos, math::Vec scrollDelta) {
	// event::HoverScroll
	event::HoverScroll eHoverScroll;
	eHoverScroll.pos = pos;
	eHoverScroll.scrollDelta = scrollDelta;
	rootWidget->onHoverScroll(eHoverScroll);
}

void WidgetState::handleDrop(math::Vec pos, std::vector<std::string> paths) {
	// event::PathDrop
	event::PathDrop ePathDrop;
	ePathDrop.pos = pos;
	ePathDrop.paths = paths;
	rootWidget->onPathDrop(ePathDrop);
}

void WidgetState::handleText(math::Vec pos, int codepoint) {
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

void WidgetState::handleKey(math::Vec pos, int key, int scancode, int action, int mods) {
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

void WidgetState::finalizeWidget(Widget *w) {
	if (hoveredWidget == w) hoveredWidget = NULL;
	if (draggedWidget == w) draggedWidget = NULL;
	if (dragHoveredWidget == w) dragHoveredWidget = NULL;
	if (selectedWidget == w) selectedWidget = NULL;
	if (scrollWidget == w) scrollWidget = NULL;
}

void WidgetState::handleZoom() {
	// event::Zoom
	event::Zoom eZoom;
	rootWidget->onZoom(eZoom);
}


// TODO Move this elsewhere
WidgetState *gWidgetState = NULL;


} // namespace rack
