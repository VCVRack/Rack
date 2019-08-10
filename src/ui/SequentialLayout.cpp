#include <ui/SequentialLayout.hpp>
#include <vector>


namespace rack {
namespace ui {


#define X(v) (orientation == HORIZONTAL_ORIENTATION ? (v).x : (v).y)
#define Y(v) (orientation == HORIZONTAL_ORIENTATION ? (v).y : (v).x)


void SequentialLayout::step() {
	Widget::step();

	// Sort widgets into rows (or columns if vertical)
	std::vector<std::vector<widget::Widget*>> rows;
	rows.resize(1);
	float rowWidth = 0.0;
	for (widget::Widget* child : children) {
		if (!child->visible)
			continue;

		// Should we wrap the widget now?
		if (!rows.back().empty() && rowWidth + X(child->box.size) >= X(box.size)) {
			rowWidth = 0.0;
			rows.resize(rows.size() + 1);
		}

		rows.back().push_back(child);
		rowWidth += X(child->box.size) + X(spacing);
	}

	// Position widgets
	math::Vec p;
	for (auto& row : rows) {
		// For center and right alignment, compute offset from the left margin
		float offset = 0.0;
		if (alignment != LEFT_ALIGNMENT) {
			float rowWidth = 0.0;
			for (widget::Widget* child : row) {
				rowWidth += X(child->box.size) + X(spacing);
			}
			rowWidth -= X(spacing);

			if (alignment == CENTER_ALIGNMENT)
				offset = (X(box.size) - rowWidth) / 2;
			else if (alignment == RIGHT_ALIGNMENT)
				offset = X(box.size) - rowWidth;
		}

		float maxHeight = 0.0;
		for (widget::Widget* child : row) {
			child->box.pos = p;
			X(child->box.pos) += offset;

			X(p) += X(child->box.size) + X(spacing);
			if (Y(child->box.size) > maxHeight)
				maxHeight = Y(child->box.size);
		}
		X(p) = 0.0;
		Y(p) += maxHeight + Y(spacing);
	}
}


} // namespace ui
} // namespace rack
