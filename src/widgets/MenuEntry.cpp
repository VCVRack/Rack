#include "../5V.hpp"


float MenuEntry::computeMinWidth(NVGcontext *vg) {
	return bndLabelWidth(vg, -1, text.c_str());
}
