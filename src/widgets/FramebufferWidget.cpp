#include "widgets.hpp"
#include "gui.hpp"
#include <GL/glew.h>
#include "../ext/nanovg/src/nanovg_gl.h"
#include "../ext/nanovg/src/nanovg_gl_utils.h"
#include "../ext/osdialog/osdialog.h"


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
	internal = new Internal();
}

FramebufferWidget::~FramebufferWidget() {
	delete internal;
}

void FramebufferWidget::draw(NVGcontext *vg) {
	if (dirty) {
		internal->box.pos = Vec(0, 0);
		internal->box.size = box.size;
		internal->box.size = Vec(ceilf(internal->box.size.x), ceilf(internal->box.size.y));
		Vec fbSize = internal->box.size.mult(gPixelRatio * oversample);

		// assert(fbSize.isFinite());
		// Reject zero area size
		if (fbSize.x <= 0.0 || fbSize.y <= 0.0)
			return;

		// Delete old one first to free up GPU memory
		internal->setFramebuffer(NULL);
		// Create a framebuffer from the main nanovg context. We will draw to this in the secondary nanovg context.
		NVGLUframebuffer *fb = nvgluCreateFramebuffer(gVg, fbSize.x, fbSize.y, NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
		if (!fb)
			return;
		internal->setFramebuffer(fb);

		nvgluBindFramebuffer(fb);
		glViewport(0.0, 0.0, fbSize.x, fbSize.y);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		nvgBeginFrame(gFramebufferVg, fbSize.x, fbSize.y, gPixelRatio * oversample);

		nvgScale(gFramebufferVg, gPixelRatio * oversample, gPixelRatio * oversample);
		Widget::draw(gFramebufferVg);

		nvgEndFrame(gFramebufferVg);
		nvgluBindFramebuffer(NULL);

		dirty = false;
	}

	if (!internal->fb) {
		return;
	}

	// Draw framebuffer image
	nvgBeginPath(vg);
	nvgRect(vg, internal->box.pos.x, internal->box.pos.y, internal->box.size.x, internal->box.size.y);
	NVGpaint paint = nvgImagePattern(vg, internal->box.pos.x, internal->box.pos.y, internal->box.size.x, internal->box.size.y, 0.0, internal->fb->image, 1.0);
	nvgFillPaint(vg, paint);
	nvgFill(vg);

	// For debugging bounding box of framebuffer image
	// nvgFillColor(vg, nvgRGBA(255, 0, 0, 64));
	// nvgFill(vg);

	{
		float xform[6];
		nvgCurrentTransform(vg, xform);
		// printf("%f %f %f %f; %f %f\n", xform[0], xform[1], xform[2], xform[3], xform[4], xform[5]);
		nvgSave(vg);
		nvgResetTransform(vg);
		nvgTranslate(vg, xform[4], xform[5]);
		nvgScale(vg, xform[0], xform[3]);
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, internal->box.size.x, internal->box.size.y);
		nvgStrokeWidth(vg, 2.0);
		nvgStrokeColor(vg, nvgRGBf(1.0, 0.0, 0.0));
		nvgStroke(vg);
		nvgRestore(vg);
	}
}

int FramebufferWidget::getImageHandle() {
	if (!internal->fb)
		return -1;
	return internal->fb->image;
}


} // namespace rack
