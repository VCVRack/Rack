#pragma once
#include <app/common.hpp>
#include <app/SvgSwitch.hpp>


namespace rack {
namespace app {


/** A ParamWidget that behaves like a Switch but draws SVG frames for mouse up/down state.
*/
struct SvgLatch : SvgSwitch {
	struct Internal;
	Internal* internal;

	SvgLatch();
	~SvgLatch();
	void onDragStart(const DragStartEvent& e) override;
	void onDragEnd(const DragEndEvent& e) override;
	void onChange(const ChangeEvent& e) override;
};


} // namespace app
} // namespace rack
