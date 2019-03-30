#pragma once
#include "ui/common.hpp"
#include "widget/Widget.hpp"
#include "Quantity.hpp"


namespace rack {
namespace ui {


struct ProgressBar : widget::Widget {
	/** Not owned. Stores the progress value and label. */
	Quantity *quantity = NULL;

	ProgressBar();
	void draw(const DrawArgs &args) override;
};


} // namespace ui
} // namespace rack
