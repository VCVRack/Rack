#pragma once
#include "widgets/Widget.hpp"


namespace rack {


/** Caches a widget's draw() result to a framebuffer so it is called less frequently
When `dirty` is true, its children will be re-rendered on the next call to step() override.
Events are not passed to the underlying scene.
*/
struct FramebufferWidget : Widget {
	/** Set this to true to re-render the children to the framebuffer the next time it is drawn */
	bool dirty = true;
	float oversample;
	NVGLUframebuffer *fb = NULL;
	/** Pixel dimensions of the allocated framebuffer */
	math::Vec fbSize;
	/** Bounding box in world coordinates of where the framebuffer should be painted
	Always has integer coordinates so that blitting framebuffers is pixel-perfect.
	*/
	math::Rect fbBox;
	/** Local scale relative to the world scale */
	math::Vec fbScale;
	/** Subpixel offset of fbBox in world coordinates */
	math::Vec fbOffset;

	FramebufferWidget();
	~FramebufferWidget();
	void draw(NVGcontext *vg) override;
	virtual void drawFramebuffer();
	int getImageHandle();

	void onZoom(const event::Zoom &e) override {
		dirty = true;
		Widget::onZoom(e);
	}
};


} // namespace rack
