#include "testrunner.hpp"
#include "../src/PrimeClock.hpp"

using namespace SynthDevKit;

uint8_t test_primeclock_exception ( ) {
  bool has_exception = 0;
  PrimeClock *clock = {0};

  try {
    clock = new PrimeClock(CLOCK_LIMIT + 1, 1.5);
  } catch (int err) {
    has_exception = true;
  }

  check(!clock, "clock is null");
  check(has_exception, "exception is thrown");

  done();
}

uint8_t test_primeclock_primes ( ) {
  PrimeClock *clock = new PrimeClock(8, 1.5);
  std::vector<uint16_t>::iterator it;
  uint16_t total;
  for (it = clock->primes.begin(), total = 0; it != clock->primes.end(); it++, total++) {
    if (total == 0) {
      check(*it == 2, "first prime is correct");
    } else if (total == 1) {
      check(*it == 3, "second prime is correct");
    } else if (total == 2) {
      check(*it == 5, "third prime is correct");
    } else if (total == 3) {
      check(*it == 7, "fourth prime is correct");
    } else if (total == 4) {
      check(*it == 11, "fifth prime is correct");
    } else if (total == 5) {
      check(*it == 13, "sixth prime is correct");
    } else if (total == 6) {
      check(*it == 17, "seventh prime is correct");
    } else if (total == 7) {
      check(*it == 19, "eighth prime is correct");
    }
  }

  check(total == 16, "the count is correct");

  done();
}

uint8_t test_primeclock_update ( ) {
  PrimeClock *clock = new PrimeClock(4, 1.5);
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

  // 1
  results = clock->update(1.5);
  check(results[0] == false, "first entry not is triggered");
  check(results[1] == false, "second entry is not triggered");
  check(results[2] == false, "third entry is not triggered");
  check(results[3] == false, "fourth entry is not triggered");

  results = clock->update(0);
  check(results[0] == false, "first entry is still not triggered");
  check(results[1] == false, "second entry is still not triggered");
  check(results[2] == false, "third entry is still not triggered");
  check(results[3] == false, "fourth entry is still not triggered");

  // 2
  results = clock->update(1.5);
  check(results[0] == true, "first entry is triggered");
  check(results[1] == false, "second entry is still not triggered");
  check(results[2] == false, "third entry is still not triggered");
  check(results[3] == false, "fourth entry is still not triggered");

  clock->update(0);

  // 3
  results = clock->update(1.5);
  check(results[0] == false, "first entry is not triggered");
  check(results[1] == true, "second entry is triggered");
  check(results[2] == false, "third entry is not triggered");
  check(results[3] == false, "fourth entry is not triggered");

  clock->update(0);

  // 4
  results = clock->update(1.5);
  check(results[0] == false, "first entry not is triggered");
  check(results[1] == false, "second entry is not triggered");
  check(results[2] == false, "third entry is not triggered");
  check(results[3] == false, "fourth entry is not triggered");

  clock->update(0);

  // 5
  results = clock->update(1.5);
  check(results[0] == false, "first entry is not triggered");
  check(results[1] == false, "second entry is not triggered");
  check(results[2] == true, "third entry is triggered");
  check(results[3] == false, "fourth entry is not triggered");

  clock->update(0);

  // 6
  results = clock->update(1.5);
  check(results[0] == false, "first entry is not triggered");
  check(results[1] == false, "second entry is not triggered");
  check(results[2] == false, "third entry is not triggered");
  check(results[3] == false, "fourth entry is not triggered");

  clock->update(0);

  // 7
  results = clock->update(1.5);
  check(results[0] == false, "first entry is not triggered");
  check(results[1] == false, "second entry is not triggered");
  check(results[2] == false, "third entry is not triggered");
  check(results[3] == true, "fourth entry is triggered");

  done();
}
