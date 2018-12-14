#pragma once
#include "ui/common.hpp"
#include "ui/MenuEntry.hpp"


namespace rack {


struct MenuSeparator : MenuEntry {
	MenuSeparator() {
		box.size.y = BND_WIDGET_HEIGHT / 2;
	}

	void draw(NVGcontext *vg) override {
		nvgBeginPath(vg);
		const float margin = 8.0;
		nvgMoveTo(vg, margin, box.size.y / 2.0);
		nvgLineTo(vg, box.size.x - margin, box.size.y / 2.0);
		nvgStrokeWidth(vg, 1.0);
		nvgStrokeColor(vg, color::alpha(bndGetTheme()->menuTheme.textColor, 0.25));
		nvgStroke(vg);
	}
};


} // namespace rack
