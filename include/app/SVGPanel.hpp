#pragma once
#include "app/common.hpp"
#include "widget/TransparentWidget.hpp"
#include "widget/FramebufferWidget.hpp"
#include "widget/SVGWidget.hpp"
#include "app.hpp"


namespace rack {
namespace app {


struct PanelBorder : widget::TransparentWidget {
	void draw(const widget::DrawContext &ctx) override;
};


struct SVGPanel : widget::FramebufferWidget {
	void step() override;
	void setBackground(std::shared_ptr<SVG> svg);
};



} // namespace app
} // namespace rack
