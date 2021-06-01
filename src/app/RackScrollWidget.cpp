#include <app/RackScrollWidget.hpp>
#include <app/Scene.hpp>
#include <window.hpp>
#include <context.hpp>
#include <settings.hpp>


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


void RackScrollWidget::reset() {
	offset = RACK_OFFSET.mult(zoomWidget->zoom);
	offset = offset.minus(math::Vec(30, 30));
}


void RackScrollWidget::step() {
	// Compute zoom from exponential zoom
	float zoom = std::pow(2.f, settings::zoom);
	if (zoom != zoomWidget->zoom) {
		// Set offset based on zoomPos
		offset = offset.plus(zoomPos).div(zoomWidget->zoom).mult(zoom).minus(zoomPos);
		// Set zoom
		zoomWidget->setZoom(zoom);
	}

	zoomPos = box.size.div(2);

	// Compute module bounding box
	math::Rect moduleBox = rackWidget->moduleContainer->getChildrenBoundingBox();
	if (!moduleBox.size.isFinite())
		moduleBox = math::Rect(RACK_OFFSET, math::Vec(0, 0));

	// Expand moduleBox by half a screen size
	math::Rect scrollBox = moduleBox;
	scrollBox.pos = scrollBox.pos.mult(zoom);
	scrollBox.size = scrollBox.size.mult(zoom);
	scrollBox = scrollBox.grow(box.size.mult(0.6666));

	// Expand to the current viewport box so that moving modules (and thus changing the module bounding box) doesn't clamp the scroll offset.
	math::Rect viewportBox;
	viewportBox.pos = oldOffset;
	viewportBox.size = box.size;
	scrollBox = scrollBox.expand(viewportBox);

	// Reposition widgets
	zoomWidget->box = scrollBox;
	rackWidget->box.pos = scrollBox.pos.div(zoom).neg();

	// Scroll rack if dragging cable near the edge of the screen
	math::Vec pos = APP->scene->mousePos;
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

	// Hide scrollbars if fullscreen
	hideScrollbars = APP->window->isFullScreen();

	ScrollWidget::step();
	oldOffset = offset;
}


void RackScrollWidget::draw(const DrawArgs& args) {
	ScrollWidget::draw(args);
}


void RackScrollWidget::onHoverKey(const HoverKeyEvent& e) {
	ScrollWidget::onHoverKey(e);
	if (e.isConsumed())
		return;

	// Scroll with arrow keys
	float arrowSpeed = 32.f;
	if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL)
		arrowSpeed /= 4.f;
	if ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT)
		arrowSpeed *= 4.f;
	if ((e.mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT))
		arrowSpeed /= 16.f;

	if (e.action == RACK_HELD) {
		if (e.key == GLFW_KEY_LEFT) {
			offset.x -= arrowSpeed;
			e.consume(this);
		}
		if (e.key == GLFW_KEY_RIGHT) {
			offset.x += arrowSpeed;
			e.consume(this);
		}
		if (e.key == GLFW_KEY_UP) {
			offset.y -= arrowSpeed;
			e.consume(this);
		}
		if (e.key == GLFW_KEY_DOWN) {
			offset.y += arrowSpeed;
			e.consume(this);
		}
	}
}


void RackScrollWidget::onHoverScroll(const HoverScrollEvent& e) {
	ScrollWidget::onHoverScroll(e);
	if (e.isConsumed())
		return;

	if ((APP->window->getMods() & RACK_MOD_MASK) == RACK_MOD_CTRL) {
		// Increase zoom
		float zoomDelta = e.scrollDelta.y / 50 / 4;
		if (settings::invertZoom)
			zoomDelta *= -1;
		settings::zoom += zoomDelta;
		// Limit min/max depending on the direction of zooming
		if (zoomDelta > 0.f)
			settings::zoom = std::fmin(settings::zoom, settings::zoomMax);
		else
			settings::zoom = std::fmax(settings::zoom, settings::zoomMin);
		zoomPos = e.pos;
		e.consume(this);
	}
}


void RackScrollWidget::onHover(const HoverEvent& e) {
	ScrollWidget::onHover(e);

	// Hide menu bar if fullscreen and moving mouse over the RackScrollWidget
	if (APP->window->isFullScreen()) {
		APP->scene->menuBar->hide();
	}
}


void RackScrollWidget::onButton(const ButtonEvent& e)  {
	ScrollWidget::onButton(e);
	if (e.isConsumed())
		return;

	// Zoom in/out with extra mouse buttons
	if (e.action == GLFW_PRESS) {
		if (e.button == GLFW_MOUSE_BUTTON_4) {
			settings::zoom -= 0.5f;
			settings::zoom = std::fmax(settings::zoom, settings::zoomMin);
			e.consume(this);
		}
		if (e.button == GLFW_MOUSE_BUTTON_5) {
			settings::zoom += 0.5f;
			settings::zoom = std::fmin(settings::zoom, settings::zoomMax);
			e.consume(this);
		}
	}
}


} // namespace app
} // namespace rack
