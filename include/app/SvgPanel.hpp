#pragma once
#include <app/common.hpp>
#include <widget/TransparentWidget.hpp>
#include <widget/FramebufferWidget.hpp>
#include <widget/SvgWidget.hpp>
#include <app.hpp>


namespace rack {
namespace app {


struct PanelBorder : widget::TransparentWidget {
	void draw(const DrawArgs& args) override;
};


struct SvgPanel : widget::FramebufferWidget {
	void step() override;
	void setBackground(std::shared_ptr<Svg> svg);
};


DEPRECATED typedef SvgPanel SVGPanel;


} // namespace app
} // namespace rack
