#include "ui.hpp"
// for gVg
#include "window.hpp"
// for key codes
#include <GLFW/glfw3.h>


namespace rack {


void TextField::draw(NVGcontext *vg) {
	nvgScissor(vg, 0, 0, box.size.x, box.size.y);

	BNDwidgetState state;
	if (this == gFocusedWidget)
		state = BND_ACTIVE;
	else if (this == gHoveredWidget)
		state = BND_HOVER;
	else
		state = BND_DEFAULT;

	int begin = min(cursor, selection);
	int end = max(cursor, selection);
	bndTextField(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str(), begin, end);
	// Draw placeholder text
	if (text.empty() && state != BND_ACTIVE) {
		bndIconLabelCaret(vg, 0.0, 0.0, box.size.x, box.size.y, -1, bndGetTheme()->textFieldTheme.itemColor, 13, placeholder.c_str(), bndGetTheme()->textFieldTheme.itemColor, 0, -1);
	}

	nvgResetScissor(vg);
}

void TextField::onMouseDown(EventMouseDown &e) {
	if (e.button == 0) {
		cursor = selection = getTextPosition(e.pos);
	}
	OpaqueWidget::onMouseDown(e);
}

void TextField::onMouseMove(EventMouseMove &e) {
	if (this == gDraggedWidget) {
		int pos = getTextPosition(e.pos);
		if (pos != selection) {
			cursor = pos;
		}
	}
	OpaqueWidget::onMouseMove(e);
}

void TextField::onFocus(EventFocus &e) {
	e.consumed = true;
}

void TextField::onText(EventText &e) {
	if (e.codepoint < 128) {
		std::string newText(1, (char) e.codepoint);
		insertText(newText);
	}
	e.consumed = true;
}

void TextField::onKey(EventKey &e) {
	switch (e.key) {
		case GLFW_KEY_BACKSPACE: {
			if (cursor == selection) {
				cursor--;
				if (cursor >= 0) {
					text.erase(cursor, 1);
					onTextChange();
				}
				selection = cursor;
			}
			else {
				int begin = min(cursor, selection);
				text.erase(begin, std::abs(selection - cursor));
				onTextChange();
				cursor = selection = begin;
			}
		} break;
		case GLFW_KEY_DELETE: {
			if (cursor == selection) {
				text.erase(cursor, 1);
				onTextChange();
			}
			else {
				int begin = min(cursor, selection);
				text.erase(begin, std::abs(selection - cursor));
				onTextChange();
				cursor = selection = begin;
			}
		} break;
		case GLFW_KEY_LEFT: {
			if (windowIsModPressed()) {
				while (--cursor > 0) {
					if (text[cursor] == ' ')
						break;
				}
			}
			else {
				cursor--;
			}
			if (!windowIsShiftPressed()) {
				selection = cursor;
			}
		} break;
		case GLFW_KEY_RIGHT: {
			if (windowIsModPressed()) {
				while (++cursor < (int) text.size()) {
					if (text[cursor] == ' ')
						break;
				}
			}
			else {
				cursor++;
			}
			if (!windowIsShiftPressed()) {
				selection = cursor;
			}
		} break;
		case GLFW_KEY_HOME: {
			selection = cursor = 0;
		} break;
		case GLFW_KEY_END: {
			selection = cursor = text.size();
		} break;
		case GLFW_KEY_V: {
			if (windowIsModPressed()) {
				const char *newText = glfwGetClipboardString(gWindow);
				if (newText)
					insertText(newText);
			}
		} break;
		case GLFW_KEY_X: {
			if (windowIsModPressed()) {
				if (cursor != selection) {
					int begin = min(cursor, selection);
					std::string selectedText = text.substr(begin, std::abs(selection - cursor));
					glfwSetClipboardString(gWindow, selectedText.c_str());
					insertText("");
				}
			}
		} break;
		case GLFW_KEY_C: {
			if (windowIsModPressed()) {
				if (cursor != selection) {
					int begin = min(cursor, selection);
					std::string selectedText = text.substr(begin, std::abs(selection - cursor));
					glfwSetClipboardString(gWindow, selectedText.c_str());
				}
			}
		} break;
		case GLFW_KEY_A: {
			if (windowIsModPressed()) {
				selection = 0;
				cursor = text.size();
			}
		} break;
		case GLFW_KEY_ENTER: {
			if (multiline) {
				insertText("\n");
			}
			else {
				EventAction e;
				onAction(e);
			}
		} break;
	}

	cursor = clamp(cursor, 0, (int) text.size());
	selection = clamp(selection, 0, (int) text.size());
	e.consumed = true;
}

void TextField::insertText(std::string text) {
	if (cursor != selection) {
		int begin = min(cursor, selection);
		this->text.erase(begin, std::abs(selection - cursor));
		cursor = selection = begin;
	}
	this->text.insert(cursor, text);
	cursor += text.size();
	selection = cursor;
	onTextChange();
}

void TextField::setText(std::string text) {
	this->text = text;
	selection = cursor = text.size();
	onTextChange();
}

int TextField::getTextPosition(Vec mousePos) {
	return bndTextFieldTextPosition(gVg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str(), mousePos.x, mousePos.y);
}


} // namespace rack
