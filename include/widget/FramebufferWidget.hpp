#pragma once
#include <widget/Widget.hpp>


namespace rack {
namespace widget {


/** Caches a widget's draw() result to a framebuffer so it is called less frequently.
When `dirty` is true, its children will be re-rendered on the next call to step().
Events are not passed to the underlying scene.
*/
struct FramebufferWidget : Widget {
	struct Internal;
	Internal* internal;

	/** Set this to true to re-render the children to the framebuffer the next time it is drawn */
	bool dirty = true;
	bool bypass = false;
	float oversample = 1.0;
	/** Redraw when the world offset of the FramebufferWidget changes its fractional value. */
	bool dirtyOnSubpixelChange = true;

	FramebufferWidget();
	~FramebufferWidget();
	void setDirty(bool dirty = true);
	void onDirty(const event::Dirty& e) override;
	void step() override;
	void draw(const DrawArgs& args) override;
	virtual void drawFramebuffer();
	int getImageHandle();
	NVGLUframebuffer* getFramebuffer();
	math::Vec getFramebufferSize();
	void setScale(math::Vec scale);
};


} // namespace widget
} // namespace rack
