#include "widgets/FramebufferWidget.hpp"
#include "app.hpp"


namespace rack {


FramebufferWidget::FramebufferWidget() {
	oversample = 1.0;
}

FramebufferWidget::~FramebufferWidget() {
	if (fb)
		nvgluDeleteFramebuffer(fb);
}

void FramebufferWidget::draw(NVGcontext *vg) {
	// Bypass framebuffer rendering if we're already drawing in a framebuffer
	// In other words, disallow nested framebuffers. They look bad.
	if (vg == app()->window->fbVg) {
		Widget::draw(vg);
		return;
	}

	// Get world transform
	float xform[6];
	nvgCurrentTransform(vg, xform);
	// Skew and rotate is not supported
	assert(math::isNear(xform[1], 0.f));
	assert(math::isNear(xform[2], 0.f));
	// Extract scale and offset from world transform
	math::Vec scale = math::Vec(xform[0], xform[3]);
	math::Vec offset = math::Vec(xform[4], xform[5]);
	math::Vec offsetI = offset.floor();

	// Render to framebuffer
	if (dirty) {
		dirty = false;

		fbScale = scale;
		// World coordinates, in range [0, 1)
		fbOffset = offset.minus(offsetI);

		math::Rect localBox;
		if (children.empty()) {
			localBox = box.zeroPos();
		}
		else {
			localBox = getChildrenBoundingBox();
		}

		// DEBUG("%g %g %g %g, %g %g, %g %g", RECT_ARGS(localBox), VEC_ARGS(fbOffset), VEC_ARGS(scale));
		// Transform to world coordinates, then expand to nearest integer coordinates
		math::Vec min = localBox.getTopLeft().mult(scale).plus(fbOffset).floor();
		math::Vec max = localBox.getBottomRight().mult(scale).plus(fbOffset).ceil();
		fbBox = math::Rect::fromMinMax(min, max);
		// DEBUG("%g %g %g %g", RECT_ARGS(fbBox));

		math::Vec newFbSize = fbBox.size.mult(app()->window->pixelRatio * oversample);

		if (!fb || !newFbSize.isEqual(fbSize)) {
			fbSize = newFbSize;
			// Delete old framebuffer
			if (fb)
				nvgluDeleteFramebuffer(fb);
			// Create a framebuffer from the main nanovg context. We will draw to this in the secondary nanovg context.
			if (fbSize.isFinite() && !fbSize.isZero())
				fb = nvgluCreateFramebuffer(vg, fbSize.x, fbSize.y, 0);
		}

		if (!fb)
			return;

		nvgluBindFramebuffer(fb);
		drawFramebuffer();
		nvgluBindFramebuffer(NULL);
	}

	if (!fb)
		return;

	// Draw framebuffer image, using world coordinates
	nvgSave(vg);
	nvgResetTransform(vg);

	nvgBeginPath(vg);
	nvgRect(vg,
		offsetI.x + fbBox.pos.x,
		offsetI.y + fbBox.pos.y,
		fbBox.size.x, fbBox.size.y);
	NVGpaint paint = nvgImagePattern(vg,
		offsetI.x + fbBox.pos.x,
		offsetI.y + fbBox.pos.y,
		fbBox.size.x, fbBox.size.y,
		0.0, fb->image, 1.0);
	nvgFillPaint(vg, paint);
	nvgFill(vg);

	// For debugging the bounding box of the framebuffer
	// nvgStrokeWidth(vg, 2.0);
	// nvgStrokeColor(vg, nvgRGBAf(1, 1, 0, 0.5));
	// nvgStroke(vg);

	nvgRestore(vg);
}

void FramebufferWidget::drawFramebuffer() {
	NVGcontext *vg = app()->window->fbVg;

	float pixelRatio = fbSize.x / fbBox.size.x;
	nvgBeginFrame(vg, fbBox.size.x, fbBox.size.y, pixelRatio);

	// Use local scaling
	nvgTranslate(vg, -fbBox.pos.x, -fbBox.pos.y);
	nvgTranslate(vg, fbOffset.x, fbOffset.y);
	nvgScale(vg, fbScale.x, fbScale.y);

	Widget::draw(vg);

	glViewport(0.0, 0.0, fbSize.x, fbSize.y);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	// glClearColor(0.0, 1.0, 1.0, 0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	nvgEndFrame(vg);
}

int FramebufferWidget::getImageHandle() {
	if (!fb)
		return -1;
	return fb->image;
}


} // namespace rack
