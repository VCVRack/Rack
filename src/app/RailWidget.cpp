#include <app/RailWidget.hpp>
#include <context.hpp>
#include <asset.hpp>
#include <svg.hpp>


namespace rack {
namespace app {


RailWidget::RailWidget() {
	svg = Svg::load(asset::system("res/ComponentLibrary/Rail.svg"));
	// DEBUG("%d %d %d", svg->getNumShapes(), svg->getNumPaths(), svg->getNumPoints());
}


void RailWidget::draw(const DrawArgs& args) {
	if (!svg)
		return;

	math::Vec tileSize = getTileSize();
	if (tileSize.area() == 0.f)
		return;

	for (float y = 0; y < box.size.y; y += tileSize.y) {
		for (float x = 0; x < box.size.x; x += tileSize.x) {
			nvgSave(args.vg);
			nvgTranslate(args.vg, x, y);
			svg->draw(args.vg);
			nvgRestore(args.vg);
		}
	}

	Widget::draw(args);
}


math::Vec RailWidget::getTileSize() {
	if (!svg)
		return math::Vec();
	return svg->getSize().div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);
}


} // namespace app
} // namespace rack
