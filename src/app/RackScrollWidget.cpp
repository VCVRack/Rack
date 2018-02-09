#include "app.hpp"
#include "window.hpp"


namespace rack {


void RackScrollWidget::step() {
	Vec pos = gMousePos;
	Rect viewport = getViewport(box.zeroPos());
	// Scroll rack if dragging cable near the edge of the screen
	if (gRackWidget->wireContainer->activeWire) {
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


} // namespace rack
