#include "DistributedClock.hpp"

namespace SynthDevKit {
  DistributedClock::DistributedClock (bool master, float trigger) {
    isMaster = master;
    upstreamCV = new CV(trigger);
    clockCV = new CV(trigger);

    reset();
  }

  void DistributedClock::update (uint8_t count, float upstreamValue, float clockValue) {
    upstreamCV->update(upstreamValue);
    clockCV->update(clockValue);
    totalTriggers = count;

    if (upstreamCV->newTrigger()) {
      reset();

      isActive = true;
      totalTriggers = count;
    }
  }

  void DistributedClock::reset ( ) {
    upstreamCV->reset();
    clockCV->reset();

    if (isMaster) {
      isActive = true;
    } else {
      isActive = false;
    }

    triggerCount = 0;
    totalTriggers = 0;
  }

  bool DistributedClock::triggerClock ( ) {
    if (triggerCount == totalTriggers) {
      return false;
    }

    if (clockCV->newTrigger() && isActive) {
      triggerCount++;
      return true;
    }

    return false;
  }

  bool DistributedClock::triggerDownstream ( ) {
    if (triggerCount == totalTriggers && isActive) {
      isActive = false;
      return true;
    }

    return false;
  }
}
