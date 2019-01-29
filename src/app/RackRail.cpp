#include "app/RackRail.hpp"


namespace rack {
namespace app {

void RackRail::draw(const widget::DrawContext &ctx) {
	const float railHeight = RACK_GRID_WIDTH;

	// Background color
	nvgBeginPath(ctx.vg);
	nvgRect(ctx.vg, 0.0, 0.0, box.size.x, box.size.y);
	nvgFillColor(ctx.vg, nvgRGBf(0.2, 0.2, 0.2));
	nvgFill(ctx.vg);

	// Rails
	nvgFillColor(ctx.vg, nvgRGBf(0.85, 0.85, 0.85));
	nvgStrokeWidth(ctx.vg, 1.0);
	nvgStrokeColor(ctx.vg, nvgRGBf(0.7, 0.7, 0.7));
	float holeRadius = 3.5;
	for (float railY = 0; railY < box.size.y; railY += RACK_GRID_HEIGHT) {
		// Top rail
		nvgBeginPath(ctx.vg);
		nvgRect(ctx.vg, 0, railY, box.size.x, railHeight);
		for (float railX = 0; railX < box.size.x; railX += RACK_GRID_WIDTH) {
			nvgCircle(ctx.vg, railX + RACK_GRID_WIDTH / 2, railY + railHeight / 2, holeRadius);
			nvgPathWinding(ctx.vg, NVG_HOLE);
		}
		nvgFill(ctx.vg);

		nvgBeginPath(ctx.vg);
		nvgMoveTo(ctx.vg, 0, railY + railHeight - 0.5);
		nvgLineTo(ctx.vg, box.size.x, railY + railHeight - 0.5);
		nvgStroke(ctx.vg);

		// Bottom rail
		nvgBeginPath(ctx.vg);
		nvgRect(ctx.vg, 0, railY + RACK_GRID_HEIGHT - railHeight, box.size.x, railHeight);
		for (float railX = 0; railX < box.size.x; railX += RACK_GRID_WIDTH) {
			nvgCircle(ctx.vg, railX + RACK_GRID_WIDTH / 2, railY + RACK_GRID_HEIGHT - railHeight + railHeight / 2, holeRadius);
			nvgPathWinding(ctx.vg, NVG_HOLE);
		}
		nvgFill(ctx.vg);

		nvgBeginPath(ctx.vg);
		nvgMoveTo(ctx.vg, 0, railY + RACK_GRID_HEIGHT - 0.5);
		nvgLineTo(ctx.vg, box.size.x, railY + RACK_GRID_HEIGHT - 0.5);
		nvgStroke(ctx.vg);
	}


	// Useful for screenshots
	if (0) {
		nvgBeginPath(ctx.vg);
		nvgRect(ctx.vg, 0.0, 0.0, box.size.x, box.size.y);
		nvgFillColor(ctx.vg, nvgRGBf(1.0, 1.0, 1.0));
		nvgFill(ctx.vg);
	}
}


} // namespace app
} // namespace rack
