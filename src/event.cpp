#include "event.hpp"
#include "widget/Widget.hpp"


namespace rack {
namespace event {


void State::setHovered(widget::Widget *w) {
	if (w == hoveredWidget)
		return;

	if (hoveredWidget) {
		// Leave
		Leave eLeave;
		hoveredWidget->onLeave(eLeave);
		hoveredWidget = NULL;
	}

	if (w) {
		// Enter
		Context cEnter;
		Enter eEnter;
		eEnter.context = &cEnter;
		w->onEnter(eEnter);
		hoveredWidget = cEnter.target;
	}
}

void State::setDragged(widget::Widget *w) {
	if (w == draggedWidget)
		return;

	if (draggedWidget) {
		// DragEnd
		DragEnd eDragEnd;
		draggedWidget->onDragEnd(eDragEnd);
		draggedWidget = NULL;
	}

	if (w) {
		// DragStart
		Context cDragStart;
		DragStart eDragStart;
		eDragStart.context = &cDragStart;
		w->onDragStart(eDragStart);
		draggedWidget = cDragStart.target;
	}
}

void State::setDragHovered(widget::Widget *w) {
	if (w == dragHoveredWidget)
		return;

	if (dragHoveredWidget) {
		// DragLeave
		DragLeave eDragLeave;
		eDragLeave.origin = draggedWidget;
		dragHoveredWidget->onDragLeave(eDragLeave);
		dragHoveredWidget = NULL;
	}

	if (w) {
		// DragEnter
		Context cDragEnter;
		DragEnter eDragEnter;
		eDragEnter.context = &cDragEnter;
		eDragEnter.origin = draggedWidget;
		w->onDragEnter(eDragEnter);
		dragHoveredWidget = cDragEnter.target;
	}
}

void State::setSelected(widget::Widget *w) {
	if (w == selectedWidget)
		return;

	if (selectedWidget) {
		// Deselect
		Deselect eDeselect;
		selectedWidget->onDeselect(eDeselect);
		selectedWidget = NULL;
	}

	if (w) {
		// Select
		Context cSelect;
		Select eSelect;
		eSelect.context = &cSelect;
		w->onSelect(eSelect);
		selectedWidget = cSelect.target;
	}
}

void State::finalizeWidget(widget::Widget *w) {
	if (hoveredWidget == w) setHovered(NULL);
	if (draggedWidget == w) setDragged(NULL);
	if (dragHoveredWidget == w) setDragHovered(NULL);
	if (selectedWidget == w) setSelected(NULL);
	if (lastClickedWidget == w) lastClickedWidget = NULL;
}

void State::handleButton(math::Vec pos, int button, int action, int mods) {
	// Button
	Context cButton;
	Button eButton;
	eButton.context = &cButton;
	eButton.pos = pos;
	eButton.button = button;
	eButton.action = action;
	eButton.mods = mods;
	rootWidget->onButton(eButton);
	widget::Widget *clickedWidget = cButton.target;

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			setDragged(clickedWidget);
		}

		if (action == GLFW_RELEASE) {
			setDragHovered(NULL);

			if (clickedWidget && draggedWidget) {
				// DragDrop
				DragDrop eDragDrop;
				eDragDrop.origin = draggedWidget;
				clickedWidget->onDragDrop(eDragDrop);
			}

			setDragged(NULL);
		}

		if (action == GLFW_PRESS) {
			setSelected(clickedWidget);
		}

		if (action == GLFW_PRESS) {
			const double doubleClickDuration = 0.3;
			double clickTime = glfwGetTime();
			if (clickedWidget
				&& clickTime - lastClickTime <= doubleClickDuration
				&& lastClickedWidget == clickedWidget) {
				// DoubleClick
				DoubleClick eDoubleClick;
				clickedWidget->onDoubleClick(eDoubleClick);
				// Reset double click
				lastClickTime = -INFINITY;
				lastClickedWidget = NULL;
			}
			else {
				lastClickTime = clickTime;
				lastClickedWidget = clickedWidget;
			}
		}
	}
}

void State::handleHover(math::Vec pos, math::Vec mouseDelta) {
	if (draggedWidget) {
		// DragMove
		DragMove eDragMove;
		eDragMove.mouseDelta = mouseDelta;
		draggedWidget->onDragMove(eDragMove);

		// DragHover
		Context cDragHover;
		DragHover eDragHover;
		eDragHover.context = &cDragHover;
		eDragHover.pos = pos;
		eDragHover.mouseDelta = mouseDelta;
		eDragHover.origin = draggedWidget;
		rootWidget->onDragHover(eDragHover);

		setDragHovered(cDragHover.target);

		return;
	}

	// Hover
	Context cHover;
	Hover eHover;
	eHover.context = &cHover;
	eHover.pos = pos;
	eHover.mouseDelta = mouseDelta;
	rootWidget->onHover(eHover);

	setHovered(cHover.target);
}

void State::handleLeave() {
	setDragHovered(NULL);
	setHovered(NULL);
}

void State::handleScroll(math::Vec pos, math::Vec scrollDelta) {
	// HoverScroll
	Context cHoverScroll;
	HoverScroll eHoverScroll;
	eHoverScroll.context = &cHoverScroll;
	eHoverScroll.pos = pos;
	eHoverScroll.scrollDelta = scrollDelta;
	rootWidget->onHoverScroll(eHoverScroll);
}

void State::handleDrop(math::Vec pos, const std::vector<std::string> &paths) {
	// PathDrop
	Context cPathDrop;
	PathDrop ePathDrop(paths);
	ePathDrop.context = &cPathDrop;
	ePathDrop.pos = pos;
	rootWidget->onPathDrop(ePathDrop);
}

void State::handleText(math::Vec pos, int codepoint) {
	if (selectedWidget) {
		// SelectText
		Context cSelectText;
		SelectText eSelectText;
		eSelectText.context = &cSelectText;
		eSelectText.codepoint = codepoint;
		selectedWidget->onSelectText(eSelectText);
		if (cSelectText.target)
			return;
	}

	// HoverText
	Context cHoverText;
	HoverText eHoverText;
	eHoverText.context = &cHoverText;
	eHoverText.pos = pos;
	eHoverText.codepoint = codepoint;
	rootWidget->onHoverText(eHoverText);
}

void State::handleKey(math::Vec pos, int key, int scancode, int action, int mods) {
	if (selectedWidget) {
		// SelectKey
		Context cSelectKey;
		SelectKey eSelectKey;
		eSelectKey.context = &cSelectKey;
		eSelectKey.key = key;
		eSelectKey.scancode = scancode;
		eSelectKey.action = action;
		eSelectKey.mods = mods;
		selectedWidget->onSelectKey(eSelectKey);
		if (cSelectKey.target)
			return;
	}

	// HoverKey
	Context cHoverKey;
	HoverKey eHoverKey;
	eHoverKey.context = &cHoverKey;
	eHoverKey.pos = pos;
	eHoverKey.key = key;
	eHoverKey.scancode = scancode;
	eHoverKey.action = action;
	eHoverKey.mods = mods;
	rootWidget->onHoverKey(eHoverKey);
}

void State::handleZoom() {
	// Zoom
	Context cZoom;
	Zoom eZoom;
	eZoom.context = &cZoom;
	rootWidget->onZoom(eZoom);
}


} // namespace event
} // namespace rack
