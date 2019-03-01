#include "app/RackScrollWidget.hpp"
#include "app/Scene.hpp"
#include "window.hpp"
#include "app.hpp"
#include "settings.hpp"


namespace rack {
namespace app {


RackScrollWidget::RackScrollWidget() {
	zoom = new widget::ZoomWidget;
	container->addChild(zoom);

	rack = new RackWidget;
	zoom->addChild(rack);
}

void RackScrollWidget::step() {
	// Set ZoomWidget's zoom every few frames
	int frame = APP->window->frame;
	if (frame % 10 == 0) {
		zoom->setZoom(std::round(settings.zoom * 100) / 100);
	}

	// Resize RackWidget to be a bit larger than the viewport
	rack->box.size = box.size
		.minus(container->box.pos)
		.plus(math::Vec(500, 500))
		.div(zoom->zoom);

	// Resize ZoomWidget
	zoom->box.size = rack->box.size.mult(zoom->zoom);

	// Scroll rack if dragging cable near the edge of the screen
	math::Vec pos = APP->window->mousePos;
	math::Rect viewport = getViewport(box.zeroPos());
	if (rack->incompleteCable) {
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
	ScrollWidget::draw(args);
}

void RackScrollWidget::onHover(const widget::HoverEvent &e) {
	if (!APP->event->selectedWidget) {
		// Scroll with arrow keys
		float arrowSpeed = 30.0;
		if ((APP->window->getMods() & WINDOW_MOD_MASK) == (WINDOW_MOD_CTRL |GLFW_MOD_SHIFT))
			arrowSpeed /= 16.0;
		else if ((APP->window->getMods() & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL)
			arrowSpeed *= 4.0;
		else if ((APP->window->getMods() & WINDOW_MOD_MASK) == GLFW_MOD_SHIFT)
			arrowSpeed /= 4.0;

		if (glfwGetKey(APP->window->win, GLFW_KEY_LEFT) == GLFW_PRESS) {
			offset.x -= arrowSpeed;
		}
		if (glfwGetKey(APP->window->win, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			offset.x += arrowSpeed;
		}
		if (glfwGetKey(APP->window->win, GLFW_KEY_UP) == GLFW_PRESS) {
			offset.y -= arrowSpeed;
		}
		if (glfwGetKey(APP->window->win, GLFW_KEY_DOWN) == GLFW_PRESS) {
			offset.y += arrowSpeed;
		}
	}

	ScrollWidget::onHover(e);
}

void RackScrollWidget::onHoverScroll(const widget::HoverScrollEvent &e) {
	if ((APP->window->getMods() & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
		e.consume(this);
		return;
	}

	ScrollWidget::onHoverScroll(e);
}


} // namespace app
} // namespace rack
