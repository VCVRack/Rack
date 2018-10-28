
#include "asserts.h"
#include "GateTrigger.h"
#include "SchmidtTrigger.h"
#include "TriggerOutput.h"

#include <stdio.h>

static void sc0()
{
    SchmidtTrigger sc(-10, 10);
    bool b = sc.go(-20);
    assert(!b);
}

static void sc1()
{
    SchmidtTrigger sc(-10, 10);
    bool b = sc.go(20);
    assert(b);
}

static void sc2()
{
    SchmidtTrigger sc(-10, 10);
    bool b = sc.go(20);
    assert(b);
    b = sc.go(9);
    assert(b);
    b = sc.go(-9);
    assert(b);
    b = sc.go(-11);
    assert(!b);

    b = sc.go(-999);
    assert(!b);

    b = sc.go(9);
    assert(!b);
    b = sc.go(11);
    assert(b);
}

// check defaults for schmidt
static void sc3()
{
    SchmidtTrigger sc;
    bool b = sc.go(cGateHi + .1f);
    assert(b);
    b = sc.go(cGateLow + .1f);
    assert(b);
    b = sc.go(cGateLow - .1f);
    assert(!b);

    b = sc.go(cGateHi - .1f);
    assert(!b);
    b = sc.go(cGateHi + .1f);
    assert(b);
}

// check that threshold accessors are sane
void g_1()
{
    GateTrigger g(true);
    assert(g.thlo() > 0);
    assert(g.thhi() > g.thlo());

    assert(g.thhi() < 10.f);
    assert(g.thhi() > 1.f);
}

void testAfterReset(GateTrigger& g)
{
    g.go(10.f);				// right after "reset", start with gate
    assert(!g.gate());
    assert(!g.trigger());

    g.go(0.f);
    assert(!g.gate());
    assert(!g.trigger());

    g.go(10.f);
    assert(g.gate());
    assert(g.trigger());
}

void grst1()
{
    GateTrigger g(true);
    testAfterReset(g);

    g.go(10.f);
    g.reset();
    testAfterReset(g);
}

void to0()
{
    TriggerOutput t;

   // const int ghigh = DACVoltage::xcodeForGateHi();
    t.go(false);
    assert(t.get() == 0);

    t.go(true);
    assert(t.get() == cGateOutHi);

    int duration = 0;
    for (int i = 0; i < 100000; ++i) {
        t.go(true);
        if (t.get() == 0)
            break;
        ++duration;
    }

    const int micros = (duration * 1000 * 1000) / 44100;        // TODO: rationalize
    //printf("duration = %d micros = %d\n", duration, micros);
    assert(micros > 4900);
    assert(micros < 5100);
}

// test that we work immediately on startup - no reset delay
void to1()
{
    TriggerOutput t;
    t.go(true);
    assertEQ(t.get(), cGateOutHi);
}




void testGateTrigger()
{
    sc0();
    sc1();
    sc2();
    sc3();
    g_1();
    grst1();
    to0();
    to1();
}
