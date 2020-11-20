#include <vector>

#include <ui/SequentialLayout.hpp>


namespace rack {
namespace ui {


#define X(v) (orientation == HORIZONTAL_ORIENTATION ? (v).x : (v).y)
#define Y(v) (orientation == HORIZONTAL_ORIENTATION ? (v).y : (v).x)


void SequentialLayout::step() {
	Widget::step();

	math::Rect bound;
	bound.pos = margin;
	bound.size = box.size.minus(margin.mult(2));

	// Sort widgets into rows (or columns if vertical)
	std::vector<widget::Widget*> row;
	math::Vec cursor = bound.pos;
	auto flushRow = [&]() {
		// For center and right alignment, compute offset from the left margin
		float offset = 0.f;
		if (alignment != LEFT_ALIGNMENT) {
			float rowWidth = 0.f;
			for (widget::Widget* child : row) {
				rowWidth += X(child->box.size) + X(spacing);
			}
			rowWidth -= X(spacing);

			if (alignment == CENTER_ALIGNMENT)
				offset = (X(bound.size) - rowWidth) / 2;
			else if (alignment == RIGHT_ALIGNMENT)
				offset = X(bound.size) - rowWidth;
		}

		// Set positions of widgets
		float maxHeight = 0.f;
		for (widget::Widget* child : row) {
			child->box.pos = cursor;
			X(child->box.pos) += offset;
			X(cursor) += X(child->box.size) + X(spacing);

			if (Y(child->box.size) > maxHeight)
				maxHeight = Y(child->box.size);
		}
		row.clear();

		// Reset cursor to next line
		X(cursor) = X(bound.pos);
		Y(cursor) += maxHeight + Y(spacing);
	};

	// Iterate through children until row is full
	float rowWidth = 0.0;
	for (widget::Widget* child : children) {
		if (!child->isVisible()) {
			child->box.pos = math::Vec();
			continue;
		}

		// Should we wrap the widget now?
		if (!row.empty() && rowWidth + X(child->box.size) > X(bound.size)) {
			flushRow();
			rowWidth = 0.0;
		}

		row.push_back(child);
		rowWidth += X(child->box.size) + X(spacing);
	}

	// Flush last row
	if (!row.empty()) {
		flushRow();
	}
}


} // namespace ui
} // namespace rack
