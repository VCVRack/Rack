#include "testrunner.hpp"
#include "../src/CVEvent.hpp"

using namespace SynthDevKit;

static int16_t emit_count = 0;
static float last_emit = 0.0f;

static void emit_once (int16_t event, float value) {
  emit_count++;
  last_emit = value;
}

uint8_t test_cv_event ( ) {
  CVEvent *ee = new CVEvent(1.1f, 4);

  check(ee != nullptr, "cv event is instantiated");

  done();
}

uint8_t test_cv_event_update ( ) {
  CVEvent *ee = new CVEvent(1.1f, 4);

  emit_count = 0;
  last_emit = 0.0f;

  ee->on(1, emit_once);

  check(ee->newTrigger() == false, "no trigger before update");

  ee->update(0.1);
  check(ee->newTrigger() == false, "no trigger if low");
  check(ee->triggerTotal() == 0, "trigger total is correct");
  check(emit_count == 0, "listener has not been called");

  ee->update(1.5f);
  check(ee->newTrigger() == true, "trigger is correct after going high");
  check(ee->triggerTotal() == 1, "trigger total is correct");
  check(emit_count == 1, "the listener has been called");

  ee->update(0.1);
  check(ee->newTrigger() == false, "no trigger if low");
  check(ee->triggerTotal() == 1, "trigger total is correct");
  check(emit_count == 1, "listener has not been called");

  ee->update(0.1);
  check(ee->newTrigger() == false, "trigger is correct after staying low");
  check(ee->triggerTotal() == 1, "trigger total is still correct");
  check(emit_count == 1, "the listener has not been called again");

  done();
}

uint8_t test_cv_event_resets ( ) {
  CVEvent *ee = new CVEvent(1.1f, 2);

  ee->update(0.1);
  check(ee->triggerTotal() == 0, "trigger total is correct at 0");

  ee->update(1.5);
  check(ee->triggerTotal() == 1, "trigger total is correct at 1");

  ee->update(0.1);
  check(ee->triggerTotal() == 1, "trigger total is still correct");

  ee->update(1.5);
  check(ee->triggerTotal() == 2, "trigger total is correct at 2");

  ee->update(0.1);

  ee->update(1.5);
  check(ee->triggerTotal() == 1, "trigger total is correct and back to 1");

  done();
}
