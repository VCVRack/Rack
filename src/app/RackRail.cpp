#include <app/RackRail.hpp>
#include <context.hpp>
#include <asset.hpp>
#include <svg.hpp>


namespace rack {
namespace app {


RackRail::RackRail() {
	busBoardSvg = Svg::load(asset::system("res/ComponentLibrary/RackBusboard.svg"));
	railsSvg = Svg::load(asset::system("res/ComponentLibrary/RackRails.svg"));
	// DEBUG("%d %d %d", railsSvg->getNumShapes(), railsSvg->getNumPaths(), railsSvg->getNumPoints());
}


void RackRail::draw(const DrawArgs& args) {
	// Background color
	nvgBeginPath(args.vg);
	nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
	nvgFillColor(args.vg, nvgRGB(0x30, 0x30, 0x30));
	nvgFill(args.vg);

	// Rails
	for (float y = 0; y < box.size.y; y += RACK_GRID_HEIGHT) {
		const float busBoardWidth = busBoardSvg->handle->width;
		const float busBoardHeight = busBoardSvg->handle->height;
		const float busBoardY = y + (RACK_GRID_HEIGHT - busBoardHeight) / 2;
		const NVGcolor shadowColor = nvgRGBA(0, 0, 0, 0x20);

		// Bus board shadow
		nvgBeginPath(args.vg);
		const float busBoardShadowY = busBoardY + busBoardHeight;
		const float busBoardShadowHeight = 10;
		nvgRect(args.vg, 0, busBoardShadowY, box.size.x, busBoardShadowHeight);
		nvgFillPaint(args.vg, nvgLinearGradient(args.vg, 0, busBoardShadowY, 0, busBoardShadowY + busBoardShadowHeight, shadowColor, color::BLACK_TRANSPARENT));
		nvgFill(args.vg);

		// Bus board
		for (float x = 0; x < box.size.x; x += busBoardWidth) {
			nvgSave(args.vg);
			nvgTranslate(args.vg, x, busBoardY);
			busBoardSvg->draw(args.vg);
			nvgRestore(args.vg);
		}

		// Rails shadow
		nvgBeginPath(args.vg);
		const float railsShadowY = y + 15;
		const float railsShadowHeight = 10;
		nvgRect(args.vg, 0, railsShadowY, box.size.x, railsShadowHeight);
		nvgFillPaint(args.vg, nvgLinearGradient(args.vg, 0, railsShadowY, 0, railsShadowY + railsShadowHeight, shadowColor, color::BLACK_TRANSPARENT));
		nvgFill(args.vg);

		// Rails
		for (float x = 0; x < box.size.x; x += RACK_GRID_WIDTH) {
			nvgSave(args.vg);
			nvgTranslate(args.vg, x, y );
			railsSvg->draw(args.vg);
			nvgRestore(args.vg);
		}
	}
}


} // namespace app
} // namespace rack
