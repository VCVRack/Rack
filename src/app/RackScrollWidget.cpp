#include "app/RackScrollWidget.hpp"
#include "app/Scene.hpp"
#include "window.hpp"
#include "app.hpp"
#include "settings.hpp"


namespace rack {
namespace app {


RackScrollWidget::RackScrollWidget() {
	zoomWidget = new widget::ZoomWidget;
	container->addChild(zoomWidget);

	rackWidget = new RackWidget;
	rackWidget->box.size = RACK_OFFSET.mult(2);
	zoomWidget->addChild(rackWidget);

	reset();
}

void RackScrollWidget::step() {
	// Clamp zoom
	settings::zoom = math::clamp(settings::zoom, 0.25f, 2.f);
	// Round zoom to nearest percent
	float zoom = std::round(settings::zoom / 0.01) * 0.01;
	if (zoom != zoomWidget->zoom) {
		// Set offset based on zoomPos
		offset = offset.plus(zoomPos).div(zoomWidget->zoom).mult(zoom).minus(zoomPos);
		// Set zoom
		zoomWidget->setZoom(zoom);
	}

	zoomPos = box.size.div(2);

	// Set zoomWidget box to module bounding box
	math::Rect moduleBox = rackWidget->moduleContainer->getChildrenBoundingBox();
	if (!moduleBox.size.isFinite())
		moduleBox = math::Rect(RACK_OFFSET, math::Vec(0, 0));
	zoomWidget->box.pos = moduleBox.pos.mult(zoomWidget->zoom);
	zoomWidget->box.size = moduleBox.size.mult(zoomWidget->zoom);
	// Expand to viewport
	math::Rect viewportBox = box;
	viewportBox.pos = viewportBox.pos.plus(offset);
	zoomWidget->box = zoomWidget->box.expand(viewportBox);
	// Grow a few pixels
	zoomWidget->box = zoomWidget->box.grow(math::Vec(100, 100));
	// Reposition rackWidget
	rackWidget->box.pos = zoomWidget->box.pos.div(zoomWidget->zoom).neg();

	// Scroll rack if dragging cable near the edge of the screen
	math::Vec pos = APP->window->mousePos;
	math::Rect viewport = getViewport(box.zeroPos());
	if (rackWidget->incompleteCable) {
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


void RackScrollWidget::draw(const DrawArgs &args) {
	// DEBUG("%f %f %f %f", RECT_ARGS(args.clipBox));
	ScrollWidget::draw(args);
}

void RackScrollWidget::onHover(const event::Hover &e) {
	// Scroll with arrow keys
	float arrowSpeed = 30.0;
	if ((APP->window->getMods() & RACK_MOD_MASK) == (RACK_MOD_CTRL |GLFW_MOD_SHIFT))
		arrowSpeed /= 16.0;
	else if ((APP->window->getMods() & RACK_MOD_MASK) == RACK_MOD_CTRL)
		arrowSpeed *= 4.0;
	else if ((APP->window->getMods() & RACK_MOD_MASK) == GLFW_MOD_SHIFT)
		arrowSpeed /= 4.0;

	if (glfwGetKey(APP->window->win, GLFW_KEY_LEFT) == GLFW_PRESS)
		offset.x -= arrowSpeed;
	if (glfwGetKey(APP->window->win, GLFW_KEY_RIGHT) == GLFW_PRESS)
		offset.x += arrowSpeed;
	if (glfwGetKey(APP->window->win, GLFW_KEY_UP) == GLFW_PRESS)
		offset.y -= arrowSpeed;
	if (glfwGetKey(APP->window->win, GLFW_KEY_DOWN) == GLFW_PRESS)
		offset.y += arrowSpeed;

	ScrollWidget::onHover(e);
}

void RackScrollWidget::onHoverScroll(const event::HoverScroll &e) {
	if ((APP->window->getMods() & RACK_MOD_MASK) == RACK_MOD_CTRL) {
		// Increase zoom
		float zoomDelta = e.scrollDelta.y / 50 / 4;
		if (settings::invertZoom)
			zoomDelta *= -1;
		settings::zoom *= std::pow(2, zoomDelta);
		zoomPos = e.pos;
		e.consume(this);
		return;
	}

	ScrollWidget::onHoverScroll(e);
}

void RackScrollWidget::reset() {
	offset = RACK_OFFSET.mult(zoomWidget->zoom);
	offset = offset.minus(math::Vec(30, 30));
}


} // namespace app
} // namespace rack
