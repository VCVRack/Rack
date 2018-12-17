#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "ui/common.hpp"
#include "event.hpp"
#include "context.hpp"


namespace rack {


struct TextField : OpaqueWidget {
	std::string text;
	std::string placeholder;
	bool multiline = false;
	/** The index of the text cursor */
	int cursor = 0;
	/** The index of the other end of the selection.
	If nothing is selected, this is equal to `cursor`.
	*/
	int selection = 0;

	TextField() {
		box.size.y = BND_WIDGET_HEIGHT;
	}

	void draw(NVGcontext *vg) override {
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

	void onButton(event::Button &e) override {
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			cursor = selection = getTextPosition(e.pos);
		}
		OpaqueWidget::onButton(e);
	}

	void onHover(event::Hover &e) override {
		if (this == context()->event->draggedWidget) {
			int pos = getTextPosition(e.pos);
			if (pos != selection) {
				cursor = pos;
			}
		}
		OpaqueWidget::onHover(e);
	}

	void onEnter(event::Enter &e) override {
		e.target = this;
	}

	void onSelectText(event::SelectText &e) override {
		if (e.codepoint < 128) {
			std::string newText(1, (char) e.codepoint);
			insertText(newText);
		}
		e.target = this;
	}

	void onSelectKey(event::SelectKey &e) override {
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
						int begin = std::min(cursor, selection);
						std::string selectedText = text.substr(begin, std::abs(selection - cursor));
						glfwSetClipboardString(gWindow, selectedText.c_str());
						insertText("");
					}
				}
			} break;
			case GLFW_KEY_C: {
				if (windowIsModPressed()) {
					if (cursor != selection) {
						int begin = std::min(cursor, selection);
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
					event::Action eAction;
					onAction(eAction);
				}
			} break;
		}

		cursor = clamp(cursor, 0, (int) text.size());
		selection = clamp(selection, 0, (int) text.size());
		e.target = this;
	}

	/** Inserts text at the cursor, replacing the selection if necessary */
	void insertText(std::string text) {
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
	void setText(std::string text) {
		this->text = text;
		selection = cursor = text.size();
		event::Change eChange;
		onChange(eChange);
	}

	virtual int getTextPosition(Vec mousePos) {
		return bndTextFieldTextPosition(gVg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str(), mousePos.x, mousePos.y);
	}
};


} // namespace rack
