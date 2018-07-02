
#include "ColoredNoise.h"
#include "TestComposite.h"
#include "asserts.h"

extern void testFinalLeaks();

using Noise = ColoredNoise<TestComposite>;

// bring it up and process a bit.
static void test0()
{
    assertEQ(ThreadServer::_count, 0);
    assertEQ(FFTDataCpx::_count, 0);
    {
        Noise cn;
        cn.init();
        // calling step should get client to request an FFT frame

        while (cn._msgCount() < 1) {
            cn.step();

        }
    }
    assertEQ(ThreadServer::_count, 0);
    assertEQ(FFTDataCpx::_count, 0);
}

// porcess until audio comes out
static void test1()
{
    Noise cn;
    cn.init();
    // calling step should get client to request an FFT frame
    int num = 0;
    bool started = false;
    for (bool done = false; !done; ) {
        cn.step();
        const float output = cn.outputs[Noise::AUDIO_OUTPUT].value;
        if (output > .1) {;
            started = true;
        }
        assert(output < 10);
        assert(output > -10);
        if (started) {
            num++;
            if (num > 100000) {
                done = true;
            }
        }
    }
}


// try to request a few different kinds of noise
static void test2()
{
    Noise cn;
    cn.init();
    while (cn._msgCount() < 1) {
        cn.step();
    }
    cn.params[Noise::SLOPE_PARAM].value = -6;
    while (cn._msgCount() < 2) {
        cn.step();
    }
    cn.params[Noise::SLOPE_PARAM].value = 3;
    while (cn._msgCount() < 3) {
        cn.step();
    }
    cn.params[Noise::SLOPE_PARAM].value = 2;
    while (cn._msgCount() < 4) {
        cn.step();
    }
    cn.params[Noise::SLOPE_PARAM].value = 2.3f;
    while (cn._msgCount() < 5) {
        cn.step();
    }
    cn.params[Noise::SLOPE_PARAM].value = 2.5f;
    while (cn._msgCount() < 6) {
        cn.step();
    }
      cn.params[Noise::SLOPE_PARAM].value = -1.2f;
    while (cn._msgCount() < 7) {
        cn.step();
    }
}

void testColoredNoise()
{

    test0();
    test1();
    test2();
    testFinalLeaks();
}