
#include <assert.h>
#include "asserts.h"

#include "ClockMult.h"
#include "TestComposite.h"
#include "Tremolo.h"


/**
 * During training, we should get no output.
 * On first ref-clock after training, should go high
 */
static void test0()
{
    ClockMult cm;
    cm.setMultiplier(1);
    const int period = 4;

   // printf("test0, 0\n");
    cm.refClock();  // give it an external clock

  //  printf("test0, 1\n");
    // train with 4 ref clocks()
    for (int i = 0; i < period; ++i) {
        cm.sampleClock();
        assertEQ(cm.getSaw(), 0);
        assertEQ(cm.getMultipliedClock(), false);
       // printf("test0, 2\n");

    }

    // send ref-clock now to set period at 4 and start clocking
    cm.refClock();
    assertEQ(cm.getSaw(), 0);
    assertEQ(cm.getMultipliedClock(), true);
    assertClose(cm._getFreq(), .25f, .000001);
}


/**
 * Test synched saw output
 */
static void test1()
{
    ClockMult cm;
    cm.setMultiplier(1);
    const int period = 4;

  //  printf("test0, 0\n");
    cm.refClock();  // give it an external clock

   // printf("test0, 1\n");
    // train with 4 ref clocks()
    for (int i = 0; i < period; ++i) {
        cm.sampleClock();
        assertEQ(cm.getSaw(), 0);
        assertEQ(cm.getMultipliedClock(), false);
       // printf("test0, 2\n");

    }

    // send ref-clock now to set period at 4 and start clocking
    cm.refClock();
    assertEQ(cm.getSaw(), 0);
    assertEQ(cm.getMultipliedClock(), true);
    assertClose(cm._getFreq(), .25f, .000001);

    for (int i = 0; i < period - 1; ++i) {
        cm.sampleClock();
    //    printf("in loop, i=%d, saw=%f\n", i, cm.getSaw());
        assertEQ(cm.getSaw(), .25 * (i + 1));
        assertEQ(cm.getMultipliedClock(), true);
    }
}

/**
 * Test free running
 */
static void test2()
{
    ClockMult cm;
    cm.setMultiplier(0);
    cm.setFreeRunFreq(.1f);

    assertEQ(cm.getSaw(), 0);
    for (int i = 0; i < 9; ++i) {
        cm.sampleClock();
        assertClose(cm.getSaw(), (i + 1) * .1f, .0001);
    }
    cm.sampleClock();
    assertClose(cm.getSaw(), 0, .0001);
    for (int i = 0; i < 9; ++i) {
        cm.sampleClock();
        assertClose(cm.getSaw(), (i + 1) * .1f, .0001);
    }
}


/**
* Test free running, no interference from ref clock
*/
static void test4()
{
    ClockMult cm;
    cm.setMultiplier(0);
    cm.setFreeRunFreq(.1f);

    assertEQ(cm.getSaw(), 0);
    for (int i = 0; i < 9; ++i) {
        cm.sampleClock();
        cm.refClock();
        assertClose(cm.getSaw(), (i + 1) * .1f, .0001);
    }
    cm.sampleClock();
    assertClose(cm.getSaw(), 0, .0001);
    for (int i = 0; i < 9; ++i) {
        cm.refClock();
        cm.sampleClock();
        assertClose(cm.getSaw(), (i + 1) * .1f, .0001);
    }
}
/**
* Test synched saw output long term, no jitter
*/
static void test3(int mult)
{

    ClockMult cm;
    cm.setMultiplier(mult);

    // ref clock period.
    const int period = 4 * mult;


    cm.refClock();  // give it an external clock

     // train with period of ref clocks()
    for (int i = 0; i < period; ++i) {
        cm.sampleClock();
        assertEQ(cm.getSaw(), 0);
        assertEQ(cm.getMultipliedClock(), false);
    }

    for (int j = 0; j < 2; ++j) {
    // send ref-clock now to set period at 4 and start clocking
        cm.refClock();

        assertEQ(cm.getSaw(), 0);

        assertEQ(cm.getMultipliedClock(), true);
        assertClose(cm._getFreq(), .25f, .000001);

        for (int i = 0; i < period; ++i) {
            cm.sampleClock();
            //printf("in loop, i=%d saw = %f\n", i, cm.getSaw());
            float expectedSaw = .25f * (i + 1);
            while (expectedSaw >= 1) {
                expectedSaw -= 1.f;
            }
            assertClose(cm.getSaw(), expectedSaw, .0001);

            // clock out not working yet
           // assertEQ(cm.getMultipliedClock(), true);
        }
    }
}

void testClockMult()
{
    test0();
    test1();
    test2();
    test4();
    test3(1);
    test3(2);
    test3(3);
    test3(4);

}