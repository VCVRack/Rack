#pragma once
#include "app/common.hpp"
#include "widget/OpaqueWidget.hpp"
#include "widget/FramebufferWidget.hpp"
#include "widget/SVGWidget.hpp"


namespace rack {
namespace app {


struct SVGButton : widget::OpaqueWidget {
	widget::FramebufferWidget *fb;
	widget::SVGWidget *sw;
	std::vector<std::shared_ptr<SVG>> frames;

	SVGButton();
	void addFrame(std::shared_ptr<SVG> svg);
	void onDragStart(const event::DragStart &e) override;
	void onDragEnd(const event::DragEnd &e) override;
	void onDragDrop(const event::DragDrop &e) override;
};


} // namespace app
} // namespace rack
