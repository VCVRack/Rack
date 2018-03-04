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

	bndTextField(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str(), begin, end);
	// Draw placeholder text
	if (text.empty() && state != BND_ACTIVE) {
		bndIconLabelCaret(vg, 0.0, 0.0, box.size.x, box.size.y, -1, bndGetTheme()->textFieldTheme.itemColor, 13, placeholder.c_str(), bndGetTheme()->textFieldTheme.itemColor, 0, -1);
	}

	nvgResetScissor(vg);
}

void TextField::onMouseDown(EventMouseDown &e) {
	dragPos = getTextPosition(e.pos);
	begin = end = dragPos;
	OpaqueWidget::onMouseDown(e);
}

void TextField::onMouseMove(EventMouseMove &e) {
	if (this == gDraggedWidget) {
		int pos = getTextPosition(e.pos);
		if (pos != dragPos) {
			begin = min(dragPos, pos);
			end = max(dragPos, pos);
		}
	}
	OpaqueWidget::onMouseMove(e);
}

void TextField::onFocus(EventFocus &e) {
	begin = 0;
	end = text.size();
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
		case GLFW_KEY_BACKSPACE:
			if (begin < end) {
				text.erase(begin, end - begin);
				onTextChange();
			}
			else {
				begin--;
				if (begin >= 0) {
					text.erase(begin, 1);
					onTextChange();
				}
			}
			end = begin;
			break;
		case GLFW_KEY_DELETE:
			if (begin < end) {
				text.erase(begin, end - begin);
				onTextChange();
			}
			else {
				text.erase(begin, 1);
				onTextChange();
			}
			end = begin;
			break;
		case GLFW_KEY_LEFT:
			if (begin < end) {
			}
			else {
				begin--;
			}
			end = begin;
			break;
		case GLFW_KEY_RIGHT:
			if (begin < end) {
				begin = end;
			}
			else {
				begin++;
			}
			end = begin;
			break;
		case GLFW_KEY_HOME:
			end = begin = 0;
			break;
		case GLFW_KEY_END:
			end = begin = text.size();
			break;
		case GLFW_KEY_V:
			if (windowIsModPressed()) {
				const char *newText = glfwGetClipboardString(gWindow);
				if (newText)
					insertText(newText);
			}
			break;
		case GLFW_KEY_X:
			if (windowIsModPressed()) {
				if (begin < end) {
					std::string selectedText = text.substr(begin, end - begin);
					glfwSetClipboardString(gWindow, selectedText.c_str());
					insertText("");
				}
			}
			break;
		case GLFW_KEY_C:
			if (windowIsModPressed()) {
				if (begin < end) {
					std::string selectedText = text.substr(begin, end - begin);
					glfwSetClipboardString(gWindow, selectedText.c_str());
				}
			}
			break;
		case GLFW_KEY_A:
			if (windowIsModPressed()) {
				begin = 0;
				end = text.size();
			}
			break;
		case GLFW_KEY_ENTER:
			if (multiline) {
				insertText("\n");
			}
			else {
				EventAction e;
				onAction(e);
			}
			break;
	}

	begin = clamp(begin, 0, text.size());
	end = clamp(end, 0, text.size());
	e.consumed = true;
}

void TextField::insertText(std::string text) {
	if (begin < end)
		this->text.erase(begin, end - begin);
	this->text.insert(begin, text);
	begin += text.size();
	end = begin;
	onTextChange();
}

void TextField::setText(std::string text) {
	this->text = text;
	begin = clamp(begin, 0, text.size());
	end = clamp(end, 0, text.size());
	onTextChange();
}

int TextField::getTextPosition(Vec mousePos) {
	return bndTextFieldTextPosition(gVg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str(), mousePos.x, mousePos.y);
}


} // namespace rack
