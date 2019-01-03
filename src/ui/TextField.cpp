#include "ui/TextField.hpp"

namespace rack {


TextField::TextField() {
	box.size.y = BND_WIDGET_HEIGHT;
}

void TextField::draw(NVGcontext *vg) {
	nvgScissor(vg, 0, 0, box.size.x, box.size.y);

	BNDwidgetState state;
	if (this == context()->event->selectedWidget)
		state = BND_ACTIVE;
	else if (this == context()->event->hoveredWidget)
		state = BND_HOVER;
	else
		state = BND_DEFAULT;

	int begin = std::min(cursor, selection);
	int end = std::max(cursor, selection);
	bndTextField(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str(), begin, end);
	// Draw placeholder text
	if (text.empty() && state != BND_ACTIVE) {
		bndIconLabelCaret(vg, 0.0, 0.0, box.size.x, box.size.y, -1, bndGetTheme()->textFieldTheme.itemColor, 13, placeholder.c_str(), bndGetTheme()->textFieldTheme.itemColor, 0, -1);
	}

	nvgResetScissor(vg);
}

void TextField::onButton(const event::Button &e) {
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
		cursor = selection = getTextPosition(e.pos);
	}
	OpaqueWidget::onButton(e);
}

void TextField::onHover(const event::Hover &e) {
	if (this == context()->event->draggedWidget) {
		int pos = getTextPosition(e.pos);
		if (pos != selection) {
			cursor = pos;
		}
	}
	OpaqueWidget::onHover(e);
}

void TextField::onEnter(const event::Enter &e) {
	e.consume(this);
}

void TextField::onSelectText(const event::SelectText &e) {
	if (e.codepoint < 128) {
		std::string newText(1, (char) e.codepoint);
		insertText(newText);
	}
	e.consume(this);
}

void TextField::onSelectKey(const event::SelectKey &e) {
	if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		switch (e.key) {
			case GLFW_KEY_BACKSPACE: {
				if (cursor == selection) {
					cursor--;
					if (cursor >= 0) {
						text.erase(cursor, 1);
						event::Change eChange;
						onChange(eChange);
					}
					selection = cursor;
				}
				else {
					int begin = std::min(cursor, selection);
					text.erase(begin, std::abs(selection - cursor));
					event::Change eChange;
					onChange(eChange);
					cursor = selection = begin;
				}
			} break;
			case GLFW_KEY_DELETE: {
				if (cursor == selection) {
					text.erase(cursor, 1);
					event::Change eChange;
					onChange(eChange);
				}
				else {
					int begin = std::min(cursor, selection);
					text.erase(begin, std::abs(selection - cursor));
					event::Change eChange;
					onChange(eChange);
					cursor = selection = begin;
				}
			} break;
			case GLFW_KEY_LEFT: {
				if (context()->window->isModPressed()) {
					while (--cursor > 0) {
						if (text[cursor] == ' ')
							break;
					}
				}
				else {
					cursor--;
				}
				if (!context()->window->isShiftPressed()) {
					selection = cursor;
				}
			} break;
			case GLFW_KEY_RIGHT: {
				if (context()->window->isModPressed()) {
					while (++cursor < (int) text.size()) {
						if (text[cursor] == ' ')
							break;
					}
				}
				else {
					cursor++;
				}
				if (!context()->window->isShiftPressed()) {
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
				if (context()->window->isModPressed()) {
					const char *newText = glfwGetClipboardString(context()->window->win);
					if (newText)
						insertText(newText);
				}
			} break;
			case GLFW_KEY_X: {
				if (context()->window->isModPressed()) {
					if (cursor != selection) {
						int begin = std::min(cursor, selection);
						std::string selectedText = text.substr(begin, std::abs(selection - cursor));
						glfwSetClipboardString(context()->window->win, selectedText.c_str());
						insertText("");
					}
				}
			} break;
			case GLFW_KEY_C: {
				if (context()->window->isModPressed()) {
					if (cursor != selection) {
						int begin = std::min(cursor, selection);
						std::string selectedText = text.substr(begin, std::abs(selection - cursor));
						glfwSetClipboardString(context()->window->win, selectedText.c_str());
					}
				}
			} break;
			case GLFW_KEY_A: {
				if (context()->window->isModPressed()) {
					selection = 0;
					cursor = text.size();
				}
			} break;
			case GLFW_KEY_ENTER: {
				if (multiline) {
					insertText("\n");
				}
				else {
					event::Action eAction;
					onAction(eAction);
				}
			} break;
		}

		cursor = math::clamp(cursor, 0, (int) text.size());
		selection = math::clamp(selection, 0, (int) text.size());
		e.consume(this);
	}
}

/** Inserts text at the cursor, replacing the selection if necessary */
void TextField::insertText(std::string text) {
	if (cursor != selection) {
		int begin = std::min(cursor, selection);
		this->text.erase(begin, std::abs(selection - cursor));
		cursor = selection = begin;
	}
	this->text.insert(cursor, text);
	cursor += text.size();
	selection = cursor;
	event::Change eChange;
	onChange(eChange);
}

/** Replaces the entire text */
void TextField::setText(std::string text) {
	this->text = text;
	selection = cursor = text.size();
	event::Change eChange;
	onChange(eChange);
}

int TextField::getTextPosition(math::Vec mousePos) {
	return bndTextFieldTextPosition(context()->window->vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str(), mousePos.x, mousePos.y);
}


} // namespace rack
