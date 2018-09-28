#pragma once

#include "widgets.hpp"


namespace rack {


/** Positions children in a row/column based on their widths/heights */
struct SequentialLayout : virtual EventWidget {
	enum Orientation {
		HORIZONTAL_ORIENTATION,
		VERTICAL_ORIENTATION,
	};
	Orientation orientation = HORIZONTAL_ORIENTATION;
	enum Alignment {
		LEFT_ALIGNMENT,
		CENTER_ALIGNMENT,
		RIGHT_ALIGNMENT,
	};
	Alignment alignment = LEFT_ALIGNMENT;
	/** Space between adjacent elements */
	float spacing = 0.0;

	void step() override {
		Widget::step();

		float offset = 0.0;
		for (Widget *child : children) {
			if (!child->visible)
				continue;
			// Set position
			(orientation == HORIZONTAL_ORIENTATION ? child->box.pos.x : child->box.pos.y) = offset;
			// Increment by size
			offset += (orientation == HORIZONTAL_ORIENTATION ? child->box.size.x : child->box.size.y);
			offset += spacing;
		}

		// We're done if left aligned
		if (alignment == LEFT_ALIGNMENT)
			return;

		// Adjust positions based on width of the layout itself
		offset -= spacing;
		if (alignment == RIGHT_ALIGNMENT)
			offset -= (orientation == HORIZONTAL_ORIENTATION ? box.size.x : box.size.y);
		else if (alignment == CENTER_ALIGNMENT)
			offset -= (orientation == HORIZONTAL_ORIENTATION ? box.size.x : box.size.y) / 2.0;
		for (Widget *child : children) {
			if (!child->visible)
				continue;
			(orientation == HORIZONTAL_ORIENTATION ? child->box.pos.x : child->box.pos.y) += offset;
		}
	}
};


} // namespace rack
