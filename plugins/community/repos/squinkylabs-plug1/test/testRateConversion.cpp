
#include "IIRUpsampler.h"
#include "IIRDecimator.h"

#include "asserts.h"

static void setup(IIRUpsampler& up, IIRDecimator& dec)
{
   // float cutoff = .25 / 16;
    up.setup(16);
    dec.setup(16);
}

// test that the functions can be called
static void test0()
{
    float buffer[16];

    IIRUpsampler up;
    IIRDecimator dec;
    setup(up, dec);
  
    up.process(buffer, 0);
    dec.process(buffer);
}

// test 0 -> 0
static void test1()
{
    float buffer[16];

    IIRUpsampler up;
    IIRDecimator dec;
    setup(up, dec);

    up.process(buffer, 0);
    const float x = dec.process(buffer);
    assertEQ(x, 0);
}

// test 10 -> 10
static void test2()
{
    float buffer[16];

    IIRUpsampler up;
    IIRDecimator dec;
    setup(up, dec);

    float x;
    for (int i = 0; i < 100; ++i) {
        up.process(buffer, 10);
        x = dec.process(buffer);
    }
    assertClose(x, 10, .001);
}

void testRateConversion()
{
    test0();
    test1();
    test2();
}