#pragma once
#include "app/common.hpp"
#include "widget/OpaqueWidget.hpp"
#include "widget/FramebufferWidget.hpp"
#include "widget/SvgWidget.hpp"


namespace rack {
namespace app {


struct SvgButton : widget::OpaqueWidget {
	widget::FramebufferWidget *fb;
	widget::SvgWidget *sw;
	std::vector<std::shared_ptr<Svg>> frames;

	SvgButton();
	void addFrame(std::shared_ptr<Svg> svg);
	void onDragStart(const widget::DragStartEvent &e) override;
	void onDragEnd(const widget::DragEndEvent &e) override;
	void onDragDrop(const widget::DragDropEvent &e) override;
};


DEPRECATED typedef SvgButton SVGButton;


} // namespace app
} // namespace rack
