#pragma once
#include <widget/Widget.hpp>


namespace rack {
namespace widget {


/** Caches a widget's draw() result to a framebuffer so it is called less frequently.
When `dirty` is true, its children will be re-rendered on the next call to step().
Events are not passed to the underlying scene.
*/
struct FramebufferWidget : Widget {
	/** Set this to true to re-render the children to the framebuffer the next time it is drawn */
	bool dirty = true;
	bool bypass = false;
	float oversample = 1.0;
	NVGLUframebuffer *fb = NULL;
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

	FramebufferWidget();
	~FramebufferWidget();
	void step() override;
	void draw(const DrawArgs &args) override;
	virtual void drawFramebuffer();
	int getImageHandle();
};


} // namespace widget
} // namespace rack
