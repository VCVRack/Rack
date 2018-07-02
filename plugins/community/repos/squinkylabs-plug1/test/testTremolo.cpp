
#include "asserts.h"
#include "Tremolo.h"
#include "TestComposite.h"

using Trem = Tremolo<TestComposite>;

static void test0()
{
    Trem t;
    t.setSampleRate(44100);
    t.init();
    assertEQ(t.outputs[Trem::SAW_OUTPUT].value, 0);
    assertEQ(t.outputs[Trem::AUDIO_OUTPUT].value, 0);
    t.step();
}

static void test1Sub(float skew)
{
    Trem t;
    t.setSampleRate(44100);
    t.init();

    t.params[Trem::CLOCK_MULT_PARAM].value = 4;        // 4 is free run for Trem
    t.params[Trem::LFO_RATE_PARAM].value = 5;       // max speed
    t.params[Trem::LFO_SKEW_PARAM].value = skew;
    float max = -100;
    float min = 100;
    for (int i = 0; i < 5000; ++i) {
        t.step();
        const float x = t.outputs[Trem::SAW_OUTPUT].value;
        max = std::max(x, max);
        min = std::min(x, min);
    }
    assertClose(max, .5f, .001);
    assertClose(min, -.5f, .001);
}

static void test1()
{
    test1Sub(0);
    test1Sub(5);
    test1Sub(-5);
}

void testTremolo()
{
    test0();
    test1();
}