#include "../../include/PianoRoll/Auditioner.hpp"

namespace rack_plugin_rcm {

void Auditioner::start(int step) {
  if (this->step != step || !auditioning) {
    needsRetrigger = true;
  }

  this->step = step;
  auditioning = true;
}

void Auditioner::retrigger() {
  needsRetrigger = true;
}

void Auditioner::stop() {
  auditioning = false;
  needsRetrigger = false;
  stopPending = true;
}

bool Auditioner::isAuditioning() {
  return auditioning;
}

int Auditioner::stepToAudition() {
  return step;
}

bool Auditioner::consumeRetrigger() {
  if (needsRetrigger) {
    needsRetrigger = false;
    return true;
  } else {
    return false;
  }
}

bool Auditioner::consumeStopEvent() {
  if (stopPending) {
    stopPending = false;
    return true;
  } else {
    return false;
  }
}

} // namespace rack_plugin_rcm
