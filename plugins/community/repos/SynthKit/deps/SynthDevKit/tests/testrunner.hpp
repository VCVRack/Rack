#pragma once

#include <stdint.h>

extern uint8_t spec;
extern uint16_t test_passed;
extern uint16_t test_failed;

void _test_fail (const char *, const char *, uint16_t);
void _test_pass (const char *);
void _test_start (const char *);

/* Successfull end of the test case */
#define done() return 0

/* Check single condition */
#define check(cond, message) do { if (!(cond)) { _test_fail(message, __FILE__, __LINE__); test_failed++; } else { _test_pass(message); test_passed++; } } while (0)

/* Test runner */
#define test(func, name) do { _test_start(name); func(); } while(0)

uint8_t test_trigger_update ( );
uint8_t test_trigger_reset ( );
uint8_t test_clock_exception ( );
uint8_t test_clock_update ( );
uint8_t test_clock_reset ( );

uint8_t test_primeclock_exception ( );
uint8_t test_primeclock_primes ( );
uint8_t test_primeclock_update ( );

uint8_t test_bjorklund_exception ( );
uint8_t test_bjorklund_update_exception ( );
uint8_t test_bjorklund_update ( );
uint8_t test_bjorklund_reset ( );

uint8_t test_distributedclock_master ( );
uint8_t test_distributedclock_servant ( );

uint8_t test_fibonacciclock_exception ( );
uint8_t test_fibonacciclock_fibonaccis ( );
uint8_t test_fibonacciclock_update ( );

uint8_t test_event_emitter ( );
uint8_t test_event_emitter_on ( );
uint8_t test_event_emitter_clear ( );
uint8_t test_event_emitter_listener ( );

uint8_t test_cv_event ( );
uint8_t test_cv_event_update ( );
uint8_t test_cv_event_resets ( );
