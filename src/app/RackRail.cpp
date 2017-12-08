#include "app.hpp"


namespace rack {

void RackRail::draw(NVGcontext *vg) {
	const float railHeight = RACK_GRID_WIDTH;

	// Background color
	nvgBeginPath(vg);
	nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);
	nvgFillColor(vg, nvgRGBf(0.2, 0.2, 0.2));
	nvgFill(vg);

	// Rails
	nvgFillColor(vg, nvgRGBf(0.85, 0.85, 0.85));
	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, nvgRGBf(0.7, 0.7, 0.7));
	float holeRadius = 3.5;
	for (float railY = 0; railY < box.size.y; railY += RACK_GRID_HEIGHT) {
		// Top rail
		nvgBeginPath(vg);
		nvgRect(vg, 0, railY, box.size.x, railHeight);
		for (float railX = 0; railX < box.size.x; railX += RACK_GRID_WIDTH) {
			nvgCircle(vg, railX + RACK_GRID_WIDTH / 2, railY + railHeight / 2, holeRadius);
			nvgPathWinding(vg, NVG_HOLE);
		}
		nvgFill(vg);

		nvgBeginPath(vg);
		nvgMoveTo(vg, 0, railY + railHeight - 0.5);
		nvgLineTo(vg, box.size.x, railY + railHeight - 0.5);
		nvgStroke(vg);

		// Bottom rail
		nvgBeginPath(vg);
		nvgRect(vg, 0, railY + RACK_GRID_HEIGHT - railHeight, box.size.x, railHeight);
		for (float railX = 0; railX < box.size.x; railX += RACK_GRID_WIDTH) {
			nvgCircle(vg, railX + RACK_GRID_WIDTH / 2, railY + RACK_GRID_HEIGHT - railHeight + railHeight / 2, holeRadius);
			nvgPathWinding(vg, NVG_HOLE);
		}
		nvgFill(vg);

		nvgBeginPath(vg);
		nvgMoveTo(vg, 0, railY + RACK_GRID_HEIGHT - 0.5);
		nvgLineTo(vg, box.size.x, railY + RACK_GRID_HEIGHT - 0.5);
		nvgStroke(vg);
	}


	// Useful for screenshots
	if (0) {
		nvgBeginPath(vg);
		nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);
		nvgFillColor(vg, nvgRGBf(1.0, 1.0, 1.0));
		nvgFill(vg);
	}
}


} // namespace rack
