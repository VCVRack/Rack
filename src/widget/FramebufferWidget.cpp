#include <widget/FramebufferWidget.hpp>
#include <context.hpp>
#include <random.hpp>


namespace rack {
namespace widget {


struct FramebufferWidget::Internal {
	NVGLUframebuffer* fb = NULL;
	/** Scale relative to the world */
	math::Vec scale;
	/** Offset in world coordinates */
	math::Vec offset;
	/** Pixel dimensions of the allocated framebuffer */
	math::Vec fbSize;
	/** Bounding box in world coordinates of where the framebuffer should be painted.
	Always has integer coordinates so that blitting framebuffers is pixel-perfect.
	*/
	math::Rect fbBox;
	/** Framebuffer's scale relative to the world */
	math::Vec fbScale;
	/** Framebuffer's subpixel offset relative to fbBox in world coordinates */
	math::Vec fbOffset;
};


FramebufferWidget::FramebufferWidget() {
	internal = new Internal;
}


FramebufferWidget::~FramebufferWidget() {
	if (internal->fb)
		nvgluDeleteFramebuffer(internal->fb);
	delete internal;
}


void FramebufferWidget::onDirty(const event::Dirty& e) {
	dirty = true;
	Widget::onDirty(e);
}


void FramebufferWidget::step() {
	Widget::step();

	// It's more important to not lag the frame than to draw the framebuffer
	if (APP->window->isFrameOverdue())
		return;

	// Check that scale has been set by `draw()` yet.
	if (internal->scale.isZero())
		return;

	// Only redraw if FramebufferWidget is dirty
	if (!dirty)
		return;

	// In case we fail drawing the framebuffer, don't try again the next frame, so reset `dirty` here.
	dirty = false;
	NVGcontext* vg = APP->window->vg;

	internal->fbScale = internal->scale;
	// Set scale to zero so we must wait for the next draw() call before drawing the framebuffer again.
	// Otherwise, if the zoom level is changed while the FramebufferWidget is off-screen, the next draw() call will be skipped, the `dirty` flag will be true, and the framebuffer will be redrawn, but at the wrong scale, since it was not set in draw().
	internal->scale = math::Vec();
	// Get subpixel offset in range [0, 1)
	math::Vec offsetI = internal->offset.floor();
	internal->fbOffset = internal->offset.minus(offsetI);

	math::Rect localBox;
	if (children.empty()) {
		localBox = box.zeroPos();
	}
	else {
		localBox = getChildrenBoundingBox();
	}

	// DEBUG("%g %g %g %g, %g %g, %g %g", RECT_ARGS(localBox), VEC_ARGS(internal->fbOffset), VEC_ARGS(internal->fbScale));
	// Transform to world coordinates, then expand to nearest integer coordinates
	math::Vec min = localBox.getTopLeft().mult(internal->fbScale).plus(internal->fbOffset).floor();
	math::Vec max = localBox.getBottomRight().mult(internal->fbScale).plus(internal->fbOffset).ceil();
	internal->fbBox = math::Rect::fromMinMax(min, max);
	// DEBUG("%g %g %g %g", RECT_ARGS(fbBox));

	math::Vec newFbSize = internal->fbBox.size.mult(APP->window->pixelRatio).ceil();

	// Create framebuffer if a new size is needed
	if (!internal->fb || !newFbSize.isEqual(internal->fbSize)) {
		internal->fbSize = newFbSize;
		// Delete old framebuffer
		if (internal->fb)
			nvgluDeleteFramebuffer(internal->fb);
		// Create a framebuffer at the oversampled size
		if (internal->fbSize.isFinite() && !internal->fbSize.isZero())
			internal->fb = nvgluCreateFramebuffer(vg, internal->fbSize.x * oversample, internal->fbSize.y * oversample, 0);
	}

	if (!internal->fb) {
		WARN("Framebuffer of size (%f, %f) * %f could not be created for FramebufferWidget.", VEC_ARGS(internal->fbSize), oversample);
		return;
	}

	nvgluBindFramebuffer(internal->fb);
	drawFramebuffer();
	nvgluBindFramebuffer(NULL);

	// If oversampling, create another framebuffer and copy it to actual size.
	if (oversample != 1.0) {
		NVGLUframebuffer* newFb = nvgluCreateFramebuffer(vg, internal->fbSize.x, internal->fbSize.y, 0);
		if (!newFb) {
			WARN("Non-oversampled framebuffer of size (%f, %f) could not be created for FramebufferWidget.", VEC_ARGS(internal->fbSize));
			return;
		}

		// Use NanoVG for resizing framebuffers
		nvgluBindFramebuffer(newFb);

		nvgBeginFrame(vg, internal->fbBox.size.x, internal->fbBox.size.y, 1.0);

		// Draw oversampled framebuffer
		nvgBeginPath(vg);
		nvgRect(vg, 0.0, 0.0, internal->fbSize.x, internal->fbSize.y);
		NVGpaint paint = nvgImagePattern(vg, 0.0, 0.0, internal->fbSize.x, internal->fbSize.y,
		                                 0.0, internal->fb->image, 1.0);
		nvgFillPaint(vg, paint);
		nvgFill(vg);

		glViewport(0.0, 0.0, internal->fbSize.x, internal->fbSize.y);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		nvgEndFrame(vg);
		nvgReset(vg);

		nvgluBindFramebuffer(NULL);

		// Swap the framebuffers
		nvgluDeleteFramebuffer(internal->fb);
		internal->fb = newFb;
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
	internal->scale = math::Vec(xform[0], xform[3]);
	internal->offset = math::Vec(xform[4], xform[5]);
	math::Vec offsetI = internal->offset.floor();

	math::Vec scaleRatio = math::Vec(1, 1);
	if (!internal->fbScale.isZero() && !internal->scale.isEqual(internal->fbScale)) {
		dirty = true;
		// Continue to draw but at the wrong scale. In the next frame, the framebuffer will be redrawn.
		scaleRatio = internal->scale.div(internal->fbScale);
	}

	if (!internal->fb)
		return;

	// Draw framebuffer image, using world coordinates
	nvgSave(args.vg);
	nvgResetTransform(args.vg);

	nvgBeginPath(args.vg);
	nvgRect(args.vg,
	        offsetI.x + internal->fbBox.pos.x,
	        offsetI.y + internal->fbBox.pos.y,
	        internal->fbBox.size.x * scaleRatio.x, internal->fbBox.size.y * scaleRatio.y);
	NVGpaint paint = nvgImagePattern(args.vg,
	                                 offsetI.x + internal->fbBox.pos.x,
	                                 offsetI.y + internal->fbBox.pos.y,
	                                 internal->fbBox.size.x * scaleRatio.x, internal->fbBox.size.y * scaleRatio.y,
	                                 0.0, internal->fb->image, 1.0);
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

	float pixelRatio = internal->fbSize.x * oversample / internal->fbBox.size.x;
	nvgBeginFrame(vg, internal->fbBox.size.x, internal->fbBox.size.y, pixelRatio);

	// Use local scaling
	nvgTranslate(vg, -internal->fbBox.pos.x, -internal->fbBox.pos.y);
	nvgTranslate(vg, internal->fbOffset.x, internal->fbOffset.y);
	nvgScale(vg, internal->fbScale.x, internal->fbScale.y);

	DrawArgs args;
	args.vg = vg;
	args.clipBox = box.zeroPos();
	args.fb = internal->fb;
	Widget::draw(args);

	glViewport(0.0, 0.0, internal->fbSize.x * oversample, internal->fbSize.y * oversample);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	// glClearColor(0.0, 1.0, 1.0, 0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	nvgEndFrame(vg);

	// Clean up the NanoVG state so that calls to nvgTextBounds() etc during step() don't use a dirty state.
	nvgReset(vg);
}


int FramebufferWidget::getImageHandle() {
	if (!internal->fb)
		return -1;
	return internal->fb->image;
}


NVGLUframebuffer* FramebufferWidget::getFramebuffer() {
	return internal->fb;
}


math::Vec FramebufferWidget::getFramebufferSize() {
	return internal->fbSize;
}


void FramebufferWidget::setScale(math::Vec scale) {
	internal->scale = scale;
}


} // namespace widget
} // namespace rack
