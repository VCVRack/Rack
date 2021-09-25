#pragma once
#include <app/common.hpp>
#include <widget/TransparentWidget.hpp>
#include <widget/FramebufferWidget.hpp>
#include <widget/SvgWidget.hpp>
#include <context.hpp>


namespace rack {
namespace app {


struct PanelBorder : widget::TransparentWidget {
	void draw(const DrawArgs& args) override;
};


struct SvgPanel : widget::Widget {
	widget::FramebufferWidget* fb;
	widget::SvgWidget* sw;
	PanelBorder* panelBorder;
	std::shared_ptr<window::Svg> svg;

	SvgPanel();
	void step() override;
	void setBackground(std::shared_ptr<window::Svg> svg);
};


DEPRECATED typedef SvgPanel SVGPanel;


} // namespace app
} // namespace rack
