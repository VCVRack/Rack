#include "widgets.hpp"


namespace rack {


float MenuEntry::computeMinWidth(NVGcontext *vg) {
	return bndLabelWidth(vg, -1, text.c_str());
}


} // namespace rack
