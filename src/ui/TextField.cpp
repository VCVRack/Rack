#include <ui/TextField.hpp>

namespace rack {
namespace ui {


TextField::TextField() {
	box.size.y = BND_WIDGET_HEIGHT;
}

void TextField::draw(const DrawArgs& args) {
	nvgScissor(args.vg, RECT_ARGS(args.clipBox));

	BNDwidgetState state;
	if (this == APP->event->selectedWidget)
		state = BND_ACTIVE;
	else if (this == APP->event->hoveredWidget)
		state = BND_HOVER;
	else
		state = BND_DEFAULT;

	int begin = std::min(cursor, selection);
	int end = std::max(cursor, selection);
	bndTextField(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str(), begin, end);
	// Draw placeholder text
	if (text.empty()) {
		bndIconLabelCaret(args.vg, 0.0, 0.0, box.size.x, box.size.y, -1, bndGetTheme()->textFieldTheme.itemColor, 13, placeholder.c_str(), bndGetTheme()->textFieldTheme.itemColor, 0, -1);
	}

	nvgResetScissor(args.vg);
}

void TextField::onDragHover(const event::DragHover& e) {
	OpaqueWidget::onDragHover(e);

	if (e.origin == this) {
		int pos = getTextPosition(e.pos);
		cursor = pos;
	}
}

void TextField::onButton(const event::Button& e) {
	OpaqueWidget::onButton(e);

	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
		cursor = selection = getTextPosition(e.pos);
	}
}

void TextField::onSelectText(const event::SelectText& e) {
	if (e.codepoint < 128) {
		std::string newText(1, (char) e.codepoint);
		insertText(newText);
	}
	e.consume(this);
}

void TextField::onSelectKey(const event::SelectKey& e) {
	auto cursorToPrevWord = [&]() {
		size_t pos = text.rfind(' ', std::max(cursor - 2, 0));
		if (pos == std::string::npos)
			cursor = 0;
		else
			cursor = std::min((int) pos + 1, (int) text.size());
	};
	auto cursorToNextWord = [&]() {
		size_t pos = text.find(' ', std::min(cursor + 1, (int) text.size()));
		if (pos == std::string::npos)
			pos = text.size();
		cursor = pos;
	};

	if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		// Backspace
		if (e.key == GLFW_KEY_BACKSPACE && (e.mods & RACK_MOD_MASK) == 0) {
			if (cursor == selection) {
				cursor = std::max(cursor - 1, 0);
			}
			insertText("");
			e.consume(this);
		}
		// Ctrl+Backspace
		if (e.key == GLFW_KEY_BACKSPACE && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			if (cursor == selection) {
				cursorToPrevWord();
			}
			insertText("");
			e.consume(this);
		}
		// Delete
		if (e.key == GLFW_KEY_DELETE && (e.mods & RACK_MOD_MASK) == 0) {
			if (cursor == selection) {
				cursor = std::min(cursor + 1, (int) text.size());
			}
			insertText("");
			e.consume(this);
		}
		// Ctrl+Delete
		if (e.key == GLFW_KEY_DELETE && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			if (cursor == selection) {
				cursorToNextWord();
			}
			insertText("");
			e.consume(this);
		}
		// Left
		if (e.key == GLFW_KEY_LEFT) {
			if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
				cursorToPrevWord();
			}
			else {
				cursor = std::max(cursor - 1, 0);
			}
			if (!(e.mods & GLFW_MOD_SHIFT)) {
				selection = cursor;
			}
			e.consume(this);
		}
		// Right
		if (e.key == GLFW_KEY_RIGHT) {
			if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
				cursorToNextWord();
			}
			else {
				cursor = std::min(cursor + 1, (int) text.size());
			}
			if (!(e.mods & GLFW_MOD_SHIFT)) {
				selection = cursor;
			}
			e.consume(this);
		}
		// Home
		if (e.key == GLFW_KEY_HOME && (e.mods & RACK_MOD_MASK) == 0) {
			selection = cursor = 0;
			e.consume(this);
		}
		// Shift+Home
		if (e.key == GLFW_KEY_HOME && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			cursor = 0;
			e.consume(this);
		}
		// End
		if (e.key == GLFW_KEY_END && (e.mods & RACK_MOD_MASK) == 0) {
			selection = cursor = text.size();
			e.consume(this);
		}
		// Shift+End
		if (e.key == GLFW_KEY_END && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			cursor = text.size();
			e.consume(this);
		}
		// Ctrl+V
		if (e.keyName == "v" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			const char* newText = glfwGetClipboardString(APP->window->win);
			if (newText)
				insertText(newText);
			e.consume(this);
		}
		// Ctrl+C
		if (e.keyName == "x" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			if (cursor != selection) {
				int begin = std::min(cursor, selection);
				std::string selectedText = text.substr(begin, std::abs(selection - cursor));
				glfwSetClipboardString(APP->window->win, selectedText.c_str());
				insertText("");
			}
			e.consume(this);
		}
		// Ctrl+C
		if (e.keyName == "c" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			if (cursor != selection) {
				int begin = std::min(cursor, selection);
				std::string selectedText = text.substr(begin, std::abs(selection - cursor));
				glfwSetClipboardString(APP->window->win, selectedText.c_str());
			}
			e.consume(this);
		}
		// Ctrl+A
		if (e.keyName == "a" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			selectAll();
			e.consume(this);
		}
		// Enter
		if ((e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER) && (e.mods & RACK_MOD_MASK) == 0) {
			if (multiline) {
				insertText("\n");
			}
			else {
				event::Action eAction;
				onAction(eAction);
			}
			e.consume(this);
		}
		// Consume all printable keys
		if (e.keyName != "") {
			e.consume(this);
		}
		// Esc
		if (e.key == GLFW_KEY_ESCAPE && (e.mods & RACK_MOD_MASK) == 0) {
			APP->event->setSelected(NULL);
			e.consume(this);
		}

		assert(0 <= cursor);
		assert(cursor <= (int) text.size());
		assert(0 <= selection);
		assert(selection <= (int) text.size());
	}

	// Consume ALL keys with ALL actions
	e.consume(this);
}

void TextField::insertText(std::string text) {
	bool changed = false;
	if (cursor != selection) {
		// Delete selected text
		int begin = std::min(cursor, selection);
		this->text.erase(begin, std::abs(selection - cursor));
		cursor = selection = begin;
		changed = true;
	}
	if (!text.empty()) {
		this->text.insert(cursor, text);
		cursor += text.size();
		selection = cursor;
		changed = true;
	}
	if (changed) {
		event::Change eChange;
		onChange(eChange);
	}
}

void TextField::setText(std::string text) {
	if (this->text != text) {
		this->text = text;
		// event::Change
		event::Change eChange;
		onChange(eChange);
	}
	selection = cursor = text.size();
}

void TextField::selectAll() {
	cursor = text.size();
	selection = 0;
}

int TextField::getTextPosition(math::Vec mousePos) {
	return bndTextFieldTextPosition(APP->window->vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str(), mousePos.x, mousePos.y);
}


} // namespace ui
} // namespace rack
