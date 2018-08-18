#include "testrunner.hpp"
#include "../src/Clock.hpp"

using namespace SynthDevKit;

uint8_t test_clock_exception ( ) {
  bool has_exception = 0;
  Clock *clock = {0};

  try {
    clock = new Clock(CLOCK_LIMIT + 1, 1.5);
  } catch (int err) {
    has_exception = true;
  }

  check(!clock, "clock is null");
  check(has_exception, "exception is thrown");

  done();
}

uint8_t test_clock_update ( ) {
  Clock *clock = new Clock(4, 1.5);
  bool *results;

  results = clock->update(0);

  for (uint16_t i = 0; i < 4; i++) {
    check(results[i] == false, "not triggered before ready");
  }

  results = clock->update(1.5);

  for (uint16_t i = 0; i < 4; i++) {
    check(results[i] == false, "not triggered on first trigger");
  }

  results = clock->update(0);
  results = clock->update(0);
  results = clock->update(0);

  results = clock->update(1.5);
  check(results[0] == true, "first entry is triggered");
  check(results[1] == false, "second entry is not triggered");
  check(results[2] == false, "third entry is not triggered");
  check(results[3] == false, "fourth entry is not triggered");

  results = clock->update(0);
  check(results[0] == true, "first entry is still triggered");
  check(results[1] == false, "second entry is still not triggered");
  check(results[2] == false, "third entry is still not triggered");
  check(results[3] == false, "fourth entry is still not triggered");

  results = clock->update(0);
  check(results[0] == false, "first entry is now not triggered");
  check(results[1] == false, "second entry is still not triggered");
  check(results[2] == false, "third entry is still not triggered");
  check(results[3] == false, "fourth entry is still not triggered");

  results = clock->update(0);
  check(results[0] == false, "first entry is still not triggered");
  check(results[1] == false, "second entry is still not triggered");
  check(results[2] == false, "third entry is still not triggered");
  check(results[3] == false, "fourth entry is still not triggered");

  results = clock->update(1.5);
  check(results[0] == true, "first entry is triggered");
  check(results[1] == true, "second entry is triggered");
  check(results[2] == false, "third entry is not triggered");
  check(results[3] == false, "fourth entry is not triggered");

  clock->update(0);
  clock->update(0);
  clock->update(0);

  results = clock->update(1.5);
  check(results[0] == true, "first entry is triggered");
  check(results[1] == false, "second entry is not triggered");
  check(results[2] == true, "third entry is triggered");
  check(results[3] == false, "fourth entry is not triggered");

  clock->update(0);
  clock->update(0);
  clock->update(0);

  results = clock->update(1.5);
  check(results[0] == true, "first entry is triggered");
  check(results[1] == true, "second entry is triggered");
  check(results[2] == false, "third entry is not triggered");
  check(results[3] == true, "fourth entry is triggered");

  done();
}

uint8_t test_clock_reset ( ) {
  Clock *clock = new Clock(4, 1.5);
  bool *results;

  results = clock->update(0);

  for (uint16_t i = 0; i < 4; i++) {
    check(results[i] == false, "not triggered before ready");
  }

  results = clock->update(1.5);

  for (uint16_t i = 0; i < 4; i++) {
    check(results[i] == false, "not triggered on first trigger");
  }

  results = clock->update(0);
  results = clock->update(0);
  results = clock->update(0);

  results = clock->update(1.5);
  check(results[0] == true, "first entry is triggered");
  check(results[1] == false, "second entry is not triggered");
  check(results[2] == false, "third entry is not triggered");
  check(results[3] == false, "fourth entry is not triggered");

  clock->reset();

  results = clock->update(0);

  for (uint16_t i = 0; i < 4; i++) {
    check(results[i] == false, "not triggered before ready");
  }

  results = clock->update(1.5);

  for (uint16_t i = 0; i < 4; i++) {
    check(results[i] == false, "not triggered on first trigger");
  }

  done();
}
