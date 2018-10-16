#pragma once
#include "ui/Button.hpp"


namespace rack {


struct ChoiceButton : Button {
	void draw(NVGcontext *vg) override {
		bndChoiceButton(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str());
	}
};


} // namespace rack
