#pragma once
#include "app/common.hpp"
#include "widget/TransparentWidget.hpp"


namespace rack {
namespace app {


struct RackRail : widget::TransparentWidget {
	void draw(const widget::DrawContext &ctx) override;
};


} // namespace app
} // namespace rack
