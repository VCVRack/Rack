#pragma once
#include <app/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <widget/FramebufferWidget.hpp>
#include <app/CircularShadow.hpp>
#include <widget/SvgWidget.hpp>


namespace rack {
namespace app {


struct SvgButton : widget::OpaqueWidget {
	widget::FramebufferWidget *fb;
	CircularShadow *shadow;
	widget::SvgWidget *sw;
	std::vector<std::shared_ptr<Svg>> frames;

	SvgButton();
	void addFrame(std::shared_ptr<Svg> svg);
	void onDragStart(const event::DragStart &e) override;
	void onDragEnd(const event::DragEnd &e) override;
	void onDragDrop(const event::DragDrop &e) override;
};


DEPRECATED typedef SvgButton SVGButton;


} // namespace app
} // namespace rack
