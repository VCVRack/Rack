#include "app/RackRail.hpp"


namespace rack {
namespace app {

void RackRail::draw(const DrawArgs &args) {
	const float railHeight = RACK_GRID_WIDTH;

	// Background color
	nvgBeginPath(args.vg);
	nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
	nvgFillColor(args.vg, nvgRGBf(0.2, 0.2, 0.2));
	nvgFill(args.vg);

	// Rails
	nvgFillColor(args.vg, nvgRGBf(0.85, 0.85, 0.85));
	nvgStrokeWidth(args.vg, 1.0);
	nvgStrokeColor(args.vg, nvgRGBf(0.7, 0.7, 0.7));
	float holeRadius = 3.5;
	for (float railY = 0; railY < box.size.y; railY += RACK_GRID_HEIGHT) {
		// Top rail
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, railY, box.size.x, railHeight);
		for (float railX = 0; railX < box.size.x; railX += RACK_GRID_WIDTH) {
			nvgCircle(args.vg, railX + RACK_GRID_WIDTH / 2, railY + railHeight / 2, holeRadius);
			nvgPathWinding(args.vg, NVG_HOLE);
		}
		nvgFill(args.vg);

		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, 0, railY + railHeight - 0.5);
		nvgLineTo(args.vg, box.size.x, railY + railHeight - 0.5);
		nvgStroke(args.vg);

		// Bottom rail
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, railY + RACK_GRID_HEIGHT - railHeight, box.size.x, railHeight);
		for (float railX = 0; railX < box.size.x; railX += RACK_GRID_WIDTH) {
			nvgCircle(args.vg, railX + RACK_GRID_WIDTH / 2, railY + RACK_GRID_HEIGHT - railHeight + railHeight / 2, holeRadius);
			nvgPathWinding(args.vg, NVG_HOLE);
		}
		nvgFill(args.vg);

		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, 0, railY + RACK_GRID_HEIGHT - 0.5);
		nvgLineTo(args.vg, box.size.x, railY + RACK_GRID_HEIGHT - 0.5);
		nvgStroke(args.vg);
	}


	// Useful for screenshots
	if (0) {
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
		nvgFillColor(args.vg, nvgRGBf(1.0, 1.0, 1.0));
		nvgFill(args.vg);
	}
}


} // namespace app
} // namespace rack
