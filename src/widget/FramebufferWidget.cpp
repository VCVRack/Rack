#include <widget/FramebufferWidget.hpp>
#include <app.hpp>
#include <random.hpp>


namespace rack {
namespace widget {


FramebufferWidget::FramebufferWidget() {
}

FramebufferWidget::~FramebufferWidget() {
	if (fb)
		nvgluDeleteFramebuffer(fb);
}

void FramebufferWidget::step() {
	Widget::step();

	// It's more important to not lag the frame than to draw the framebuffer
	if (APP->window->isFrameOverdue())
		return;

	// Check that scale has been set by `draw()` yet.
	if (scale.isZero())
		return;

	// Only redraw if FramebufferWidget is dirty
	if (!dirty)
		return;

	// In case we fail drawing the framebuffer, don't try again the next frame, so reset `dirty` here.
	dirty = false;
	NVGcontext* vg = APP->window->vg;

	fbScale = scale;
	// Set scale to zero so we must wait for the next draw() call before drawing the framebuffer again.
	// Otherwise, if the zoom level is changed while the FramebufferWidget is off-screen, the next draw() call will be skipped, the `dirty` flag will be true, and the framebuffer will be redrawn, but at the wrong scale, since it was not set in draw().
	scale = math::Vec();
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

	math::Vec newFbSize = fbBox.size.mult(APP->window->pixelRatio).ceil();

	// Create framebuffer if a new size is needed
	if (!fb || !newFbSize.isEqual(fbSize)) {
		fbSize = newFbSize;
		// Delete old framebuffer
		if (fb)
			nvgluDeleteFramebuffer(fb);
		// Create a framebuffer at the oversampled size
		if (fbSize.isFinite() && !fbSize.isZero())
			fb = nvgluCreateFramebuffer(vg, fbSize.x * oversample, fbSize.y * oversample, 0);
	}

	if (!fb) {
		WARN("Framebuffer of size (%f, %f) * %f could not be created for FramebufferWidget.", VEC_ARGS(fbSize), oversample);
		return;
	}

	nvgluBindFramebuffer(fb);
	drawFramebuffer();
	nvgluBindFramebuffer(NULL);

	// If oversampling, create another framebuffer and copy it to actual size.
	if (oversample != 1.0) {
		NVGLUframebuffer* newFb = nvgluCreateFramebuffer(vg, fbSize.x, fbSize.y, 0);
		if (!newFb) {
			WARN("Non-oversampled framebuffer of size (%f, %f) could not be created for FramebufferWidget.", VEC_ARGS(fbSize));
			return;
		}

		// Use NanoVG for resizing framebuffers
		nvgluBindFramebuffer(newFb);

		nvgBeginFrame(vg, fbBox.size.x, fbBox.size.y, 1.0);

		// Draw oversampled framebuffer
		nvgBeginPath(vg);
		nvgRect(vg, 0.0, 0.0, fbSize.x, fbSize.y);
		NVGpaint paint = nvgImagePattern(vg, 0.0, 0.0, fbSize.x, fbSize.y,
		                                 0.0, fb->image, 1.0);
		nvgFillPaint(vg, paint);
		nvgFill(vg);

		glViewport(0.0, 0.0, fbSize.x, fbSize.y);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		nvgEndFrame(vg);
		nvgReset(vg);

		nvgluBindFramebuffer(NULL);

		// Swap the framebuffers
		nvgluDeleteFramebuffer(fb);
		fb = newFb;
	}
}

void FramebufferWidget::draw(const DrawArgs& args) {
	// Draw directly if already drawing in a framebuffer
	if (bypass || args.fb) {
		Widget::draw(args);
		return;
	}

	// Get world transform
	float xform[6];
	nvgCurrentTransform(args.vg, xform);
	// Skew and rotate is not supported
	if (!math::isNear(xform[1], 0.f) || !math::isNear(xform[2], 0.f)) {
		WARN("Skew and rotation detected but not supported in FramebufferWidget.");
		return;
	}
	// Extract scale and offset from world transform
	scale = math::Vec(xform[0], xform[3]);
	offset = math::Vec(xform[4], xform[5]);
	math::Vec offsetI = offset.floor();

	math::Vec scaleRatio = math::Vec(1, 1);
	if (!fbScale.isZero() && !scale.isEqual(fbScale)) {
		dirty = true;
		// Continue to draw but at the wrong scale. In the next frame, the framebuffer will be redrawn.
		scaleRatio = scale.div(fbScale);
	}

	if (!fb)
		return;

	// Draw framebuffer image, using world coordinates
	nvgSave(args.vg);
	nvgResetTransform(args.vg);

	nvgBeginPath(args.vg);
	nvgRect(args.vg,
	        offsetI.x + fbBox.pos.x,
	        offsetI.y + fbBox.pos.y,
	        fbBox.size.x * scaleRatio.x, fbBox.size.y * scaleRatio.y);
	NVGpaint paint = nvgImagePattern(args.vg,
	                                 offsetI.x + fbBox.pos.x,
	                                 offsetI.y + fbBox.pos.y,
	                                 fbBox.size.x * scaleRatio.x, fbBox.size.y * scaleRatio.y,
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
	NVGcontext* vg = APP->window->vg;

	float pixelRatio = fbSize.x * oversample / fbBox.size.x;
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

	glViewport(0.0, 0.0, fbSize.x * oversample, fbSize.y * oversample);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	// glClearColor(0.0, 1.0, 1.0, 0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	nvgEndFrame(vg);

	// Clean up the NanoVG state so that calls to nvgTextBounds() etc during step() don't use a dirty state.
	nvgReset(vg);
}

int FramebufferWidget::getImageHandle() {
	if (!fb)
		return -1;
	return fb->image;
}


} // namespace widget
} // namespace rack
