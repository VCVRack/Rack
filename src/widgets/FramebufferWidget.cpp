#include "global_pre.hpp"
#include "widgets.hpp"
#include "window.hpp"
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"
#include "global_ui.hpp"

// #include <windows.h>

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
   // printf("xxx FramebufferWidget::draw: global_ui=%p\n", global_ui);
#ifdef RACK_PLUGIN_SHARED
   bool bFBO = global_ui->b_fbo_shared;
#else
   bool bFBO = global_ui->b_fbo;
#endif // RACK_PLUGIN_SHARED
   // printf("xxx FramebufferWidget::draw: global_ui=%p, bFBO=%d\n", global_ui, bFBO);
   // (note) FBO path crashes when plugin is a DLL (!)
   //         (the glGenFramebuffers() call in nvgluCreateFramebuffer() to be precise)
   if(!bFBO)
   {
      Widget::draw(vg);
      return;
   }
   // printf("xxx FramebufferWidget::draw: ENTER vg=%p\n", vg);
   // printf("xxx FramebufferWidget::draw: GetCurrentThreadId=%d\n", GetCurrentThreadId());

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

   // printf("xxx FramebufferWidget::draw: 2 dirty=%d\n", dirty);

	// Render to framebuffer
	if (dirty) {
		dirty = false;

      // printf("xxx FramebufferWidget::draw: 2b internal=%p\n", internal);

		internal->box = getChildrenBoundingBox();
		internal->box.pos = internal->box.pos.mult(s).floor();
		internal->box.size = internal->box.size.mult(s).ceil().plus(Vec(1, 1));

      // printf("xxx FramebufferWidget::draw: 3b\n");

		Vec fbSize = internal->box.size.mult(global_ui->window.gPixelRatio * oversample);

      // printf("xxx FramebufferWidget::draw: 4b\n");

		if (!fbSize.isFinite())
			return;
		if (fbSize.isZero())
			return;

      // printf("xxx FramebufferWidget::draw: 5b\n");

		// info("rendering framebuffer %f %f", fbSize.x, fbSize.y);
		// Delete old one first to free up GPU memory
		internal->setFramebuffer(NULL);
      // printf("xxx FramebufferWidget::draw: 5b2 global_ui->window.gVg=%p nvgluCreateFramebuffer=%p global_ui=%p\n", global_ui->window.gVg, global_ui);
		// Create a framebuffer from the main nanovg context. We will draw to this in the secondary nanovg context.
      NVGLUframebuffer *fb = nvgluCreateFramebuffer(global_ui->window.gVg, fbSize.x, fbSize.y, 0);
		// // NVGLUframebuffer *fb = nvgluCreateFramebuffer(NULL, 0, 0, 0);
      // printf("xxx FramebufferWidget::draw: 6b fb=%p\n", fb);
		if (!fb)
			return;
		internal->setFramebuffer(fb);

      // printf("xxx FramebufferWidget::draw: 7b\n");

		nvgluBindFramebuffer(fb);
		glViewport(0.0, 0.0, fbSize.x, fbSize.y);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      // printf("xxx FramebufferWidget::draw: 8b\n");

		nvgBeginFrame(global_ui->window.gFramebufferVg, fbSize.x, fbSize.y, global_ui->window.gPixelRatio * oversample);

		nvgScale(global_ui->window.gFramebufferVg, global_ui->window.gPixelRatio * oversample, global_ui->window.gPixelRatio * oversample);

      // printf("xxx FramebufferWidget::draw: 9b\n");

		// Use local scaling
		nvgTranslate(global_ui->window.gFramebufferVg, bf.x, bf.y);
		nvgTranslate(global_ui->window.gFramebufferVg, -internal->box.pos.x, -internal->box.pos.y);
		nvgScale(global_ui->window.gFramebufferVg, s.x, s.y);
      // printf("xxx FramebufferWidget::draw: 10b\n");
		Widget::draw(global_ui->window.gFramebufferVg);
      // printf("xxx FramebufferWidget::draw: 11b\n");

		nvgEndFrame(global_ui->window.gFramebufferVg);
      // printf("xxx FramebufferWidget::draw: 12b\n");
		nvgluBindFramebuffer(NULL);
      // printf("xxx FramebufferWidget::draw: 13b\n");
	}

   // printf("xxx FramebufferWidget::draw: 3\n");

	if (!internal->fb) {
		return;
	}

   // printf("xxx FramebufferWidget::draw: 4\n");

	// Draw framebuffer image, using world coordinates
	nvgSave(vg);
	nvgResetTransform(vg);
	nvgTranslate(vg, bi.x, bi.y);

   // printf("xxx FramebufferWidget::draw: 5\n");

	nvgBeginPath(vg);
	nvgRect(vg, internal->box.pos.x, internal->box.pos.y, internal->box.size.x, internal->box.size.y);
	NVGpaint paint = nvgImagePattern(vg, internal->box.pos.x, internal->box.pos.y, internal->box.size.x, internal->box.size.y, 0.0, internal->fb->image, 1.0);
	nvgFillPaint(vg, paint);
	nvgFill(vg);

   // printf("xxx FramebufferWidget::draw: 6\n");

	// For debugging the bounding box of the framebuffer
	// nvgStrokeWidth(vg, 2.0);
	// nvgStrokeColor(vg, nvgRGBA(255, 0, 0, 128));
	// nvgStroke(vg);

	nvgRestore(vg);

   // printf("xxx FramebufferWidget::draw: LEAVE\n");
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
