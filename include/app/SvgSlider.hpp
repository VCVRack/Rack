#pragma once
#include <app/common.hpp>
#include <app/SliderKnob.hpp>
#include <widget/FramebufferWidget.hpp>
#include <widget/SvgWidget.hpp>


namespace rack {
namespace app {


/** Behaves like a knob but linearly moves an widget::SvgWidget between two points.
Can be used for horizontal or vertical linear faders.
*/
struct SvgSlider : app::SliderKnob {
	widget::FramebufferWidget* fb;
	widget::SvgWidget* background;
	widget::SvgWidget* handle;
	/** Intermediate positions will be interpolated between these positions */
	math::Vec minHandlePos, maxHandlePos;

	SvgSlider();
	void setBackgroundSvg(std::shared_ptr<window::Svg> svg);
	void setHandleSvg(std::shared_ptr<window::Svg> svg);
	void setHandlePos(math::Vec minHandlePos, math::Vec maxHandlePos);
	void setHandlePosCentered(math::Vec minHandlePosCentered, math::Vec maxHandlePosCentered);
	void onChange(const ChangeEvent& e) override;

	DEPRECATED void setBackgroundSVG(std::shared_ptr<window::Svg> svg) {
		setBackgroundSvg(svg);
	}
	DEPRECATED void setHandleSVG(std::shared_ptr<window::Svg> svg) {
		setBackgroundSvg(svg);
	}
	DEPRECATED void setSVGs(std::shared_ptr<window::Svg> backgroundSvg, std::shared_ptr<window::Svg> handleSvg) {
		setBackgroundSvg(backgroundSvg);
		setHandleSvg(handleSvg);
	}
};


DEPRECATED typedef SvgSlider SVGSlider;


} // namespace app
} // namespace rack
