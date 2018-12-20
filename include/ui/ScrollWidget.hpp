#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "ui/common.hpp"
#include "event.hpp"
#include "context.hpp"


namespace rack {


/** Parent must be a ScrollWidget */
struct ScrollBar : OpaqueWidget {
	enum Orientation {
		VERTICAL,
		HORIZONTAL
	};
	Orientation orientation;
	BNDwidgetState state = BND_DEFAULT;
	float offset = 0.0;
	float size = 0.0;

	ScrollBar() {
		box.size = math::Vec(BND_SCROLLBAR_WIDTH, BND_SCROLLBAR_HEIGHT);
	}

	void draw(NVGcontext *vg) override {
		bndScrollBar(vg, 0.0, 0.0, box.size.x, box.size.y, state, offset, size);
	}

	void onDragStart(event::DragStart &e) override {
		state = BND_ACTIVE;
		context()->window->cursorLock();
	}

	void onDragMove(event::DragMove &e) override;

	void onDragEnd(event::DragEnd &e) override {
		state = BND_DEFAULT;
		context()->window->cursorUnlock();
	}
};


/** Handles a container with ScrollBar */
struct ScrollWidget : OpaqueWidget {
	Widget *container;
	ScrollBar *horizontalScrollBar;
	ScrollBar *verticalScrollBar;
	math::Vec offset;

	ScrollWidget() {
		container = new Widget;
		addChild(container);

		horizontalScrollBar = new ScrollBar;
		horizontalScrollBar->orientation = ScrollBar::HORIZONTAL;
		horizontalScrollBar->visible = false;
		addChild(horizontalScrollBar);

		verticalScrollBar = new ScrollBar;
		verticalScrollBar->orientation = ScrollBar::VERTICAL;
		verticalScrollBar->visible = false;
		addChild(verticalScrollBar);
	}

	void scrollTo(math::Rect r) {
		math::Rect bound = math::Rect::fromMinMax(r.getBottomRight().minus(box.size), r.pos);
		offset = offset.clampBetween(bound);
	}

	void draw(NVGcontext *vg) override {
		nvgScissor(vg, 0, 0, box.size.x, box.size.y);
		Widget::draw(vg);
		nvgResetScissor(vg);
	}

	void step() override {
		Widget::step();

		// Clamp scroll offset
		math::Vec containerCorner = container->getChildrenBoundingBox().getBottomRight();
		math::Rect containerBox = math::Rect(math::Vec(0, 0), containerCorner.minus(box.size));
		offset = offset.clamp(containerBox);
		// Lock offset to top/left if no scrollbar will display
		if (containerBox.size.x < 0.0)
			offset.x = 0.0;
		if (containerBox.size.y < 0.0)
			offset.y = 0.0;

		// Update the container's positions from the offset
		container->box.pos = offset.neg().round();

		// Update scrollbar offsets and sizes
		math::Vec viewportSize = container->getChildrenBoundingBox().getBottomRight();
		math::Vec scrollbarOffset = offset.div(viewportSize.minus(box.size));
		math::Vec scrollbarSize = box.size.div(viewportSize);

		horizontalScrollBar->visible = (0.0 < scrollbarSize.x && scrollbarSize.x < 1.0);
		verticalScrollBar->visible = (0.0 < scrollbarSize.y && scrollbarSize.y < 1.0);
		horizontalScrollBar->offset = scrollbarOffset.x;
		verticalScrollBar->offset = scrollbarOffset.y;
		horizontalScrollBar->size = scrollbarSize.x;
		verticalScrollBar->size = scrollbarSize.y;

		// Resize scroll bars
		math::Vec inner = math::Vec(box.size.x - verticalScrollBar->box.size.x, box.size.y - horizontalScrollBar->box.size.y);
		horizontalScrollBar->box.pos.y = inner.y;
		verticalScrollBar->box.pos.x = inner.x;
		horizontalScrollBar->box.size.x = verticalScrollBar->visible ? inner.x : box.size.x;
		verticalScrollBar->box.size.y = horizontalScrollBar->visible ? inner.y : box.size.y;
	}

	void onHover(event::Hover &e) override {
		// Scroll with arrow keys
		if (!context()->event->selectedWidget) {
			float arrowSpeed = 30.0;
			if (context()->window->isShiftPressed() && context()->window->isModPressed())
				arrowSpeed /= 16.0;
			else if (context()->window->isShiftPressed())
				arrowSpeed *= 4.0;
			else if (context()->window->isModPressed())
				arrowSpeed /= 4.0;

			if (glfwGetKey(context()->window->win, GLFW_KEY_LEFT) == GLFW_PRESS) {
				offset.x -= arrowSpeed;
			}
			if (glfwGetKey(context()->window->win, GLFW_KEY_RIGHT) == GLFW_PRESS) {
				offset.x += arrowSpeed;
			}
			if (glfwGetKey(context()->window->win, GLFW_KEY_UP) == GLFW_PRESS) {
				offset.y -= arrowSpeed;
			}
			if (glfwGetKey(context()->window->win, GLFW_KEY_DOWN) == GLFW_PRESS) {
				offset.y += arrowSpeed;
			}
		}

		OpaqueWidget::onHover(e);
	}

	void onHoverScroll(event::HoverScroll &e) override {
		offset = offset.minus(e.scrollDelta);
		e.target = this;
	}
};


inline void ScrollBar::onDragMove(event::DragMove &e) {
	ScrollWidget *scrollWidget = dynamic_cast<ScrollWidget*>(parent);
	assert(scrollWidget);
	if (orientation == HORIZONTAL)
		scrollWidget->offset.x += e.mouseDelta.x;
	else
		scrollWidget->offset.y += e.mouseDelta.y;
}


} // namespace rack
