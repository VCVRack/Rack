#pragma once
#include "ui/common.hpp"
#include "widget/Widget.hpp"
#include "ui/Quantity.hpp"


namespace rack {
namespace ui {


struct ProgressBar : widget::Widget {
	Quantity *quantity = NULL;

	ProgressBar();
	~ProgressBar();
	void draw(const DrawArgs &args) override;
};


} // namespace ui
} // namespace rack
