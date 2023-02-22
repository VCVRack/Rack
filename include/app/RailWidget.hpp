#pragma once
#include <app/common.hpp>
#include <widget/TransparentWidget.hpp>


namespace rack {
namespace app {


struct RailWidget : widget::TransparentWidget {
	struct Internal;
	Internal* internal;

	RailWidget();
	~RailWidget();
	void step() override;
	void draw(const DrawArgs& args) override;
};


} // namespace app
} // namespace rack
