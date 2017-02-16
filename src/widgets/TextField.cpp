#include "widgets.hpp"
// for gVg
#include "gui.hpp"
// for key codes
#include <GLFW/glfw3.h>


namespace rack {


void TextField::draw(NVGcontext *vg) {
	BNDwidgetState state;
	if (this == gSelectedWidget)
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


void TextField::onText(int codepoint) {
	if (begin < end)
		text.erase(begin, end - begin);
	char c = codepoint;
	text.insert(begin, &c, 1);
	begin++;
	end = begin;
}

void TextField::onKey(int key) {
	switch (key) {
		case GLFW_KEY_BACKSPACE:
			if (begin < end) {
				text.erase(begin, end - begin);
			}
			else {
				begin--;
				if (begin >= 0)
					text.erase(begin, 1);
			}
			end = begin;
			break;
		case GLFW_KEY_DELETE:
			if (begin < end) {
				text.erase(begin, end - begin);
			}
			else {
				text.erase(begin, 1);
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
	}

	begin = mini(maxi(begin, 0), text.size());
	end = mini(maxi(end, 0), text.size());
}

void TextField::onSelect() {
	begin = 0;
	end = text.size();
}


} // namespace rack
