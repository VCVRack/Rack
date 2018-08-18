#include "arptest.hpp"

#include "../src/controller/ClockDivider.hpp"

uint8_t test_clock_divider_initial_clock() {
  ClockDividerModule *m = new ClockDividerModule();

  check(m, "clock divider is instantiated");

  m->inputs[0].value = 1.7f;
  m->step();

  check(m->outputs[0].value == 0.0f, "output 1 is not active");
  check(m->outputs[1].value == 0.0f, "output 2 is not active");
  check(m->outputs[2].value == 0.0f, "output 3 is not active");
  check(m->outputs[3].value == 0.0f, "output 4 is not active");
  check(m->outputs[4].value == 0.0f, "output 5 is not active");
  check(m->outputs[5].value == 0.0f, "output 6 is not active");
  check(m->outputs[6].value == 0.0f, "output 7 is not active");
  check(m->outputs[7].value == 0.0f, "output 8 is not active");

  done();
}

uint8_t test_clock_divider_first_clock() {
  ClockDividerModule *m = new ClockDividerModule();

  check(m, "clock divider is instantiated");

  // initial
  m->inputs[0].value = 1.7f;
  m->step();

  for (uint8_t i = 0; i < 1; i++) {
    m->inputs[0].value = 0.0f;
    m->step();
    m->inputs[0].value = 1.7f;
    m->step();
  }

  check(m->outputs[0].value == 1.7f, "output 1 is active");
  check(m->outputs[1].value == 0.0f, "output 2 is not active");
  check(m->outputs[2].value == 0.0f, "output 3 is not active");
  check(m->outputs[3].value == 0.0f, "output 4 is not active");
  check(m->outputs[4].value == 0.0f, "output 5 is not active");
  check(m->outputs[5].value == 0.0f, "output 6 is not active");
  check(m->outputs[6].value == 0.0f, "output 7 is not active");
  check(m->outputs[7].value == 0.0f, "output 8 is not active");

  done();
}

uint8_t test_clock_divider_second_clock() {
  ClockDividerModule *m = new ClockDividerModule();

  check(m, "clock divider is instantiated");

  // initial
  m->inputs[0].value = 1.7f;
  m->step();

  for (uint8_t i = 0; i < 2; i++) {
    m->inputs[0].value = 0.0f;
    m->step();
    m->inputs[0].value = 1.7f;
    m->step();
  }

  check(m->outputs[0].value == 1.7f, "output 1 is active");
  check(m->outputs[1].value == 1.7f, "output 2 is active");
  check(m->outputs[2].value == 0.0f, "output 3 is not active");
  check(m->outputs[3].value == 0.0f, "output 4 is not active");
  check(m->outputs[4].value == 0.0f, "output 5 is not active");
  check(m->outputs[5].value == 0.0f, "output 6 is not active");
  check(m->outputs[6].value == 0.0f, "output 7 is not active");
  check(m->outputs[7].value == 0.0f, "output 8 is not active");

  done();
}

uint8_t test_clock_divider_third_clock() {
  ClockDividerModule *m = new ClockDividerModule();

  check(m, "clock divider is instantiated");

  // initial
  m->inputs[0].value = 1.7f;
  m->step();

  for (uint8_t i = 0; i < 3; i++) {
    m->inputs[0].value = 0.0f;
    m->step();
    m->inputs[0].value = 1.7f;
    m->step();
  }

  check(m->outputs[0].value == 1.7f, "output 1 is active");
  check(m->outputs[1].value == 0.0f, "output 2 is not active");
  check(m->outputs[2].value == 1.7f, "output 3 is active");
  check(m->outputs[3].value == 0.0f, "output 4 is not active");
  check(m->outputs[4].value == 0.0f, "output 5 is not active");
  check(m->outputs[5].value == 0.0f, "output 6 is not active");
  check(m->outputs[6].value == 0.0f, "output 7 is not active");
  check(m->outputs[7].value == 0.0f, "output 8 is not active");

  done();
}

uint8_t test_clock_divider_fourth_clock() {
  ClockDividerModule *m = new ClockDividerModule();

  check(m, "clock divider is instantiated");

  // initial
  m->inputs[0].value = 1.7f;
  m->step();

  for (uint8_t i = 0; i < 4; i++) {
    m->inputs[0].value = 0.0f;
    m->step();
    m->inputs[0].value = 1.7f;
    m->step();
  }

  check(m->outputs[0].value == 1.7f, "output 1 is active");
  check(m->outputs[1].value == 1.7f, "output 2 is active");
  check(m->outputs[2].value == 0.0f, "output 3 is not active");
  check(m->outputs[3].value == 1.7f, "output 4 is active");
  check(m->outputs[4].value == 0.0f, "output 5 is not active");
  check(m->outputs[5].value == 0.0f, "output 6 is not active");
  check(m->outputs[6].value == 0.0f, "output 7 is not active");
  check(m->outputs[7].value == 0.0f, "output 8 is not active");

  done();
}

uint8_t test_clock_divider_fifth_clock() {
  ClockDividerModule *m = new ClockDividerModule();

  check(m, "clock divider is instantiated");

  // initial
  m->inputs[0].value = 1.7f;
  m->step();

  for (uint8_t i = 0; i < 5; i++) {
    m->inputs[0].value = 0.0f;
    m->step();
    m->inputs[0].value = 1.7f;
    m->step();
  }

  check(m->outputs[0].value == 1.7f, "output 1 is active");
  check(m->outputs[1].value == 0.0f, "output 2 is not active");
  check(m->outputs[2].value == 0.0f, "output 3 is not active");
  check(m->outputs[3].value == 0.0f, "output 4 is not active");
  check(m->outputs[4].value == 1.7f, "output 5 is active");
  check(m->outputs[5].value == 0.0f, "output 6 is not active");
  check(m->outputs[6].value == 0.0f, "output 7 is not active");
  check(m->outputs[7].value == 0.0f, "output 8 is not active");

  done();
}

uint8_t test_clock_divider_sixth_clock() {
  ClockDividerModule *m = new ClockDividerModule();

  check(m, "clock divider is instantiated");

  // initial
  m->inputs[0].value = 1.7f;
  m->step();

  for (uint8_t i = 0; i < 6; i++) {
    m->inputs[0].value = 0.0f;
    m->step();
    m->inputs[0].value = 1.7f;
    m->step();
  }

  check(m->outputs[0].value == 1.7f, "output 1 is active");
  check(m->outputs[1].value == 1.7f, "output 2 is active");
  check(m->outputs[2].value == 1.7f, "output 3 is active");
  check(m->outputs[3].value == 0.0f, "output 4 is not active");
  check(m->outputs[4].value == 0.0f, "output 5 is not active");
  check(m->outputs[5].value == 1.7f, "output 6 is active");
  check(m->outputs[6].value == 0.0f, "output 7 is not active");
  check(m->outputs[7].value == 0.0f, "output 8 is not active");

  done();
}

uint8_t test_clock_divider_seventh_clock() {
  ClockDividerModule *m = new ClockDividerModule();

  check(m, "clock divider is instantiated");

  // initial
  m->inputs[0].value = 1.7f;
  m->step();

  for (uint8_t i = 0; i < 7; i++) {
    m->inputs[0].value = 0.0f;
    m->step();
    m->inputs[0].value = 1.7f;
    m->step();
  }

  check(m->outputs[0].value == 1.7f, "output 1 is active");
  check(m->outputs[1].value == 0.0f, "output 2 is not active");
  check(m->outputs[2].value == 0.0f, "output 3 is not active");
  check(m->outputs[3].value == 0.0f, "output 4 is not active");
  check(m->outputs[4].value == 0.0f, "output 5 is not active");
  check(m->outputs[5].value == 0.0f, "output 6 is not active");
  check(m->outputs[6].value == 1.7f, "output 7 is active");
  check(m->outputs[7].value == 0.0f, "output 8 is not active");

  done();
}

uint8_t test_clock_divider_eighth_clock() {
  ClockDividerModule *m = new ClockDividerModule();

  check(m, "clock divider is instantiated");

  // initial
  m->inputs[0].value = 1.7f;
  m->step();

  for (uint8_t i = 0; i < 8; i++) {
    m->inputs[0].value = 0.0f;
    m->step();
    m->inputs[0].value = 1.7f;
    m->step();
  }

  check(m->outputs[0].value == 1.7f, "output 1 is active");
  check(m->outputs[1].value == 1.7f, "output 2 is active");
  check(m->outputs[2].value == 0.0f, "output 3 is not active");
  check(m->outputs[3].value == 1.7f, "output 4 is active");
  check(m->outputs[4].value == 0.0f, "output 5 is not active");
  check(m->outputs[5].value == 0.0f, "output 6 is not active");
  check(m->outputs[6].value == 0.0f, "output 7 is not active");
  check(m->outputs[7].value == 1.7f, "output 8 is active");

  done();
}

uint8_t test_clock_divider_ninth_clock() {
  ClockDividerModule *m = new ClockDividerModule();

  check(m, "clock divider is instantiated");

  // initial
  m->inputs[0].value = 1.7f;
  m->step();

  for (uint8_t i = 0; i < 9; i++) {
    m->inputs[0].value = 0.0f;
    m->step();
    m->inputs[0].value = 1.7f;
    m->step();
  }

  check(m->outputs[0].value == 1.7f, "output 1 is active");
  check(m->outputs[1].value == 0.0f, "output 2 is not active");
  check(m->outputs[2].value == 0.0f, "output 3 is not active");
  check(m->outputs[3].value == 0.0f, "output 4 is not active");
  check(m->outputs[4].value == 0.0f, "output 5 is not active");
  check(m->outputs[5].value == 0.0f, "output 6 is not active");
  check(m->outputs[6].value == 0.0f, "output 7 is not active");
  check(m->outputs[7].value == 0.0f, "output 8 is not active");

  done();
}

uint8_t test_clock_divider_reset() {
  ClockDividerModule *m = new ClockDividerModule();

  check(m, "clock divider is instantiated");

  // initial
  m->inputs[0].value = 1.7f;
  m->step();

  for (uint8_t i = 0; i < 2; i++) {
    m->inputs[0].value = 0.0f;
    m->step();
    m->inputs[0].value = 1.7f;
    m->step();
  }

  check(m->outputs[0].value == 1.7f, "output 1 is active");
  check(m->outputs[1].value == 1.7f, "output 2 is active");
  check(m->outputs[2].value == 0.0f, "output 3 is not active");
  check(m->outputs[3].value == 0.0f, "output 4 is not active");
  check(m->outputs[4].value == 0.0f, "output 5 is not active");
  check(m->outputs[5].value == 0.0f, "output 6 is not active");
  check(m->outputs[6].value == 0.0f, "output 7 is not active");
  check(m->outputs[7].value == 0.0f, "output 8 is not active");

  // trigger a reset
  m->inputs[1].value = 1.7f;
  m->step();

  for (uint8_t i = 0; i < 2; i++) {
    m->inputs[0].value = 0.0f;
    m->step();
    m->inputs[0].value = 1.7f;
    m->step();
  }

  check(m->outputs[0].value == 1.7f, "output 1 is active after reset");
  check(m->outputs[1].value == 1.7f, "output 2 is active after reset");
  check(m->outputs[2].value == 0.0f, "output 3 is not active after reset");
  check(m->outputs[3].value == 0.0f, "output 4 is not active after reset");
  check(m->outputs[4].value == 0.0f, "output 5 is not active after reset");
  check(m->outputs[5].value == 0.0f, "output 6 is not active after reset");
  check(m->outputs[6].value == 0.0f, "output 7 is not active after reset");
  check(m->outputs[7].value == 0.0f, "output 8 is not active after reset");

  done();
}
