#pragma once
#include <app/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <widget/FramebufferWidget.hpp>
#include <app/CircularShadow.hpp>
#include <widget/SvgWidget.hpp>


namespace rack {
namespace app {


struct SvgButton : widget::OpaqueWidget {
	widget::FramebufferWidget* fb;
	CircularShadow* shadow;
	widget::SvgWidget* sw;
	std::vector<std::shared_ptr<window::Svg>> frames;

	SvgButton();
	void addFrame(std::shared_ptr<window::Svg> svg);
	void onButton(const ButtonEvent& e) override;
	void onDragStart(const DragStartEvent& e) override;
	void onDragEnd(const DragEndEvent& e) override;
	void onDragDrop(const DragDropEvent& e) override;
};


DEPRECATED typedef SvgButton SVGButton;


} // namespace app
} // namespace rack
