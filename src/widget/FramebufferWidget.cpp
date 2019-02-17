#include "widget/FramebufferWidget.hpp"
#include "app.hpp"


namespace rack {
namespace widget {


FramebufferWidget::FramebufferWidget() {
	oversample = 1.0;
}

FramebufferWidget::~FramebufferWidget() {
	if (fb)
		nvgluDeleteFramebuffer(fb);
}

void FramebufferWidget::step() {
	Widget::step();

	// Render to framebuffer if dirty.
	// Also check that scale has been set by `draw()` yet.
	if (dirty && !scale.isZero()) {
		// In case we fail drawing the framebuffer, don't try again the next frame, so reset `dirty` here.
		dirty = false;

		fbScale = scale;
		// Get subpixel offset in range [0, 1)
		math::Vec offsetI = offset.floor();
		fbOffset = offset.minus(offsetI);

		math::Rect localBox;
		if (children.empty()) {
			localBox = box.zeroPos();
		}
		else {
			localBox = getChildrenBoundingBox();
		}

		// DEBUG("%g %g %g %g, %g %g, %g %g", RECT_ARGS(localBox), VEC_ARGS(fbOffset), VEC_ARGS(fbScale));
		// Transform to world coordinates, then expand to nearest integer coordinates
		math::Vec min = localBox.getTopLeft().mult(fbScale).plus(fbOffset).floor();
		math::Vec max = localBox.getBottomRight().mult(fbScale).plus(fbOffset).ceil();
		fbBox = math::Rect::fromMinMax(min, max);
		// DEBUG("%g %g %g %g", RECT_ARGS(fbBox));

		math::Vec newFbSize = fbBox.size.mult(APP->window->pixelRatio * oversample);

		// Create framebuffer if a new size is needed
		if (!fb || !newFbSize.isEqual(fbSize)) {
			fbSize = newFbSize;
			// Delete old framebuffer
			if (fb)
				nvgluDeleteFramebuffer(fb);
			// Create a framebuffer from the main nanovg context. We will draw to this in the secondary nanovg context.
			if (fbSize.isFinite() && !fbSize.isZero())
				fb = nvgluCreateFramebuffer(APP->window->vg, fbSize.x, fbSize.y, 0);
		}

		if (!fb)
			return;

		nvgluBindFramebuffer(fb);
		drawFramebuffer();
		nvgluBindFramebuffer(NULL);
	}
}

void FramebufferWidget::draw(const DrawArgs &args) {
	// Draw directly if already drawing in a framebuffer
	if (args.fb) {
		Widget::draw(args);
		return;
	}

	// Get world transform
	float xform[6];
	nvgCurrentTransform(args.vg, xform);
	// Skew and rotate is not supported
	assert(math::isNear(xform[1], 0.f));
	assert(math::isNear(xform[2], 0.f));
	// Extract scale and offset from world transform
	scale = math::Vec(xform[0], xform[3]);
	offset = math::Vec(xform[4], xform[5]);
	math::Vec offsetI = offset.floor();

	if (!fb)
		return;

	// Draw framebuffer image, using world coordinates
	nvgSave(args.vg);
	nvgResetTransform(args.vg);

	nvgBeginPath(args.vg);
	nvgRect(args.vg,
		offsetI.x + fbBox.pos.x,
		offsetI.y + fbBox.pos.y,
		fbBox.size.x, fbBox.size.y);
	NVGpaint paint = nvgImagePattern(args.vg,
		offsetI.x + fbBox.pos.x,
		offsetI.y + fbBox.pos.y,
		fbBox.size.x, fbBox.size.y,
		0.0, fb->image, 1.0);
	nvgFillPaint(args.vg, paint);
	nvgFill(args.vg);

	// For debugging the bounding box of the framebuffer
	// nvgStrokeWidth(args.vg, 2.0);
	// nvgStrokeColor(args.vg, nvgRGBAf(1, 1, 0, 0.5));
	// nvgStroke(args.vg);

	nvgRestore(args.vg);
}

void FramebufferWidget::drawFramebuffer() {
	NVGcontext *vg = APP->window->vg;

	float pixelRatio = fbSize.x / fbBox.size.x;
	nvgBeginFrame(vg, fbBox.size.x, fbBox.size.y, pixelRatio);

	// Use local scaling
	nvgTranslate(vg, -fbBox.pos.x, -fbBox.pos.y);
	nvgTranslate(vg, fbOffset.x, fbOffset.y);
	nvgScale(vg, fbScale.x, fbScale.y);

	DrawArgs args;
	args.vg = vg;
	args.clipBox = box.zeroPos();
	args.fb = fb;
	Widget::draw(args);

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


} // namespace widget
} // namespace rack
