#include "event.hpp"
#include "widgets/Widget.hpp"


namespace rack {
namespace event {


void State::setHovered(Widget *w) {
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

void State::setDragged(Widget *w) {
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

void State::setDragHovered(Widget *w) {
	if (w == dragHoveredWidget)
		return;

	if (dragHoveredWidget) {
		// event::DragLeave
		event::DragLeave eDragLeave;
		eDragLeave.origin = draggedWidget;
		dragHoveredWidget->onDragLeave(eDragLeave);
	}

	dragHoveredWidget = w;

	if (dragHoveredWidget) {
		// event::DragEnter
		event::DragEnter eDragEnter;
		eDragEnter.origin = draggedWidget;
		dragHoveredWidget->onDragEnter(eDragEnter);
	}
}

void State::setSelected(Widget *w) {
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

void State::finalizeWidget(Widget *w) {
	if (hoveredWidget == w) setHovered(NULL);
	if (draggedWidget == w) setDragged(NULL);
	if (dragHoveredWidget == w) setDragHovered(NULL);
	if (selectedWidget == w) setSelected(NULL);
	if (scrollWidget == w) scrollWidget = NULL;
}

void State::handleButton(math::Vec pos, int button, int action, int mods) {
	// event::Button
	event::Context eButtonContext;
	event::Button eButton;
	eButton.context = &eButtonContext;
	eButton.pos = pos;
	eButton.button = button;
	eButton.action = action;
	eButton.mods = mods;
	rootWidget->onButton(eButton);
	Widget *clickedWidget = eButtonContext.consumed;

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

void State::handleHover(math::Vec pos, math::Vec mouseDelta) {
	if (draggedWidget) {
		// event::DragMove
		event::DragMove eDragMove;
		eDragMove.mouseDelta = mouseDelta;
		draggedWidget->onDragMove(eDragMove);

		// event::DragHover
		event::Context eDragHoverContext;
		event::DragHover eDragHover;
		eDragHover.context = &eDragHoverContext;
		eDragHover.pos = pos;
		eDragHover.mouseDelta = mouseDelta;
		eDragHover.origin = draggedWidget;
		rootWidget->onDragHover(eDragHover);

		setDragHovered(eDragHoverContext.consumed);

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
	event::Context eHoverContext;
	event::Hover eHover;
	eHover.context = &eHoverContext;
	eHover.pos = pos;
	eHover.mouseDelta = mouseDelta;
	rootWidget->onHover(eHover);

	setHovered(eHoverContext.consumed);
}

void State::handleLeave() {
	setDragHovered(NULL);
	setHovered(NULL);
}

void State::handleScroll(math::Vec pos, math::Vec scrollDelta) {
	// event::HoverScroll
	event::Context eHoverScrollContext;
	event::HoverScroll eHoverScroll;
	eHoverScroll.context = &eHoverScrollContext;
	eHoverScroll.pos = pos;
	eHoverScroll.scrollDelta = scrollDelta;
	rootWidget->onHoverScroll(eHoverScroll);
}

void State::handleDrop(math::Vec pos, const std::vector<std::string> &paths) {
	// event::PathDrop
	event::Context ePathDropContext;
	event::PathDrop ePathDrop(paths);
	ePathDrop.context = &ePathDropContext;
	ePathDrop.pos = pos;
	rootWidget->onPathDrop(ePathDrop);
}

void State::handleText(math::Vec pos, int codepoint) {
	if (selectedWidget) {
		// event::SelectText
		event::Context eSelectTextContext;
		event::SelectText eSelectText;
		eSelectText.context = &eSelectTextContext;
		eSelectText.codepoint = codepoint;
		selectedWidget->onSelectText(eSelectText);
		if (eSelectTextContext.consumed)
			return;
	}

	// event::HoverText
	event::Context eHoverTextContext;
	event::HoverText eHoverText;
	eHoverText.context = &eHoverTextContext;
	eHoverText.pos = pos;
	eHoverText.codepoint = codepoint;
	rootWidget->onHoverText(eHoverText);
}

void State::handleKey(math::Vec pos, int key, int scancode, int action, int mods) {
	if (selectedWidget) {
		// event::SelectKey
		event::Context eSelectKeyContext;
		event::SelectKey eSelectKey;
		eSelectKey.context = &eSelectKeyContext;
		eSelectKey.key = key;
		eSelectKey.scancode = scancode;
		eSelectKey.action = action;
		eSelectKey.mods = mods;
		selectedWidget->onSelectKey(eSelectKey);
		if (eSelectKeyContext.consumed)
			return;
	}

	// event::HoverKey
	event::Context eHoverKeyContext;
	event::HoverKey eHoverKey;
	eHoverKey.context = &eHoverKeyContext;
	eHoverKey.pos = pos;
	eHoverKey.key = key;
	eHoverKey.scancode = scancode;
	eHoverKey.action = action;
	eHoverKey.mods = mods;
	rootWidget->onHoverKey(eHoverKey);
}

void State::handleZoom() {
	// event::Zoom
	event::Context eZoomContext;
	event::Zoom eZoom;
	eZoom.context = &eZoomContext;
	rootWidget->onZoom(eZoom);
}


} // namespace event
} // namespace rack
