#include <app/RackRail.hpp>
#include <app.hpp>
#include <asset.hpp>
#include <svg.hpp>


namespace rack {
namespace app {


RackRail::RackRail() {
	busBoardSvg = APP->window->loadSvg(asset::system("res/ComponentLibrary/RackBusboard.svg"));
}


void RackRail::draw(const DrawArgs& args) {
	const float railHeight = 15;

	// Background color
	nvgBeginPath(args.vg);
	nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
	nvgFillColor(args.vg, nvgRGB(0x30, 0x30, 0x30));
	nvgFill(args.vg);

	// Rails
	float holeRadius = 4.0;
	for (float y = 0; y < box.size.y; y += RACK_GRID_HEIGHT) {
		nvgFillColor(args.vg, nvgRGB(0xc9, 0xc9, 0xc9));
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, nvgRGB(0x9d, 0x9f, 0xa2));
		// Top rail
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, y, box.size.x, railHeight);
		for (float x = 0; x < box.size.x; x += RACK_GRID_WIDTH) {
			nvgCircle(args.vg, x + RACK_GRID_WIDTH / 2, y + railHeight / 2, holeRadius);
			nvgPathWinding(args.vg, NVG_HOLE);
		}
		nvgFill(args.vg);

		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, 0, y + railHeight - 0.5);
		nvgLineTo(args.vg, box.size.x, y + railHeight - 0.5);
		nvgStroke(args.vg);

		// Bottom rail
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, y + RACK_GRID_HEIGHT - railHeight, box.size.x, railHeight);
		for (float x = 0; x < box.size.x; x += RACK_GRID_WIDTH) {
			nvgCircle(args.vg, x + RACK_GRID_WIDTH / 2, y + RACK_GRID_HEIGHT - railHeight + railHeight / 2, holeRadius);
			nvgPathWinding(args.vg, NVG_HOLE);
		}
		nvgFill(args.vg);

		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, 0, y + RACK_GRID_HEIGHT - 0.5);
		nvgLineTo(args.vg, box.size.x, y + RACK_GRID_HEIGHT - 0.5);
		nvgStroke(args.vg);

		// Bus board
		const float busBoardWidth = busBoardSvg->handle->width;
		const float busBoardHeight = busBoardSvg->handle->height;
		const float busBoardY = y + (RACK_GRID_HEIGHT - busBoardHeight) / 2;
		for (float x = 0; x < box.size.x; x += busBoardWidth) {
			nvgSave(args.vg);
			nvgTranslate(args.vg, x, busBoardY);
			svgDraw(args.vg, busBoardSvg->handle);
			nvgRestore(args.vg);
		}

		// Bus board shadow
		nvgBeginPath(args.vg);
		const float shadowY = busBoardY + busBoardHeight;
		const float shadowHeight = 10;
		nvgRect(args.vg, 0, shadowY, box.size.x, shadowHeight);
		NVGcolor shadowColor = nvgRGBA(0, 0, 0, 0x20);
		NVGcolor transparentColor = nvgRGBAf(0, 0, 0, 0);
		nvgFillPaint(args.vg, nvgLinearGradient(args.vg, 0, shadowY, 0, shadowY + shadowHeight, shadowColor, transparentColor));
		nvgFill(args.vg);
	}
}


} // namespace app
} // namespace rack
