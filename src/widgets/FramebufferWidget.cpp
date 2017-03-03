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
	if (scene) {
		delete scene;
	}
	delete internal;
}

void FramebufferWidget::step() {
	if (!scene)
		return;

	// Step scene before rendering
	scene->step();

	// Render the scene to the framebuffer if dirty
	if (dirty) {
		assert(scene->box.size.isFinite());
		int width = ceilf(scene->box.size.x) + 2*margin;
		int height = ceilf(scene->box.size.y) + 2*margin;

		internal->setFramebuffer(NULL);
		NVGLUframebuffer *fb = nvgluCreateFramebuffer(gVg, width, height, NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
		if (!fb)
			return;
		internal->setFramebuffer(fb);

		// TODO Support screens with pixelRatio != 1.0 (e.g. Retina) by using the actual size of the framebuffer, etc.
		const float pixelRatio = 1.0;
		nvgluBindFramebuffer(fb);
		glViewport(0.0, 0.0, width, height);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		nvgBeginFrame(gVg, width, height, pixelRatio);

		nvgTranslate(gVg, margin, margin);
		scene->draw(gVg);

		nvgEndFrame(gVg);
		nvgluBindFramebuffer(NULL);

		dirty = false;
	}
}

void FramebufferWidget::draw(NVGcontext *vg) {
	if (!internal->fb)
		return;
	if (!scene)
		return;

	// Draw framebuffer image
	int width, height;
	nvgImageSize(vg, internal->fb->image, &width, &height);
	nvgBeginPath(vg);
	nvgRect(vg, -margin, -margin, width, height);
	NVGpaint paint = nvgImagePattern(vg, -margin + scene->box.pos.x, -margin + scene->box.pos.y, width, height, 0.0, internal->fb->image, 1.0);
	nvgFillPaint(vg, paint);
	nvgFill(vg);
}


} // namespace rack
