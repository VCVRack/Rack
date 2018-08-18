#include "testrunner.hpp"
#include "../src/EventEmitter.hpp"

using namespace SynthDevKit;

static int16_t emit_count = 0;
static float last_emit = 0.0f;

static void emit_once (int16_t event, float value) {
  emit_count++;
  last_emit = value;
}

static int16_t emit_value = 0;

static void emit_test (int16_t event, float value) {
  emit_value = event;
}

uint8_t test_event_emitter ( ) {
  EventEmitter *ee = new EventEmitter();

  check(ee != nullptr, "event emitter is instantiated");

  done();
}

uint8_t test_event_emitter_on ( ) {
  EventEmitter *ee = new EventEmitter();

  emit_count = 0;
  last_emit = 0.0f;

  ee->on(10, emit_once);
  ee->on(EVENT_FIRST, emit_test);

  ee->emit(10, 1.234f);

  check(emit_count == 1, "event emitter was run");
  check(last_emit == 1.234f, "the value was correct");

  check(emit_value == EVENT_FIRST, "first event triggered");

  ee->on(EVENT_EVEN, emit_test);
  ee->on(EVENT_ODD, emit_test);

  ee->emit(10, 2.234f);

  check(emit_count == 2, "event emitter was run again");
  check(last_emit == 2.234f, "the value was correct");

  check(emit_value == EVENT_EVEN, "even emitter was run");

  ee->emit(11, 2.234f);

  check(emit_value == EVENT_ODD, "odd emitter was run");

  done();
}

uint8_t test_event_emitter_clear ( ) {
  EventEmitter *ee = new EventEmitter();

  emit_count = 0;
  last_emit = 1.0f;

  ee->on(EVENT_CLEAR, emit_once);

  ee->clear();

  check(emit_count == 1, "event emitter was run");
  check(last_emit == 0.0, "the value was correct");

  ee->clear(true);

  check(emit_count == 1, "event emitter not run when fully cleared");

  done();
}

uint8_t test_event_emitter_listener ( ) {
  EventEmitter *ee = new EventEmitter();

  ee->on(10, emit_once);

  check(ee->listenerCount(10) == 1, "listener count is correct");

  ee->removeListener(10, emit_once);
  check(ee->listenerCount(10) == 0, "listener count is correct after removal");

  done();
}
