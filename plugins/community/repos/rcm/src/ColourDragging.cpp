#include "../include/ColourDragging.hpp"
#include "../include/BaseWidget.hpp"
#include "window.hpp"

using namespace rack;

namespace rack_plugin_rcm {

static const float COLOURDRAG_SENSITIVITY = 0.0015f;

ColourDragging::ColourDragging(BaseWidget* widget) : widget(widget) {
  windowCursorLock();
}

ColourDragging::~ColourDragging() {
	windowCursorUnlock();
}

void ColourDragging::onDragMove(EventDragMove& e) {
	float speed = 1.f;
	float range = 1.f;

	float delta = COLOURDRAG_SENSITIVITY * e.mouseRel.y * speed * range;
	if (windowIsModPressed()) {
		delta /= 16.f;
	}

	widget->backgroundHue = clamp(widget->backgroundHue + delta, 0.f, 1.f);
}

} // namespace rack_plugin_rcm
