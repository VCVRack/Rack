#pragma once
#include <widget/Widget.hpp>
#include <ui/common.hpp>


namespace rack {
namespace ui {


/** Positions children in a row/column based on their widths/heights */
struct SequentialLayout : widget::Widget {
	enum Orientation {
		HORIZONTAL_ORIENTATION,
		VERTICAL_ORIENTATION,
	};
	enum Alignment {
		LEFT_ALIGNMENT,
		CENTER_ALIGNMENT,
		RIGHT_ALIGNMENT,

		TOP_ALIGNMENT = LEFT_ALIGNMENT,
		MIDDLE_ALIGNMENT = CENTER_ALIGNMENT,
		BOTTOM_ALIGNMENT = RIGHT_ALIGNMENT,
	};

	Orientation orientation = HORIZONTAL_ORIENTATION;
	Alignment alignment = LEFT_ALIGNMENT;
	bool wrap = true;
	/** Space between box bounds. */
	math::Vec margin;
	/** Space between adjacent elements, and adjacent lines if wrapped. */
	math::Vec spacing;

	void step() override;
};


} // namespace ui
} // namespace rack
