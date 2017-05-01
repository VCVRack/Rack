#include "app.hpp"


namespace rack {


SVGScrew::SVGScrew() {
	padding = Vec(1, 1);

	sw = new SVGWidget();
	addChild(sw);
}


} // namespace rack
