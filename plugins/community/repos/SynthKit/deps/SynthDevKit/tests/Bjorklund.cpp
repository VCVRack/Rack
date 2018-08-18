#include "testrunner.hpp"
#include "../src/Bjorklund.hpp"

using namespace SynthDevKit;

uint8_t test_bjorklund_exception ( ) {
  bool has_exception = 0;
  Bjorklund *b = {0};

  try {
    b = new Bjorklund(MAX_SLOTS + 1);
  } catch (int err) {
    has_exception = true;
  }

  check(!b, "clock is null");
  check(has_exception, "exception is thrown");

  done();
}

uint8_t test_bjorklund_update_exception ( ) {
  bool has_exception = 0;
  Bjorklund *b;

  b = new Bjorklund(MAX_SLOTS);

  try {
    b->update(3, MAX_SLOTS + 1);
  } catch (int err) {
    has_exception = true;
  }

  check(has_exception, "exception is thrown");

  done();
}

uint8_t test_bjorklund_update ( ) {
  Bjorklund *b = new Bjorklund(13);

  b->update(5, 8);

  check(b->stepValue() == 1, "first value is 1");
  check(b->stepValue() == 0, "second value is 0");
  check(b->stepValue() == 1, "third value is 1");
  check(b->stepValue() == 1, "fourth value is 1");
  check(b->stepValue() == 0, "fifth value is 0");
  check(b->stepValue() == 1, "sixth value is 1");
  check(b->stepValue() == 1, "seventh value is 1");
  check(b->stepValue() == 0, "eighth value is 0");

  done();
}

uint8_t test_bjorklund_reset ( ) {
  Bjorklund *b = new Bjorklund(13);

  b->update(5, 8);

  check(b->stepValue() == 1, "first value is 1");
  b->reset();
  check(b->stepValue() == 1, "first value after reset is 1");

  done();
}
