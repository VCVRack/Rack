#pragma once
#include <app/common.hpp>
#include <app/PortWidget.hpp>
#include <widget/FramebufferWidget.hpp>
#include <widget/SvgWidget.hpp>
#include <app/CircularShadow.hpp>
#include <settings.hpp>


namespace rack {
namespace app {


struct SvgPort : PortWidget {
	widget::FramebufferWidget* fb;
	CircularShadow* shadow;
	widget::SvgWidget* sw;

	SvgPort();
	void setSvg(std::shared_ptr<window::Svg> svg);
	DEPRECATED void setSVG(std::shared_ptr<window::Svg> svg) {
		setSvg(svg);
	}
};


DEPRECATED typedef SvgPort SVGPort;


struct ThemedSvgPort : SvgPort {
	std::shared_ptr<window::Svg> lightSvg;
	std::shared_ptr<window::Svg> darkSvg;

	void setSvg(std::shared_ptr<window::Svg> lightSvg, std::shared_ptr<window::Svg> darkSvg) {
		this->lightSvg = lightSvg;
		this->darkSvg = darkSvg;
		SvgPort::setSvg(settings::preferDarkPanels ? darkSvg : lightSvg);
	}

	void step() override {
		SvgPort::setSvg(settings::preferDarkPanels ? darkSvg : lightSvg);
		SvgPort::step();
	}
};


} // namespace app
} // namespace rack
