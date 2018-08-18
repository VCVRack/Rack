#include "CVEvent.hpp"

namespace SynthDevKit {
  CVEvent::CVEvent (float value, int16_t count) : CV(value) {
    this->count = count;
  }

  void CVEvent::update (float value) {
    if (triggerTotal() == count) {
      triggerCount = 0;
    }
    CV::update(value);

    if (triggered == true && lastTriggered == false) {
      emit(has_emitted ? triggerTotal() : 1, currentValue());
    }
  }

  void CVEvent::reset ( )  {
    CV::reset();

    emit(EVENT_RESET, 0.0f);
  }
}
