#include "widgets.hpp"


namespace rack {


float MenuEntry::computeMinWidth(NVGcontext *vg) {
	// Add 10 more pixels because Retina measurements are sometimes too small
	return bndLabelWidth(vg, -1, text.c_str()) + 10.0;
}


} // namespace rack
