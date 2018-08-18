#include <iostream>
#include <string.h>
#include "testrunner.hpp"

using namespace std;

uint8_t spec = 0;
uint16_t test_passed = 0;
uint16_t test_failed = 0;

void _test_pass (const char *message) {
  if (spec) {
    cout << "  ✓ " << message << endl;
  } else {
    cout << ".";
  }
}

void _test_fail (const char *message, const char *file, uint16_t line) {
  if (spec) {
    cout << "  𝙭 " << message << " (" << file << ":" << line << ")" << endl;
  } else {
    cout << "𝙭";
  }
}

void _test_start (const char *name) {
  if (spec) {
    cout << endl << name << endl;
  }
}

int main (int argc, char **argv) {
  if (argc > 1) {
    if ((strcmp(argv[1], "--spec") == 0) || (strcmp(argv[1], "-s") == 0)) {
      spec = 1;
    }
  }

  test(test_trigger_update, "trigger update");
  test(test_trigger_reset, "trigger reset");

  test(test_clock_exception, "clock exception");
  test(test_clock_update, "clock update");

  test(test_primeclock_exception, "primeclock exception");
  test(test_primeclock_primes, "primeclock primes");
  test(test_primeclock_update, "primeclock update");

  test(test_bjorklund_exception, "bjorklund exception");
  test(test_bjorklund_update_exception, "bjorklund update exception");
  test(test_bjorklund_update, "bjorklund update");
  test(test_bjorklund_reset, "bjorklund reset");

  test(test_distributedclock_master, "distributed clock master");
  test(test_distributedclock_servant, "distributed clock servant");

  test(test_fibonacciclock_exception, "fibonacciclock exception");
  test(test_fibonacciclock_fibonaccis, "fibonacciclock fibonaccis");
  test(test_fibonacciclock_update, "fibonacciclock update");

  test(test_event_emitter, "event emitter");
  test(test_event_emitter_on, "event emitter on");
  test(test_event_emitter_clear, "event emitter clear");
  test(test_event_emitter_listener, "event emitter listeners");

  test(test_cv_event, "cv event");
  test(test_cv_event_update, "cv event update");
  test(test_cv_event_resets, "cv event resets");

  cout << endl << "PASSED: " << test_passed << endl << "FAILED: " << test_failed << endl;

  return (test_failed > 0 ? 1 : 0);
}
