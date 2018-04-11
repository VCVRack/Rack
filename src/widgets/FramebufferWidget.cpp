#include "widgets.hpp"
#include "window.hpp"
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"


namespace rack {


struct FramebufferWidget::Internal {
	NVGLUframebuffer *fb = NULL;
	Rect box;

	~Internal() {
		setFramebuffer(NULL);
	}
	void setFramebuffer(NVGLUframebuffer *fb) {
		if (this->fb)
			nvgluDeleteFramebuffer(this->fb);
		this->fb = fb;
	}
};


FramebufferWidget::FramebufferWidget() {
	oversample = 1.0;
	internal = new Internal();
}

FramebufferWidget::~FramebufferWidget() {
	delete internal;
}

void FramebufferWidget::draw(NVGcontext *vg) {
	// Bypass framebuffer rendering entirely
	// Widget::draw(vg);
	// return;

	// Get world transform
	float xform[6];
	nvgCurrentTransform(vg, xform);
	// Skew and rotate is not supported
	assert(fabsf(xform[1]) < 1e-6);
	assert(fabsf(xform[2]) < 1e-6);
	Vec s = Vec(xform[0], xform[3]);
	Vec b = Vec(xform[4], xform[5]);
	Vec bi = b.floor();
	Vec bf = b.minus(bi);

	// Render to framebuffer
	if (dirty) {
		dirty = false;

		internal->box = getChildrenBoundingBox();
		internal->box.pos = internal->box.pos.mult(s).floor();
		internal->box.size = internal->box.size.mult(s).ceil().plus(Vec(1, 1));

		Vec fbSize = internal->box.size.mult(gPixelRatio * oversample);

		if (!fbSize.isFinite())
			return;
		if (fbSize.isZero())
			return;

		// info("rendering framebuffer %f %f", fbSize.x, fbSize.y);
		// Delete old one first to free up GPU memory
		internal->setFramebuffer(NULL);
		// Create a framebuffer from the main nanovg context. We will draw to this in the secondary nanovg context.
		NVGLUframebuffer *fb = nvgluCreateFramebuffer(gVg, fbSize.x, fbSize.y, 0);
		if (!fb)
			return;
		internal->setFramebuffer(fb);

		nvgluBindFramebuffer(fb);
		glViewport(0.0, 0.0, fbSize.x, fbSize.y);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		nvgBeginFrame(gFramebufferVg, fbSize.x, fbSize.y, gPixelRatio * oversample);

		nvgScale(gFramebufferVg, gPixelRatio * oversample, gPixelRatio * oversample);
		// Use local scaling
		nvgTranslate(gFramebufferVg, bf.x, bf.y);
		nvgTranslate(gFramebufferVg, -internal->box.pos.x, -internal->box.pos.y);
		nvgScale(gFramebufferVg, s.x, s.y);
		Widget::draw(gFramebufferVg);

		nvgEndFrame(gFramebufferVg);
		nvgluBindFramebuffer(NULL);
	}

	if (!internal->fb) {
		return;
	}

	// Draw framebuffer image, using world coordinates
	nvgSave(vg);
	nvgResetTransform(vg);
	nvgTranslate(vg, bi.x, bi.y);

	nvgBeginPath(vg);
	nvgRect(vg, internal->box.pos.x, internal->box.pos.y, internal->box.size.x, internal->box.size.y);
	NVGpaint paint = nvgImagePattern(vg, internal->box.pos.x, internal->box.pos.y, internal->box.size.x, internal->box.size.y, 0.0, internal->fb->image, 1.0);
	nvgFillPaint(vg, paint);
	nvgFill(vg);

	// For debugging the bounding box of the framebuffer
	// nvgStrokeWidth(vg, 2.0);
	// nvgStrokeColor(vg, nvgRGBA(255, 0, 0, 128));
	// nvgStroke(vg);

	nvgRestore(vg);
}

int FramebufferWidget::getImageHandle() {
	if (!internal->fb)
		return -1;
	return internal->fb->image;
}

void FramebufferWidget::onZoom(EventZoom &e) {
	dirty = true;
	Widget::onZoom(e);
}


} // namespace rack
