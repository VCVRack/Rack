#include <event.hpp>
#include <widget/Widget.hpp>
#include <context.hpp>
#include <window.hpp>


namespace rack {
namespace event {


void State::setHovered(widget::Widget* w) {
	if (w == hoveredWidget)
		return;

	if (hoveredWidget) {
		// Trigger Leave event
		Leave eLeave;
		hoveredWidget->onLeave(eLeave);
		hoveredWidget = NULL;
	}

	if (w) {
		// Trigger Enter event
		Context cEnter;
		cEnter.target = w;
		Enter eEnter;
		eEnter.context = &cEnter;
		w->onEnter(eEnter);
		hoveredWidget = cEnter.target;
	}
}

void State::setDragged(widget::Widget* w, int button) {
	if (w == draggedWidget)
		return;

	if (draggedWidget) {
		// Trigger DragEnd event
		DragEnd eDragEnd;
		eDragEnd.button = dragButton;
		draggedWidget->onDragEnd(eDragEnd);
		draggedWidget = NULL;
	}

	dragButton = button;

	if (w) {
		// Trigger DragStart event
		Context cDragStart;
		cDragStart.target = w;
		DragStart eDragStart;
		eDragStart.context = &cDragStart;
		eDragStart.button = dragButton;
		w->onDragStart(eDragStart);
		draggedWidget = cDragStart.target;
	}
}

void State::setDragHovered(widget::Widget* w) {
	if (w == dragHoveredWidget)
		return;

	if (dragHoveredWidget) {
		// Trigger DragLeave event
		DragLeave eDragLeave;
		eDragLeave.button = dragButton;
		eDragLeave.origin = draggedWidget;
		dragHoveredWidget->onDragLeave(eDragLeave);
		dragHoveredWidget = NULL;
	}

	if (w) {
		// Trigger DragEnter event
		Context cDragEnter;
		cDragEnter.target = w;
		DragEnter eDragEnter;
		eDragEnter.context = &cDragEnter;
		eDragEnter.button = dragButton;
		eDragEnter.origin = draggedWidget;
		w->onDragEnter(eDragEnter);
		dragHoveredWidget = cDragEnter.target;
	}
}

void State::setSelected(widget::Widget* w) {
	if (w == selectedWidget)
		return;

	if (selectedWidget) {
		// Trigger Deselect event
		Deselect eDeselect;
		selectedWidget->onDeselect(eDeselect);
		selectedWidget = NULL;
	}

	if (w) {
		// Trigger Select event
		Context cSelect;
		cSelect.target = w;
		Select eSelect;
		eSelect.context = &cSelect;
		w->onSelect(eSelect);
		selectedWidget = cSelect.target;
	}
}

void State::finalizeWidget(widget::Widget* w) {
	if (hoveredWidget == w)
		setHovered(NULL);
	if (draggedWidget == w)
		setDragged(NULL, 0);
	if (dragHoveredWidget == w)
		setDragHovered(NULL);
	if (selectedWidget == w)
		setSelected(NULL);
	if (lastClickedWidget == w)
		lastClickedWidget = NULL;
}

bool State::handleButton(math::Vec pos, int button, int action, int mods) {
	bool cursorLocked = APP->window->isCursorLocked();

	widget::Widget* clickedWidget = NULL;
	if (!cursorLocked) {
		// Trigger Button event
		Context cButton;
		Button eButton;
		eButton.context = &cButton;
		eButton.pos = pos;
		eButton.button = button;
		eButton.action = action;
		eButton.mods = mods;
		rootWidget->onButton(eButton);
		clickedWidget = cButton.target;
	}

	if (action == GLFW_PRESS) {
		setDragged(clickedWidget, button);
	}

	if (action == GLFW_RELEASE) {
		setDragHovered(NULL);

		if (clickedWidget && draggedWidget) {
			// Trigger DragDrop event
			DragDrop eDragDrop;
			eDragDrop.button = dragButton;
			eDragDrop.origin = draggedWidget;
			clickedWidget->onDragDrop(eDragDrop);
		}

		setDragged(NULL, 0);
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			setSelected(clickedWidget);
		}

		if (action == GLFW_PRESS) {
			const double doubleClickDuration = 0.3;
			double clickTime = glfwGetTime();
			if (clickedWidget
			    && clickTime - lastClickTime <= doubleClickDuration
			    && lastClickedWidget == clickedWidget) {
				// Trigger DoubleClick event
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

	return !!clickedWidget;
}

bool State::handleHover(math::Vec pos, math::Vec mouseDelta) {
	bool cursorLocked = APP->window->isCursorLocked();

	// Fake a key RACK_HELD event for each held key
	if (!cursorLocked) {
		int mods = APP->window->getMods();
		for (int key : heldKeys) {
			int scancode = glfwGetKeyScancode(key);
			handleKey(pos, key, scancode, RACK_HELD, mods);
		}
	}

	if (draggedWidget) {
		bool dragHovered = false;
		if (!cursorLocked) {
			// Trigger DragHover event
			Context cDragHover;
			DragHover eDragHover;
			eDragHover.context = &cDragHover;
			eDragHover.button = dragButton;
			eDragHover.pos = pos;
			eDragHover.mouseDelta = mouseDelta;
			eDragHover.origin = draggedWidget;
			rootWidget->onDragHover(eDragHover);

			setDragHovered(cDragHover.target);
			// If consumed, don't continue after DragMove so Hover is not triggered.
			if (cDragHover.target)
				dragHovered = true;
		}

		// Trigger DragMove event
		DragMove eDragMove;
		eDragMove.button = dragButton;
		eDragMove.mouseDelta = mouseDelta;
		draggedWidget->onDragMove(eDragMove);
		if (dragHovered)
			return true;
	}

	if (!cursorLocked) {
		// Trigger Hover event
		Context cHover;
		Hover eHover;
		eHover.context = &cHover;
		eHover.pos = pos;
		eHover.mouseDelta = mouseDelta;
		rootWidget->onHover(eHover);

		setHovered(cHover.target);
		if (cHover.target)
			return true;
	}
	return false;
}

bool State::handleLeave() {
	heldKeys.clear();
	// When leaving the window, don't un-hover widgets because the mouse might be dragging.
	// setDragHovered(NULL);
	// setHovered(NULL);
	return true;
}

bool State::handleScroll(math::Vec pos, math::Vec scrollDelta) {
	// Trigger HoverScroll event
	Context cHoverScroll;
	HoverScroll eHoverScroll;
	eHoverScroll.context = &cHoverScroll;
	eHoverScroll.pos = pos;
	eHoverScroll.scrollDelta = scrollDelta;
	rootWidget->onHoverScroll(eHoverScroll);

	return !!cHoverScroll.target;
}

bool State::handleDrop(math::Vec pos, const std::vector<std::string>& paths) {
	// Trigger PathDrop event
	Context cPathDrop;
	PathDrop ePathDrop(paths);
	ePathDrop.context = &cPathDrop;
	ePathDrop.pos = pos;
	rootWidget->onPathDrop(ePathDrop);

	return !!cPathDrop.target;
}

bool State::handleText(math::Vec pos, int codepoint) {
	if (selectedWidget) {
		// Trigger SelectText event
		Context cSelectText;
		SelectText eSelectText;
		eSelectText.context = &cSelectText;
		eSelectText.codepoint = codepoint;
		selectedWidget->onSelectText(eSelectText);
		if (cSelectText.target)
			return true;
	}

	// Trigger HoverText event
	Context cHoverText;
	HoverText eHoverText;
	eHoverText.context = &cHoverText;
	eHoverText.pos = pos;
	eHoverText.codepoint = codepoint;
	rootWidget->onHoverText(eHoverText);

	return !!cHoverText.target;
}

bool State::handleKey(math::Vec pos, int key, int scancode, int action, int mods) {
	// Update heldKey state
	if (action == GLFW_PRESS) {
		heldKeys.insert(key);
	}
	else if (action == GLFW_RELEASE) {
		auto it = heldKeys.find(key);
		if (it != heldKeys.end())
			heldKeys.erase(it);
	}

	if (selectedWidget) {
		// Trigger SelectKey event
		Context cSelectKey;
		SelectKey eSelectKey;
		eSelectKey.context = &cSelectKey;
		eSelectKey.key = key;
		eSelectKey.scancode = scancode;
		const char* keyName = glfwGetKeyName(key, scancode);
		if (keyName)
			eSelectKey.keyName = keyName;
		eSelectKey.action = action;
		eSelectKey.mods = mods;
		selectedWidget->onSelectKey(eSelectKey);
		if (cSelectKey.target)
			return true;
	}

	// Trigger HoverKey event
	Context cHoverKey;
	HoverKey eHoverKey;
	eHoverKey.context = &cHoverKey;
	eHoverKey.pos = pos;
	eHoverKey.key = key;
	eHoverKey.scancode = scancode;
	const char* keyName = glfwGetKeyName(key, scancode);
	if (keyName)
		eHoverKey.keyName = keyName;
	eHoverKey.action = action;
	eHoverKey.mods = mods;
	rootWidget->onHoverKey(eHoverKey);
	return !!cHoverKey.target;
}

bool State::handleDirty() {
	// Trigger Dirty event
	Context cDirty;
	Dirty eDirty;
	eDirty.context = &cDirty;
	rootWidget->onDirty(eDirty);
	return true;
}


} // namespace event
} // namespace rack
