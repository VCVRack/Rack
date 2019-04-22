#include "app/RackRail.hpp"


namespace rack {
namespace app {

void RackRail::draw(const DrawArgs &args) {
	const float railHeight = 15;

	// Background color
	nvgBeginPath(args.vg);
	nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
	nvgFillColor(args.vg, nvgRGB(0x30, 0x30, 0x30));
	nvgFill(args.vg);

	// Rails
	nvgFillColor(args.vg, nvgRGB(0xc9, 0xc9, 0xc9));
	nvgStrokeWidth(args.vg, 1.0);
	nvgStrokeColor(args.vg, nvgRGB(0x9d, 0x9f, 0xa2));
	float holeRadius = 4.0;
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
}


} // namespace app
} // namespace rack
