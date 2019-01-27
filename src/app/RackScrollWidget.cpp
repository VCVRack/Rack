#include "app/RackScrollWidget.hpp"
#include "app/Scene.hpp"
#include "window.hpp"
#include "app.hpp"


namespace rack {


void RackScrollWidget::step() {
	math::Vec pos = app()->window->mousePos;
	math::Rect viewport = getViewport(box.zeroPos());
	// Scroll rack if dragging cable near the edge of the screen
	if (app()->scene->rackWidget->incompleteCable) {
		float margin = 20.0;
		float speed = 15.0;
		if (pos.x <= viewport.pos.x + margin)
			offset.x -= speed;
		if (pos.x >= viewport.pos.x + viewport.size.x - margin)
			offset.x += speed;
		if (pos.y <= viewport.pos.y + margin)
			offset.y -= speed;
		if (pos.y >= viewport.pos.y + viewport.size.y - margin)
			offset.y += speed;
	}
	ScrollWidget::step();
}


void RackScrollWidget::draw(NVGcontext *vg) {
	ScrollWidget::draw(vg);

	if (app()->scene->rackWidget->isEmpty()) {
		math::Rect b;
		b.size = math::Vec(600, 300);
		b.pos = box.size.minus(b.size).div(2);
		NVGcolor bg = nvgRGBAf(0, 0, 0, 0.4);
		bndInnerBox(vg, b.pos.x, b.pos.y, b.size.x, b.size.y,
			0, 0, 0, 0, bg, bg);

		NVGcolor fg = nvgRGBAf(1, 1, 1, 0.25);
		std::string text = "Right-click or press Enter to add modules";
		bndIconLabelValue(vg, b.pos.x, b.pos.y + 80, b.size.x, b.size.y, -1, fg, BND_CENTER, 80, text.c_str(), NULL);
	}
}


} // namespace rack
