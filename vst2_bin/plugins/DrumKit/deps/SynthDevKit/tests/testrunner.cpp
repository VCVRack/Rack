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

  test(test_trigger_update, "test trigger update");
  test(test_trigger_reset, "test trigger reset");

  test(test_clock_exception, "test clock exception");
  test(test_clock_update, "test clock update");

  cout << endl << "PASSED: " << test_passed << endl << "FAILED: " << test_failed << endl;

  return (test_failed > 0 ? 1 : 0);
}
