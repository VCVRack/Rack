#include "app.hpp"
#include "window.hpp"
#include "engine.hpp"
// For GLFW_KEY_LEFT_CONTROL, etc.
#include <GLFW/glfw3.h>


namespace rack {


static const float KNOB_SENSITIVITY = 0.0015f;

Knob::Knob() {
	// smooth = true;
	smooth = false;  // xxx
}

void Knob::onDragStart(EventDragStart &e) {
	windowCursorLock();
   // printf("xxx Knob::onDragStart: value=%f\n", value);
   dragValue = value;
	randomizable = false;
}

void Knob::onDragMove(EventDragMove &e) {
	float range;
   // printf("xxx Knob::onDragMove: minValue=%f maxValue=%f\n", minValue, maxValue);

	if (isfinite(minValue) && isfinite(maxValue)) {
      // Regular knob
		range = maxValue - minValue;

      float delta = KNOB_SENSITIVITY * -e.mouseRel.y * speed * range;
      // Drag slower if Mod is held
      if (windowIsModPressed())
         delta /= 16.f;
      dragValue += delta;
      dragValue = clamp2(dragValue, minValue, maxValue);

      if (snap)
         setValue(roundf(dragValue));
      else
         setValue(dragValue);

      // printf("xxx Knob::onDragMove: dragValue=%f delta=%f value=%f snap=%d\n", dragValue, delta, value, snap);
	}
	else {
		// Continuous encoders scale as if their limits are +/-1
		range = 1.f - (-1.f);
      // printf("xxx Knob::onDragMove: range=inf (+-1)\n");

      float delta = KNOB_SENSITIVITY * -e.mouseRel.y * speed * range;
      // Drag slower if Mod is held
      if (windowIsModPressed())
         delta /= 16.f;

      dragValue += delta;

      this->value = dragValue;

      // printf("xxx Knob::onDragMove: inf dragValue=%f delta=%f value=%f\n", dragValue, delta, value);

      EventChange e;
      onChange(e);

      // printf("xxx Knob::onDragMove: LEAVE inf dragValue=%f delta=%f value=%f\n", dragValue, delta, value);
	}
}

void Knob::onDragEnd(EventDragEnd &e) {
	windowCursorUnlock();
	randomizable = true;
}


} // namespace rack
