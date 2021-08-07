#include <vector>

#include <ui/SequentialLayout.hpp>


namespace rack {
namespace ui {


/** We assume horizontal orientation in this file, but we can achieve vertical orientation just by swapping the axes.
*/
#define X(v) (orientation == HORIZONTAL_ORIENTATION ? (v).x : (v).y)
#define Y(v) (orientation == HORIZONTAL_ORIENTATION ? (v).y : (v).x)


void SequentialLayout::step() {
	Widget::step();

	float boundWidth = X(box.size) - 2 * X(margin);

	// Sort widgets into rows (or columns if vertical)
	std::vector<widget::Widget*> row;
	math::Vec cursor = margin;
	auto flushRow = [&]() {
		// For center and right alignment, compute offset from the left margin
		if (alignment != LEFT_ALIGNMENT) {
			float rowWidth = 0.f;
			for (widget::Widget* child : row) {
				rowWidth += X(child->box.size) + X(spacing);
			}
			rowWidth -= X(spacing);

			if (alignment == CENTER_ALIGNMENT)
				X(cursor) += (boundWidth - rowWidth) / 2;
			else if (alignment == RIGHT_ALIGNMENT)
				X(cursor) += boundWidth - rowWidth;
		}

		// Set positions of widgets
		float maxHeight = 0.f;
		for (widget::Widget* child : row) {
			child->box.pos = cursor;
			X(cursor) += X(child->box.size) + X(spacing);

			if (Y(child->box.size) > maxHeight)
				maxHeight = Y(child->box.size);
		}
		row.clear();

		// Reset cursor to next line
		X(cursor) = X(margin);
		Y(cursor) += maxHeight + Y(spacing);
	};

	// Iterate through children until row is full
	float rowWidth = 0.0;
	for (widget::Widget* child : children) {
		// Skip invisible children
		if (!child->isVisible()) {
			child->box.pos = math::Vec();
			continue;
		}

		// Should we wrap the widget now?
		if (wrap && !row.empty() && rowWidth + X(child->box.size) > boundWidth) {
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

	Y(box.size) = Y(cursor) - Y(spacing) + Y(margin);
}


} // namespace ui
} // namespace rack
