#pragma once
#include <widget/Widget.hpp>
#include <ui/common.hpp>


namespace rack {
namespace ui {


/** Positions children with a margin between the layout's box. */
struct MarginLayout : widget::Widget {
	math::Vec margin;

	void step() override;
};


} // namespace ui
} // namespace rack
