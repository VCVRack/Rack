#include "app/SvgScrew.hpp"


namespace rack {
namespace app {


SvgScrew::SvgScrew() {
	sw = new widget::SvgWidget;
	addChild(sw);
}


} // namespace app
} // namespace rack
