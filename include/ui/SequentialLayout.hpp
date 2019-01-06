#pragma once
#include "widgets/Widget.hpp"
#include "ui/common.hpp"


namespace rack {


/** Positions children in a row/column based on their widths/heights */
struct SequentialLayout : virtual Widget {
	enum Orientation {
		HORIZONTAL_ORIENTATION,
		VERTICAL_ORIENTATION,
	};
	enum Alignment {
		LEFT_ALIGNMENT,
		CENTER_ALIGNMENT,
		RIGHT_ALIGNMENT,
	};

	Orientation orientation = HORIZONTAL_ORIENTATION;
	Alignment alignment = LEFT_ALIGNMENT;
	/** Space between adjacent elements */
	math::Vec spacing;

	void step() override;
};


} // namespace rack
