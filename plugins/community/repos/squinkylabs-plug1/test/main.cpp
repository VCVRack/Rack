/**
  * Unit test entry point
  */

#include <stdio.h>
#include <assert.h>
#include <string>

extern void testBiquad();
extern void testTestSignal();
extern void testSaw();
extern void testLookupTable();
extern void testSinOscillator();
extern void testHilbert();
extern void testAudioMath();
extern void perfTest();
extern void testFrequencyShifter();
extern void testStateVariable();
extern void testVocalAnimator();
extern void testObjectCache();
extern void testThread(bool exended);
extern void testFFT();
extern void testRingBuffer();
extern void testManagedPool();
extern void testColoredNoise();
extern void testFFTCrossFader();
extern void testFinalLeaks();
extern void testClockMult();
extern void testTremolo();
extern void testGateTrigger();

int main(int argc, char ** argv)
{
    bool runPerf = false;
    bool extended = false;
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--ext") {
            extended = true;
        }
    }
#ifdef _PERF
    runPerf = true;
#ifndef NDEBUG
#error asserts should be off for perf test
#endif
#endif
    // While this code may work in 32 bit applications, it's not tested for that.
    // Want to be sure we are testing the case we care about.
    assert(sizeof(size_t) == 8);

    testAudioMath();
    testRingBuffer();
    testGateTrigger();
    testManagedPool();
    testLookupTable();
    testObjectCache();
    testTestSignal();
    testBiquad();
    testSaw();
    testClockMult();

    testSinOscillator();
    testHilbert();
    testStateVariable();

    testFFT();
    testFFTCrossFader();
    testThread(extended);

    // after testing all the components, test composites.
    testTremolo();
    testColoredNoise();
    testFrequencyShifter();
    testVocalAnimator();

    if (runPerf) {
        perfTest();
    }

    testFinalLeaks();

    // When we run inside Visual Studio, don't exit debugger immediately
#if defined(_MSC_VER)
    printf("Test passed. Press any key to continue...\n"); fflush(stdout);
    getchar();
#else
    printf("Tests passed.\n");
#endif
}