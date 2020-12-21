#include <widget/FramebufferWidget.hpp>
#include <context.hpp>
#include <random.hpp>


namespace rack {
namespace widget {


struct FramebufferWidget::Internal {
	NVGLUframebuffer* fb = NULL;

	// Set by draw()

	/** Scale relative to the world */
	math::Vec scale;
	/** Subpixel offset in world coordinates */
	math::Vec offsetF;

	// Set by step() and drawFramebuffer()

	/** Pixel dimensions of the allocated framebuffer */
	math::Vec fbSize;
	/** Bounding box in world coordinates of where the framebuffer should be painted.
	Always has integer coordinates so that blitting framebuffers is pixel-perfect.
	*/
	math::Rect fbBox;
	/** Framebuffer's scale relative to the world */
	math::Vec fbScale;
	/** Framebuffer's subpixel offset relative to fbBox in world coordinates */
	math::Vec fbOffsetF;
};


FramebufferWidget::FramebufferWidget() {
	internal = new Internal;
}


FramebufferWidget::~FramebufferWidget() {
	if (internal->fb)
		nvgluDeleteFramebuffer(internal->fb);
	delete internal;
}


void FramebufferWidget::setDirty(bool dirty) {
	this->dirty = dirty;
}


void FramebufferWidget::onDirty(const event::Dirty& e) {
	dirty = true;
	Widget::onDirty(e);
}


void FramebufferWidget::step() {
	Widget::step();

	// It's more important to not lag the frame than to draw the framebuffer
	if (APP->window->getFrameTimeOverdue() > 0.0)
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
	internal->fbOffsetF = internal->offsetF;

	math::Rect localBox;
	if (children.empty()) {
		localBox = box.zeroPos();
	}
	else {
		localBox = getVisibleChildrenBoundingBox();
	}

	// DEBUG("rendering FramebufferWidget localBox (%g %g %g %g) fbOffset (%g %g) fbScale (%g %g)", RECT_ARGS(localBox), VEC_ARGS(internal->fbOffsetF), VEC_ARGS(internal->fbScale));
	// Transform to world coordinates, then expand to nearest integer coordinates
	math::Vec min = localBox.getTopLeft().mult(internal->fbScale).plus(internal->fbOffsetF).floor();
	math::Vec max = localBox.getBottomRight().mult(internal->fbScale).plus(internal->fbOffsetF).ceil();
	internal->fbBox = math::Rect::fromMinMax(min, max);
	// DEBUG("%g %g %g %g", RECT_ARGS(internal->fbBox));

	math::Vec newFbSize = internal->fbBox.size.mult(APP->window->pixelRatio).ceil();

	// Create framebuffer if a new size is needed
	if (!internal->fb || !newFbSize.isEqual(internal->fbSize)) {
		internal->fbSize = newFbSize;
		// Delete old framebuffer
		if (internal->fb)
			nvgluDeleteFramebuffer(internal->fb);
		// Create a framebuffer
		if (internal->fbSize.isFinite() && !internal->fbSize.isZero()) {
			internal->fb = nvgluCreateFramebuffer(vg, internal->fbSize.x, internal->fbSize.y, 0);
		}
	}
	if (!internal->fb) {
		WARN("Framebuffer of size (%f, %f) could not be created for FramebufferWidget %p.", VEC_ARGS(internal->fbSize), this);
		return;
	}

	// Render to framebuffer
	if (oversample == 1.0) {
		// If not oversampling, render directly to framebuffer.
		nvgluBindFramebuffer(internal->fb);
		drawFramebuffer();
		nvgluBindFramebuffer(NULL);
	}
	else {
		NVGLUframebuffer* fb = internal->fb;
		// If oversampling, create another framebuffer and copy it to actual size.
		math::Vec oversampledFbSize = internal->fbSize.mult(oversample).ceil();
		NVGLUframebuffer* oversampledFb = nvgluCreateFramebuffer(vg, oversampledFbSize.x, oversampledFbSize.y, 0);

		if (!oversampledFb) {
			WARN("Oversampled framebuffer of size (%f, %f) could not be created for FramebufferWidget %p.", VEC_ARGS(oversampledFbSize), this);
			return;
		}

		// Render to oversampled framebuffer.
		nvgluBindFramebuffer(oversampledFb);
		internal->fb = oversampledFb;
		drawFramebuffer();
		internal->fb = fb;
		nvgluBindFramebuffer(NULL);

		// Use NanoVG for resizing framebuffers
		nvgluBindFramebuffer(internal->fb);

		nvgBeginFrame(vg, internal->fbBox.size.x, internal->fbBox.size.y, 1.0);

		// Draw oversampled framebuffer
		nvgBeginPath(vg);
		nvgRect(vg, 0.0, 0.0, internal->fbSize.x, internal->fbSize.y);
		NVGpaint paint = nvgImagePattern(vg, 0.0, 0.0,
			internal->fbSize.x, internal->fbSize.y,
			0.0, oversampledFb->image, 1.0);
		nvgFillPaint(vg, paint);
		nvgFill(vg);

		glViewport(0.0, 0.0, internal->fbSize.x, internal->fbSize.y);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		nvgEndFrame(vg);
		nvgReset(vg);

		nvgluBindFramebuffer(NULL);
		nvgluDeleteFramebuffer(oversampledFb);
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
	math::Vec offset = math::Vec(xform[4], xform[5]);
	math::Vec offsetI = offset.floor();
	internal->offsetF = offset.minus(offsetI);

	if (dirtyOnSubpixelChange && !(math::isNear(internal->offsetF.x, internal->fbOffsetF.x, 0.01f) && math::isNear(internal->offsetF.y, internal->fbOffsetF.y, 0.01f))) {
		// If drawing to a new subpixel location, rerender in the next frame.
		// DEBUG("%p dirty subpixel", this);
		dirty = true;
	}
	if (!internal->scale.isEqual(internal->fbScale)) {
		// If rescaled, rerender in the next frame.
		// DEBUG("%p dirty scale", this);
		dirty = true;
	}

	math::Vec scaleRatio = math::Vec(1, 1);
	if (!internal->fbScale.isZero() && !internal->scale.isEqual(internal->fbScale)) {
		// Continue to draw with the last framebuffer, but stretch it to rescale.
		scaleRatio = internal->scale.div(internal->fbScale);
	}
	// DEBUG("%f %f %f %f", scaleRatio.x, scaleRatio.y, offsetF.x, offsetF.y);

	if (!internal->fb)
		return;

	// Draw framebuffer image, using world coordinates
	nvgSave(args.vg);
	nvgResetTransform(args.vg);

	// DEBUG("%f %f %f %f, %f %f", RECT_ARGS(internal->fbBox), VEC_ARGS(internal->fbSize));
	nvgBeginPath(args.vg);
	nvgRect(args.vg,
	        offsetI.x + internal->fbBox.pos.x,
	        offsetI.y + internal->fbBox.pos.y,
	        internal->fbBox.size.x * scaleRatio.x,
	        internal->fbBox.size.y * scaleRatio.y);
	NVGpaint paint = nvgImagePattern(args.vg,
	                                 offsetI.x + internal->fbBox.pos.x,
	                                 offsetI.y + internal->fbBox.pos.y,
	                                 internal->fbBox.size.x * scaleRatio.x,
	                                 internal->fbBox.size.y * scaleRatio.y,
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
	nvgSave(vg);

	float pixelRatio = internal->fbSize.x * oversample / internal->fbBox.size.x;
	nvgBeginFrame(vg, internal->fbBox.size.x, internal->fbBox.size.y, pixelRatio);

	// Use local scaling
	nvgTranslate(vg, -internal->fbBox.pos.x, -internal->fbBox.pos.y);
	nvgTranslate(vg, internal->fbOffsetF.x, internal->fbOffsetF.y);
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
	nvgRestore(vg);
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
