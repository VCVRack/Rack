

#include "FractionalDelay.h"
#include "asserts.h"


// test that we can set up and get zero output
static void test0()
{
    FractionalDelay f(100);
    f.setDelay(50);
    float x = f.run(1);
    assertEQ(x, 0);
}

// test that the output is perhaps a delayed version of the input
static void test1()
{
    printf("test1\n\n");
    FractionalDelay f(100);
    f.setDelay(10);
    float x = 0;
    for (int i = 0; i < 20; ++i) {
        x = f.run(1);
    }
    assertEQ(x, 1);
}

static void testTime(float delayTime, float expectedValue, float tolerance)
{
    int iTime = int(delayTime);
    if ((delayTime - iTime) > .0000000001) {
        iTime++;
    }

    FractionalDelay f(100);
    f.setDelay(delayTime);
    float x = 0;
    float lastX = 0;
    for (int i = 0; i < 20; ++i) {
        x = f.run(1);
       // printf("in loop i=%d, x=%f\n", i, x);
        if (x > .99f) {
            assertEQ(i, iTime);    // delay amount should be >= 10 < 11;
            assertClose(lastX, expectedValue, tolerance);
            return;
        }

        lastX = x;
    }
    assert(false);      // we should have exited already
}

static void test2()
{
    testTime(10.0f, 0, .0001f);
}


static void test3()
{
    testTime(10.5f, 0.5f, .0001f);
}

static void test4()
{
    testTime(10.25f, 0.75f, .1f);
}


static void test5()
{
    // MAKE THIS WORK
 //   testTime(10.05f, 0, .1f);
}


static void test6()
{
    testTime(10.75f, .25, .05f);
}

static void test10()
{
    RecirculatingFractionalDelay d(1000);
    d.setDelay(100);
    assertEQ(d.run(0), 0);
}



static void test11()
{
    RecirculatingFractionalDelay d(1000);
    d.setDelay(100);
    for (int i = 0; ; ++i) {
        assertLT(i, 110);           // too far with no sound
        float x = d.run(1);
        if (x > .5) {
            return;                 //we found sound!
        }

    }
}

static void testRecirc(float delayTime, float feedback, float expectedDuration, bool expectDurationLonger)
{
    RecirculatingFractionalDelay d(1000);
    d.setDelay(delayTime);
    d.setFeedback(feedback);

    const int maxDur = 10000;
    assert(expectedDuration < maxDur);

    // feed a blast into it
    for (int i = 0; i < 10; ++i) {
        d.run(1);
    }

    int lastSound = 0;
    for (int i = 0; ; ++i) {
        if (i > maxDur) {
            // we have hit end of test
            // If we expect infinit, that's ok
            if (!expectDurationLonger) {
                assertLT(lastSound, expectedDuration);
            } else {
                assertGT(lastSound, expectedDuration);
            }
            break;
        }
        float x = d.run(0);
        if (x > .1) {
            lastSound = i;
        }
        if (lastSound > expectedDuration && !expectDurationLonger) {
            assert(false);
        }

    }
}

static void test12()
{
    testRecirc(20, 0, 22, false);
}


static void test13()
{
    testRecirc(20, .9f, 100, true);
}

void testDelay()
{
    test0();
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();

    test10();
    test11();
    test12();

    test13();
}