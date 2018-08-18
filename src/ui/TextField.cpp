#include "global_pre.hpp"
#include "ui.hpp"
// for gVg
#include "window.hpp"
// for key codes
#include <GLFW/glfw3.h>
#include "global_ui.hpp"


namespace rack {

extern bool b_touchkeyboard_enable;

void TextField::draw(NVGcontext *vg) {
	nvgScissor(vg, 0, 0, box.size.x, box.size.y);

	BNDwidgetState state;
	if (this == global_ui->widgets.gFocusedWidget)
		state = BND_ACTIVE;
	else if (this == global_ui->widgets.gHoveredWidget)
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

#ifdef RACK_HOST
   if(b_touchkeyboard_enable)
   {
      lglw_touchkeyboard_show(rack::global_ui->window.lglw, LGLW_TRUE);
   }
#endif // RACK_HOST
}

void TextField::onMouseMove(EventMouseMove &e) {
	if (this == global_ui->widgets.gDraggedWidget) {
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

		case LGLW_VKEY_BACKSPACE:
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
         break;

		case LGLW_VKEY_DELETE:
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
         break;

		case LGLW_VKEY_LEFT:
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
         break;

		case LGLW_VKEY_RIGHT:
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
         break;

		case LGLW_VKEY_HOME:
			selection = cursor = 0;
         break;

		case LGLW_VKEY_END:
			selection = cursor = text.size();
         break;

		case 'v':
#ifdef RACK_HOST
			if (windowIsModPressed()) {
#if 0
				const char *newText = glfwGetClipboardString(global_ui->window.gWindow);
				if (newText)
					insertText(newText);
#else
            char newText[16384];
            lglw_clipboard_text_get(global_ui->window.lglw, 16384u/*maxChars*/, NULL/*retNumChars*/, newText);
            insertText(newText);
#endif
			}
#endif // RACK_HOST
         break;

		case 'x':
#ifdef RACK_HOST
			if (windowIsModPressed()) {
				if (cursor != selection) {
					int begin = min(cursor, selection);
					std::string selectedText = text.substr(begin, std::abs(selection - cursor));
#if 0
					glfwSetClipboardString(global_ui->window.gWindow, selectedText.c_str());
#else
               lglw_clipboard_text_set(global_ui->window.lglw, 0u/*numChars=auto*/, selectedText.c_str());
#endif
					insertText("");
				}
			}
#endif // RACK_HOST
         break;

		case 'c':
#ifdef RACK_HOST
			if (windowIsModPressed()) {
				if (cursor != selection) {
					int begin = min(cursor, selection);
					std::string selectedText = text.substr(begin, std::abs(selection - cursor));
#if 0
					glfwSetClipboardString(global_ui->window.gWindow, selectedText.c_str());
#else
               lglw_clipboard_text_set(global_ui->window.lglw, 0u/*numChars=auto*/, selectedText.c_str());
#endif
				}
			}
#endif // RACK_HOST
         break;

		case 'a':
			if (windowIsModPressed()) {
				selection = 0;
				cursor = text.size();
			}
         break;

		case LGLW_VKEY_RETURN:
         // printf("xxx TextField::onKey: RETURN\n");
			if (multiline) {
				insertText("\n");
			}
			else {
				EventAction e;
#ifdef RACK_HOST
            if(b_touchkeyboard_enable)
            {
               lglw_touchkeyboard_show(rack::global_ui->window.lglw, LGLW_FALSE);
            }
#endif // RACK_HOST
				onAction(e);
			}
         break;
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
	return bndTextFieldTextPosition(global_ui->window.gVg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str(), mousePos.x, mousePos.y);
}


} // namespace rack
