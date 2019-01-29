#pragma once
#include "app/common.hpp"
#include "ui/ScrollWidget.hpp"


namespace rack {
namespace app {


struct RackScrollWidget : ui::ScrollWidget {
	void step() override;
	void draw(const widget::DrawContext &ctx) override;
};


} // namespace app
} // namespace rack
