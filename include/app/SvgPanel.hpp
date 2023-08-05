#pragma once
#include <app/common.hpp>
#include <widget/TransparentWidget.hpp>
#include <widget/FramebufferWidget.hpp>
#include <widget/SvgWidget.hpp>
#include <settings.hpp>


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


struct ThemedSvgPanel : SvgPanel {
	std::shared_ptr<window::Svg> lightSvg;
	std::shared_ptr<window::Svg> darkSvg;

	void setBackground(std::shared_ptr<window::Svg> lightSvg, std::shared_ptr<window::Svg> darkSvg) {
		this->lightSvg = lightSvg;
		this->darkSvg = darkSvg;
		SvgPanel::setBackground(settings::preferDarkPanels ? darkSvg : lightSvg);
	}

	void step() override {
		SvgPanel::setBackground(settings::preferDarkPanels ? darkSvg : lightSvg);
		SvgPanel::step();
	}
};


} // namespace app
} // namespace rack
