#include "app/SVGScrew.hpp"


namespace rack {
namespace app {


SVGScrew::SVGScrew() {
	sw = new widget::SVGWidget;
	addChild(sw);
}


} // namespace app
} // namespace rack
