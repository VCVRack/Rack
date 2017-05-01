#include "widgets.hpp"
#include "gui.hpp"
#include <GL/glew.h>
#include "../ext/nanovg/src/nanovg_gl.h"
#include "../ext/nanovg/src/nanovg_gl_utils.h"


namespace rack {


struct FramebufferWidget::Internal {
	NVGLUframebuffer *fb = NULL;

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

void FramebufferWidget::step() {
	// Step children before rendering
	Widget::step();

	// Render the scene to the framebuffer if dirty
	if (dirty) {
		Vec fbSize = box.size.plus(padding.mult(2));
		assert(fbSize.isFinite());

		internal->setFramebuffer(NULL);
		NVGLUframebuffer *fb = nvgluCreateFramebuffer(gVg, fbSize.x, fbSize.y, NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
		if (!fb)
			return;
		internal->setFramebuffer(fb);

		// TODO Support screens with pixelRatio != 1.0 (e.g. Retina) by using the actual size of the framebuffer, etc.
		const float pixelRatio = 1.0;
		nvgluBindFramebuffer(fb);
		glViewport(0.0, 0.0, fbSize.x, fbSize.y);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		nvgBeginFrame(gVg, fbSize.x, fbSize.y, pixelRatio);

		nvgTranslate(gVg, padding.x, padding.y);
		Widget::draw(gVg);

		nvgEndFrame(gVg);
		nvgluBindFramebuffer(NULL);

		dirty = false;
	}
}

void FramebufferWidget::draw(NVGcontext *vg) {
	if (!internal->fb)
		return;

	// Draw framebuffer image
	int width, height;
	nvgImageSize(vg, internal->fb->image, &width, &height);
	nvgBeginPath(vg);
	nvgRect(vg, -padding.x, -padding.y, width, height);
	NVGpaint paint = nvgImagePattern(vg, -padding.x, -padding.y, width, height, 0.0, internal->fb->image, 1.0);
	nvgFillPaint(vg, paint);
	nvgFill(vg);

	// nvgFillColor(vg, nvgRGBA(255, 0, 0, 64));
	// nvgFill(vg);
}


} // namespace rack
