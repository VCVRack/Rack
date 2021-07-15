#pragma once
#include <app/common.hpp>
#include <widget/TransparentWidget.hpp>


namespace rack {
namespace app {


struct RailWidget : widget::TransparentWidget {
	std::shared_ptr<Svg> svg;

	RailWidget();
	void draw(const DrawArgs& args) override;
	math::Vec getTileSize();
};


} // namespace app
} // namespace rack
