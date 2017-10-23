#include "widgets.hpp"
// for gVg
#include "gui.hpp"
// for key codes
#include <GLFW/glfw3.h>


namespace rack {


void TextField::draw(NVGcontext *vg) {
	BNDwidgetState state;
	if (this == gFocusedWidget)
		state = BND_ACTIVE;
	else if (this == gHoveredWidget)
		state = BND_HOVER;
	else
		state = BND_DEFAULT;

	bndTextField(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str(), begin, end);
}

Widget *TextField::onMouseDown(Vec pos, int button) {
	end = begin = bndTextFieldTextPosition(gVg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str(), pos.x, pos.y);
	return OpaqueWidget::onMouseDown(pos, button);
}


bool TextField::onFocusText(int codepoint) {
	char c = codepoint;
	std::string newText(1, c);
	insertText(newText);
	return true;
}

bool TextField::onFocusKey(int key) {
	switch (key) {
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
			if (guiIsModPressed()) {
				const char *newText = glfwGetClipboardString(gWindow);
				if (newText)
					insertText(newText);
			}
			break;
		case GLFW_KEY_C:
			if (guiIsModPressed()) {
				if (begin < end) {
					std::string selectedText = text.substr(begin, end - begin);
					glfwSetClipboardString(gWindow, selectedText.c_str());
				}
			}
			break;
		case GLFW_KEY_ENTER:
			if (multiline) {
				insertText("\n");
			}
			else {
				onAction();
			}
			break;
	}

	begin = mini(maxi(begin, 0), text.size());
	end = mini(maxi(end, 0), text.size());
	return true;
}

bool TextField::onFocus() {
	begin = 0;
	end = text.size();
	return true;
}

void TextField::insertText(std::string newText) {
	if (begin < end)
		text.erase(begin, end - begin);
	text.insert(begin, newText);
	begin += newText.size();
	end = begin;
	onTextChange();
}


} // namespace rack
