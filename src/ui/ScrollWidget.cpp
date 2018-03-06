#include "ui.hpp"
#include "window.hpp"


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
		box.size = Vec(BND_SCROLLBAR_WIDTH, BND_SCROLLBAR_HEIGHT);
	}

	void draw(NVGcontext *vg) override {
		bndScrollBar(vg, 0.0, 0.0, box.size.x, box.size.y, state, offset, size);
	}

	void onDragStart(EventDragStart &e) override {
		state = BND_ACTIVE;
		windowCursorLock();
	}

	void onDragMove(EventDragMove &e) override {
		ScrollWidget *scrollWidget = dynamic_cast<ScrollWidget*>(parent);
		assert(scrollWidget);
		if (orientation == HORIZONTAL)
			scrollWidget->offset.x += e.mouseRel.x;
		else
			scrollWidget->offset.y += e.mouseRel.y;
	}

	void onDragEnd(EventDragEnd &e) override {
		state = BND_DEFAULT;
		windowCursorUnlock();
	}
};


ScrollWidget::ScrollWidget() {
	container = new Widget();
	addChild(container);

	horizontalScrollBar = new ScrollBar();
	horizontalScrollBar->orientation = ScrollBar::HORIZONTAL;
	horizontalScrollBar->visible = false;
	addChild(horizontalScrollBar);

	verticalScrollBar = new ScrollBar();
	verticalScrollBar->orientation = ScrollBar::VERTICAL;
	verticalScrollBar->visible = false;
	addChild(verticalScrollBar);
}

void ScrollWidget::scrollTo(Rect r) {
	Rect bound = Rect::fromMinMax(r.getBottomRight().minus(box.size), r.pos);
	offset = offset.clamp2(bound);
}

void ScrollWidget::draw(NVGcontext *vg) {
	nvgScissor(vg, 0, 0, box.size.x, box.size.y);
	Widget::draw(vg);
	nvgResetScissor(vg);
}

void ScrollWidget::step() {
	Widget::step();

	// Clamp scroll offset
	Vec containerCorner = container->getChildrenBoundingBox().getBottomRight();
	Rect containerBox = Rect(Vec(0, 0), containerCorner.minus(box.size));
	offset = offset.clamp(containerBox);
	// Lock offset to top/left if no scrollbar will display
	if (containerBox.size.x < 0.0)
		offset.x = 0.0;
	if (containerBox.size.y < 0.0)
		offset.y = 0.0;

	// Update the container's positions from the offset
	container->box.pos = offset.neg().round();

	// Update scrollbar offsets and sizes
	Vec viewportSize = container->getChildrenBoundingBox().getBottomRight();
	Vec scrollbarOffset = offset.div(viewportSize.minus(box.size));
	Vec scrollbarSize = box.size.div(viewportSize);

	horizontalScrollBar->visible = (0.0 < scrollbarSize.x && scrollbarSize.x < 1.0);
	verticalScrollBar->visible = (0.0 < scrollbarSize.y && scrollbarSize.y < 1.0);
	horizontalScrollBar->offset = scrollbarOffset.x;
	verticalScrollBar->offset = scrollbarOffset.y;
	horizontalScrollBar->size = scrollbarSize.x;
	verticalScrollBar->size = scrollbarSize.y;

	// Resize scroll bars
	Vec inner = Vec(box.size.x - verticalScrollBar->box.size.x, box.size.y - horizontalScrollBar->box.size.y);
	horizontalScrollBar->box.pos.y = inner.y;
	verticalScrollBar->box.pos.x = inner.x;
	horizontalScrollBar->box.size.x = verticalScrollBar->visible ? inner.x : box.size.x;
	verticalScrollBar->box.size.y = horizontalScrollBar->visible ? inner.y : box.size.y;
}

void ScrollWidget::onMouseMove(EventMouseMove &e) {
	// Scroll with arrow keys
	if (!gFocusedWidget) {
		float arrowSpeed = 30.0;
		if (windowIsShiftPressed() && windowIsModPressed())
			arrowSpeed /= 16.0;
		else if (windowIsShiftPressed())
			arrowSpeed *= 4.0;
		else if (windowIsModPressed())
			arrowSpeed /= 4.0;

		if (glfwGetKey(gWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
			offset.x -= arrowSpeed;
		}
		if (glfwGetKey(gWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			offset.x += arrowSpeed;
		}
		if (glfwGetKey(gWindow, GLFW_KEY_UP) == GLFW_PRESS) {
			offset.y -= arrowSpeed;
		}
		if (glfwGetKey(gWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
			offset.y += arrowSpeed;
		}
	}

	Widget::onMouseMove(e);
}

void ScrollWidget::onScroll(EventScroll &e) {
	offset = offset.minus(e.scrollRel);
	e.consumed = true;
}

void ScrollWidget::onHoverKey(EventHoverKey &e) {
	Widget::onHoverKey(e);
}


} // namespace rack
