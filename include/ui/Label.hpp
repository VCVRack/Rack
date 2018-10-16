#pragma once
#include "ui/common.hpp"


namespace rack {


struct Label : virtual Widget {
	std::string text;
	float fontSize;
	NVGcolor color;
	enum Alignment {
		LEFT_ALIGNMENT,
		CENTER_ALIGNMENT,
		RIGHT_ALIGNMENT,
	};
	Alignment alignment = LEFT_ALIGNMENT;

	Label() {
		box.size.y = BND_WIDGET_HEIGHT;
		fontSize = 13;
		color = bndGetTheme()->regularTheme.textColor;
	}

	void draw(NVGcontext *vg) override {
		// TODO
		// Custom font sizes do not work with right or center alignment
		float x;
		switch (alignment) {
			default:
			case LEFT_ALIGNMENT: {
				x = 0.0;
			} break;
			case RIGHT_ALIGNMENT: {
				x = box.size.x - bndLabelWidth(vg, -1, text.c_str());
			} break;
			case CENTER_ALIGNMENT: {
				x = (box.size.x - bndLabelWidth(vg, -1, text.c_str())) / 2.0;
			} break;
		}

		bndIconLabelValue(vg, x, 0.0, box.size.x, box.size.y, -1, color, BND_LEFT, fontSize, text.c_str(), NULL);
	}
};


} // namespace rack
