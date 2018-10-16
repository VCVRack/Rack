#pragma once
#include "app/common.hpp"


namespace rack {


struct RackScene : Scene {
	ScrollWidget *scrollWidget;
	ZoomWidget *zoomWidget;

	RackScene();
	void step() override;
	void draw(NVGcontext *vg) override;
	void onHoverKey(event::HoverKey &e) override;
	void onPathDrop(event::PathDrop &e) override;
};


} // namespace rack
