#pragma once
#include "app/common.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "widgets/FramebufferWidget.hpp"
#include "widgets/SVGWidget.hpp"


namespace rack {


struct SVGButton : OpaqueWidget {
	FramebufferWidget *fb;
	SVGWidget *sw;
	std::vector<std::shared_ptr<SVG>> frames;

	SVGButton();
	void addFrame(std::shared_ptr<SVG> svg);
	void onDragStart(const event::DragStart &e) override;
	void onDragEnd(const event::DragEnd &e) override;
	void onDragDrop(const event::DragDrop &e) override;
};


} // namespace rack
