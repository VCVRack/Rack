#include "widget/event.hpp"
#include "widget/Widget.hpp"


namespace rack {
namespace widget {


void EventState::setHovered(Widget *w) {
	if (w == hoveredWidget)
		return;

	if (hoveredWidget) {
		// LeaveEvent
		LeaveEvent eLeave;
		hoveredWidget->onLeave(eLeave);
		hoveredWidget = NULL;
	}

	if (w) {
		// EnterEvent
		EventContext cEnter;
		EnterEvent eEnter;
		eEnter.context = &cEnter;
		w->onEnter(eEnter);
		hoveredWidget = cEnter.consumed;
	}
}

void EventState::setDragged(Widget *w) {
	if (w == draggedWidget)
		return;

	if (draggedWidget) {
		// DragEndEvent
		DragEndEvent eDragEnd;
		draggedWidget->onDragEnd(eDragEnd);
		draggedWidget = NULL;
	}

	if (w) {
		// DragStartEvent
		EventContext cDragStart;
		DragStartEvent eDragStart;
		eDragStart.context = &cDragStart;
		w->onDragStart(eDragStart);
		draggedWidget = cDragStart.consumed;
	}
}

void EventState::setDragHovered(Widget *w) {
	if (w == dragHoveredWidget)
		return;

	if (dragHoveredWidget) {
		// DragLeaveEvent
		DragLeaveEvent eDragLeave;
		eDragLeave.origin = draggedWidget;
		dragHoveredWidget->onDragLeave(eDragLeave);
		dragHoveredWidget = NULL;
	}

	if (w) {
		// DragEnterEvent
		EventContext cDragEnter;
		DragEnterEvent eDragEnter;
		eDragEnter.context = &cDragEnter;
		eDragEnter.origin = draggedWidget;
		w->onDragEnter(eDragEnter);
		dragHoveredWidget = cDragEnter.consumed;
	}
}

void EventState::setSelected(Widget *w) {
	if (w == selectedWidget)
		return;

	if (selectedWidget) {
		// DeselectEvent
		DeselectEvent eDeselect;
		selectedWidget->onDeselect(eDeselect);
		selectedWidget = NULL;
	}

	if (w) {
		// SelectEvent
		EventContext cSelect;
		SelectEvent eSelect;
		eSelect.context = &cSelect;
		w->onSelect(eSelect);
		selectedWidget = cSelect.consumed;
	}
}

void EventState::finalizeWidget(Widget *w) {
	if (hoveredWidget == w) setHovered(NULL);
	if (draggedWidget == w) setDragged(NULL);
	if (dragHoveredWidget == w) setDragHovered(NULL);
	if (selectedWidget == w) setSelected(NULL);
	if (lastClickedWidget == w) lastClickedWidget = NULL;
}

void EventState::handleButton(math::Vec pos, int button, int action, int mods) {
	// ButtonEvent
	EventContext cButton;
	ButtonEvent eButton;
	eButton.context = &cButton;
	eButton.pos = pos;
	eButton.button = button;
	eButton.action = action;
	eButton.mods = mods;
	rootWidget->onButton(eButton);
	Widget *clickedWidget = cButton.consumed;

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			setDragged(clickedWidget);
		}

		if (action == GLFW_RELEASE) {
			setDragHovered(NULL);

			if (clickedWidget && draggedWidget) {
				// DragDropEvent
				DragDropEvent eDragDrop;
				eDragDrop.origin = draggedWidget;
				clickedWidget->onDragDrop(eDragDrop);
			}

			setDragged(NULL);
		}

		if (action == GLFW_PRESS) {
			setSelected(clickedWidget);
		}

		if (action == GLFW_PRESS) {
			const double doubleClickDuration = 0.5;
			double clickTime = glfwGetTime();
			if (clickedWidget
				&& clickTime - lastClickTime <= doubleClickDuration
				&& lastClickedWidget == clickedWidget) {
				// DoubleClickEvent
				DoubleClickEvent eDoubleClick;
				clickedWidget->onDoubleClick(eDoubleClick);
			}
			lastClickTime = clickTime;
			lastClickedWidget = clickedWidget;
		}
	}
}

void EventState::handleHover(math::Vec pos, math::Vec mouseDelta) {
	if (draggedWidget) {
		// DragMoveEvent
		DragMoveEvent eDragMove;
		eDragMove.mouseDelta = mouseDelta;
		draggedWidget->onDragMove(eDragMove);

		// DragHoverEvent
		EventContext cDragHover;
		DragHoverEvent eDragHover;
		eDragHover.context = &cDragHover;
		eDragHover.pos = pos;
		eDragHover.mouseDelta = mouseDelta;
		eDragHover.origin = draggedWidget;
		rootWidget->onDragHover(eDragHover);

		setDragHovered(cDragHover.consumed);

		return;
	}

	// HoverEvent
	EventContext cHover;
	HoverEvent eHover;
	eHover.context = &cHover;
	eHover.pos = pos;
	eHover.mouseDelta = mouseDelta;
	rootWidget->onHover(eHover);

	setHovered(cHover.consumed);
}

void EventState::handleLeave() {
	setDragHovered(NULL);
	setHovered(NULL);
}

void EventState::handleScroll(math::Vec pos, math::Vec scrollDelta) {
	// HoverScrollEvent
	EventContext cHoverScroll;
	HoverScrollEvent eHoverScroll;
	eHoverScroll.context = &cHoverScroll;
	eHoverScroll.pos = pos;
	eHoverScroll.scrollDelta = scrollDelta;
	rootWidget->onHoverScroll(eHoverScroll);
}

void EventState::handleDrop(math::Vec pos, const std::vector<std::string> &paths) {
	// PathDropEvent
	EventContext cPathDrop;
	PathDropEvent ePathDrop(paths);
	ePathDrop.context = &cPathDrop;
	ePathDrop.pos = pos;
	rootWidget->onPathDrop(ePathDrop);
}

void EventState::handleText(math::Vec pos, int codepoint) {
	if (selectedWidget) {
		// SelectTextEvent
		EventContext cSelectText;
		SelectTextEvent eSelectText;
		eSelectText.context = &cSelectText;
		eSelectText.codepoint = codepoint;
		selectedWidget->onSelectText(eSelectText);
		if (cSelectText.consumed)
			return;
	}

	// HoverTextEvent
	EventContext cHoverText;
	HoverTextEvent eHoverText;
	eHoverText.context = &cHoverText;
	eHoverText.pos = pos;
	eHoverText.codepoint = codepoint;
	rootWidget->onHoverText(eHoverText);
}

void EventState::handleKey(math::Vec pos, int key, int scancode, int action, int mods) {
	if (selectedWidget) {
		// SelectKeyEvent
		EventContext cSelectKey;
		SelectKeyEvent eSelectKey;
		eSelectKey.context = &cSelectKey;
		eSelectKey.key = key;
		eSelectKey.scancode = scancode;
		eSelectKey.action = action;
		eSelectKey.mods = mods;
		selectedWidget->onSelectKey(eSelectKey);
		if (cSelectKey.consumed)
			return;
	}

	// HoverKeyEvent
	EventContext cHoverKey;
	HoverKeyEvent eHoverKey;
	eHoverKey.context = &cHoverKey;
	eHoverKey.pos = pos;
	eHoverKey.key = key;
	eHoverKey.scancode = scancode;
	eHoverKey.action = action;
	eHoverKey.mods = mods;
	rootWidget->onHoverKey(eHoverKey);
}

void EventState::handleZoom() {
	// ZoomEvent
	EventContext cZoom;
	ZoomEvent eZoom;
	eZoom.context = &cZoom;
	rootWidget->onZoom(eZoom);
}


} // namespace widget
} // namespace rack
