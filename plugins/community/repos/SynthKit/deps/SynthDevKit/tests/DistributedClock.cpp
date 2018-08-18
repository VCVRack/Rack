#include "testrunner.hpp"
#include "../src/DistributedClock.hpp"

using namespace SynthDevKit;

uint8_t test_distributedclock_master ( ) {
  DistributedClock *clock = new DistributedClock(true, 1.7f);

  clock->update(3, 0.0f, 0.0f);
  check(clock->triggerClock() == false, "clock should not be triggered");
  check(clock->triggerDownstream() == false, "downstream should not be triggered");

  clock->update(3, 0.0f, 1.7f);
  check(clock->triggerClock() == true, "clock should be triggered");
  check(clock->triggerDownstream() == false, "downstream should not be triggered");

  clock->update(3, 0.0f, 0.0f);
  check(clock->triggerClock() == false, "clock should not be triggered");
  check(clock->triggerDownstream() == false, "downstream should not be triggered");

  clock->update(3, 0.0f, 1.7f);
  check(clock->triggerClock() == true, "clock should be triggered");
  check(clock->triggerDownstream() == false, "downstream should not be triggered");

  clock->update(3, 0.0f, 0.0f);
  check(clock->triggerClock() == false, "clock should not be triggered");
  check(clock->triggerDownstream() == false, "downstream should not be triggered");

  clock->update(3, 0.0f, 1.7f);
  check(clock->triggerClock() == true, "clock should be triggered");
  check(clock->triggerDownstream() == true, "downstream should be triggered");

  clock->update(3, 0.0f, 0.0f);
  check(clock->triggerClock() == false, "clock should not be triggered");
  check(clock->triggerDownstream() == false, "downstream should not be triggered");

  clock->update(3, 0.0f, 1.7f);
  check(clock->triggerClock() == false, "clock should not be triggered");
  check(clock->triggerDownstream() == false, "downstream should not be triggered");

  done();
}

uint8_t test_distributedclock_servant ( ) {
  DistributedClock *clock = new DistributedClock(false, 1.7f);

  clock->update(3, 0.0f, 1.7f);
  check(clock->triggerClock() == false, "clock should not be triggered");
  check(clock->triggerDownstream() == false, "downstream should not be triggered");

  clock->update(3, 1.7f, 0.0f);
  check(clock->triggerClock() == false, "clock should not be triggered");
  check(clock->triggerDownstream() == false, "downstream should not be triggered");

  clock->update(3, 0.0f, 1.7f);
  check(clock->triggerClock() == true, "clock should be triggered");
  check(clock->triggerDownstream() == false, "downstream should not be triggered");

  done();
}
