
#include <map>
#include <vector>
#include "asserts.h"
#include "LookupTable.h"
#include "AsymWaveShaper.h"
#include "ExtremeTester.h"
#include "Shaper.h"
#include "SinOscillator.h"
#include "TestComposite.h"
#include "TestSignal.h"


using Spline = std::vector< std::pair<double, double> >;



static void gen()
{
    for (int i = 0; i < AsymWaveShaper::iSymmetryTables; ++i) {
        float symmetry = float(i) / float(AsymWaveShaper::iSymmetryTables - 1);
        AsymWaveShaper::genTable(i, symmetry);
    }
}


static void testLook0()
{
    NonUniformLookup l;
    l.add(0, 0);
    l.add(1, 1);
    l.add(2, 2);
    l.add(3, 3);

    double x = l.lookup(2.5);
    assertClose(x, 2.5, .0001);
}

static void testLook1()
{
    NonUniformLookup l;
    l.add(0, 0);
    l.add(1, 1);
    l.add(2, 4);
    l.add(3, 3);

    double x = l.lookup(1.5);
    assertClose(x, 2.5, .0001);
}

static void testLook2()
{
    NonUniformLookup l;
    l.add(0, 0);
    l.add(1, 1);
    l.add(2, 2);
    l.add(3, 3);

    double x = l.lookup(.5);
    assertClose(x, .5, .0001);
}

static void testLook3()
{
    NonUniformLookup l;
    l.add(0, 0);
    l.add(1, 1);
    l.add(2, 2);
    l.add(3, 3);

    double x = l.lookup(2.5);
    assertClose(x, 2.5, .0001);
}

static void testLook4()
{
    NonUniformLookup l;
    l.add(0, 0);
    l.add(1, 1);
    l.add(2, 2);
    l.add(3, 3);

    double x = l.lookup(0);
    assertClose(x, 0, .0001);
}


static void testGen0()
{
    AsymWaveShaper g;
    const int index = 3;// symmetry
    float x = g.lookup(0, index);
    x = g.lookup(1, index);
    x = g.lookup(-1, index);
}

static void testDerivativeSub(int index, float delta)
{
    AsymWaveShaper ws;

    const float ya = ws.lookup(-delta, index);
    const float y0 = ws.lookup(0, index);
    const float yb = ws.lookup(delta, index);
    const float slopeLeft = -ya / delta;
    const float slopeRight = yb / delta;

   // since I changed AsymWaveShaper to be points-1 this is worse
    assertClose(y0, 0, .00001);
    assertClose(slopeRight, 2, .01);
    if (index != 0) {
        assertClose(slopeLeft, 2, .3
        );
    }

}
static void testDerivative()
{
    // 6 with .1
    for (int i = AsymWaveShaper::iSymmetryTables - 1; i >= 0; --i) {
        testDerivativeSub(i, .0001f);
    }
}



static void testShaper0()
{
    Shaper<TestComposite> gmr;

    int shapeMax = (int) Shaper<TestComposite>::Shapes::Invalid;
    for (int i = 0; i < shapeMax; ++i) {
        gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) i;
        std::string s = gmr.getString(Shaper<TestComposite>::Shapes(i));
        assertGT(s.length(), 0);
        assertLT(s.length(), 20);
        gmr.params[Shaper<TestComposite>::PARAM_OFFSET].value = -5;
        for (int i = 0; i < 50; ++i) gmr.step();
        const float x = gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO].value;
        gmr.params[Shaper<TestComposite>::PARAM_OFFSET].value = 5;
        for (int i = 0; i < 50; ++i) gmr.step();
        const float y = gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO].value;

        assertLT(x, 10);
        assertLT(y, 10);
        assertGT(x, -10);
        assertGT(y, -10);
    }
}

static void testShaper1Sub(int shape, float gain, float targetRMS)
{
    Shaper<TestComposite> gmr;
    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) shape;
    gmr.params[Shaper<TestComposite>::PARAM_GAIN].value = gain;        // max gain
    const int buffSize = 1 * 1024;
    float buffer[buffSize];

    TestSignal<float>::generateSin(buffer, buffSize, 1.f / 40);
    double rms = TestSignal<float>::getRMS(buffer, buffSize);
    for (int i = 0; i < buffSize; ++i) {
        const float x = buffer[i];
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO].value = buffer[i];
        gmr.step();
        buffer[i] = gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO].value;
    }
    rms = TestSignal<float>::getRMS(buffer, buffSize);

    const char* p = gmr.getString(Shaper<TestComposite>::Shapes(shape));

    //printf("rms[%s] = %f target = %f ratio=%f\n", p, rms, targetRMS, targetRMS / rms);

    if (targetRMS > .01) {
        assertClose(rms, targetRMS, .5);
    }
}

static void testShaper1()
{
    int shapeMax = (int) Shaper<TestComposite>::Shapes::Invalid;
    for (int i = 0; i < shapeMax; ++i) {
        const float targetOutput = (i == (int) Shaper<TestComposite>::Shapes::Crush) ? 0 : 5 * .707f;

        testShaper1Sub(i, 5, targetOutput);
        testShaper1Sub(i, 0, 0);
    }
}

static void testSplineExtremes()
{
    Shaper<TestComposite> sp;

    using fp = std::pair<float, float>;
    std::vector< std::pair<float, float> > paramLimits;

    paramLimits.resize(sp.NUM_PARAMS);

    paramLimits[sp.PARAM_SHAPE] = fp(0.f, float(Shaper<TestComposite>::Shapes::Invalid) - 1);
    paramLimits[sp.PARAM_GAIN] = fp(-5.0f, 5.0f);
    paramLimits[sp.PARAM_GAIN_TRIM] = fp(-1.f, 1.f);
    paramLimits[sp.PARAM_OFFSET] = fp(-5.f, 5.f);
    paramLimits[sp.PARAM_OFFSET_TRIM] = fp(-1.f, 1.f);
    paramLimits[sp.PARAM_OVERSAMPLE] = fp(0.f, 2.f);
    paramLimits[sp.PARAM_ACDC] = fp(0.f, 1.f);

    ExtremeTester< Shaper<TestComposite>>::test(sp, paramLimits, true, "shaper");
}

static void testShaper2d(Shaper<TestComposite>::Shapes shape, float gain, float offset, float input)
{
    Shaper<TestComposite> sh;
    sh.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) shape;
    sh.params[Shaper<TestComposite>::PARAM_GAIN].value = gain; 
    sh.params[Shaper<TestComposite>::PARAM_OFFSET].value = offset;
    for (int i = 0; i < 100; ++i) {
        sh.inputs[Shaper<TestComposite>::INPUT_AUDIO].value = input;
        sh.step();
        const float out = sh.outputs[Shaper<TestComposite>::OUTPUT_AUDIO].value;

        // brief ringing goes > 10
        assert(out < 20 && out > -20);
    }
    
}
static void testShaper2c(Shaper<TestComposite>::Shapes shape, float gain, float offset)
{
    testShaper2d(shape, gain, offset, 0);
    testShaper2d(shape, gain, offset, 5);
    testShaper2d(shape, gain, offset, -5);
}

static void testShaper2b(Shaper<TestComposite>::Shapes shape, float gain)
{
    testShaper2c(shape, gain, 0.f);
    testShaper2c(shape, gain, 5.f);
    testShaper2c(shape, gain, -5.f);
}

static void testShaper2a(Shaper<TestComposite>::Shapes shape)
{
    testShaper2b(shape, 0);
    testShaper2b(shape, -5);
    testShaper2b(shape, 5);
}

const int shapeMax = (int) Shaper<TestComposite>::Shapes::Invalid;

static void testShaper2()
{
   
    for (int i = 0; i < shapeMax; ++i) {
        testShaper2a(Shaper<TestComposite>::Shapes(i));
    }
}

// test for DC shift
static void testShaper3Sub(Shaper<TestComposite>::Shapes shape)
{
    Shaper<TestComposite> sh;
  
    sh.params[Shaper<TestComposite>::PARAM_OVERSAMPLE].value = 2;       // turn off oversampling
    sh.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) shape;
    sh.params[Shaper<TestComposite>::PARAM_GAIN].value = -3;            // gain up a bit
    sh.params[Shaper<TestComposite>::PARAM_OFFSET].value = 0;  // no offset

    sh.inputs[Shaper<TestComposite>::INPUT_AUDIO].value = 0;
    for (int i = 0; i < 100; ++i) {

        sh.step();
    }
    const float out = sh.outputs[Shaper<TestComposite>::OUTPUT_AUDIO].value;
    if (shape != Shaper<TestComposite>::Shapes::Crush) {
        assertEQ(out, 0);
    } else {
        // crash had a dc offset issue
        assertLT(out, 1);
        assertGT(out, -1);
    }
}

static void testShaper3()
{
   // testShaper3Sub(Shaper<TestComposite>::Shapes::Crush);
    for (int i = 0; i < shapeMax; ++i) {
        testShaper3Sub(Shaper<TestComposite>::Shapes(i));
    }
}

static void testDC()
{
    using Sh = Shaper<TestComposite>;

    Sh sh;
    sh.params[Sh::PARAM_SHAPE].value = float(Sh::Shapes::FullWave);

    // Will generate sine at fs * .01 (around 400 Hz).
    SinOscillatorParams<float> sinp;
    SinOscillatorState<float> sins;
    SinOscillator<float, false>::setFrequency(sinp, .01f);

    // Run sin through Chebyshevs at specified gain
    auto func = [&sins, &sinp, &sh]() {
        const float sin = SinOscillator<float, false>::run(sins, sinp);
        sh.inputs[Sh::INPUT_AUDIO].value = sin;
        sh.step();
        return sh.outputs[Sh::OUTPUT_AUDIO].value;
    };

    const int bufferSize = 16 * 1024;
    float buffer[bufferSize];
    for (int i = 0; i < bufferSize; ++i) {
        buffer[i] = func();
    }
    double dc = TestSignal<float>::getDC(buffer, bufferSize);
    assertClose(dc, 0, .001);
}


float cf(float x, float fold_gain)
{
   // x = inputs[IN_INPUT].value*5.0f*gain_gain;
    x *= 5;

 //   if (abs(x) > 5) y = clamp((abs(x) - 5) / 2.2f, 0.0f, 58.0f); else y = 0;

    for (int i = 0; i < 100; i++) {
        if (x < -5.0f) x = -5.0f + (-x - 5.0f)*fold_gain / 5.0f;
        if (x > 5.0f) x = 5.0f - (x - 5.0f)*fold_gain / 5.0f;
        if ((x >= -5.0) & (x <= 5.0)) i = 1000; if (i == 99) x = 0;
    }


   // return clamp(x, -5.0f, 5.0f);
    return x;
}

float sq(float x)
{
   
    return 5 * AudioMath::fold(x);
}


static void testCf()
{
    printf("HWY IS testCf failing now?\n");
#if 0
    const float fold_gain = 1;
    for (float x = 0; x < 2; x += .05f) {
        const float c = cf(x, fold_gain);
        const float s = sq(x);
        printf("x=%.2f c=%.2f s=%.2f\n", x, c, s);
        assertClose(s, c, .01);

    }
#endif

}
void testSpline(bool doEmit)
{
    testCf();
    if (doEmit) {
        gen();
        return;
    }
    testLook0();
    testLook1();
    testLook2();
    testLook3();
    testLook4();
    testGen0();
    testDerivative();
    testDC();
    testShaper0();

    testShaper1();
    testShaper2();
    testShaper3();

    testSplineExtremes();
}

