#include "app.hpp"


namespace rack {

void PanelBorder::draw(NVGcontext *vg) {
	nvgBeginPath(vg);
	nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);

	NVGcolor borderColor = nvgRGB(0xac, 0xac, 0xac);

	// Border
	nvgBeginPath(vg);
	nvgRect(vg, 0.5, 0.5, box.size.x - 1, box.size.y - 1);
	nvgStrokeColor(vg, borderColor);
	nvgStrokeWidth(vg, 1.0);
	nvgStroke(vg);
}

} // namespace rack
