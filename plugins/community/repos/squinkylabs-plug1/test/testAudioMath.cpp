
#include <assert.h>
#include <iostream>

#include "asserts.h"
#include "AudioMath.h"
#include "LookupTableFactory.h"

using namespace std;

static void test0()
{
    assert(AudioMath::closeTo(0, 0, .000001));
    assert(AudioMath::closeTo(1000, 1001, 1.1));
    assert(!AudioMath::closeTo(1000, 1010, 1.1));
    assert(!AudioMath::closeTo(1010, 1000, 1.1));
}

static void test1()
{
    assert(AudioMath::closeTo(3.145, AudioMath::Pi, .1));
    assert(AudioMath::closeTo(3.145 / 2, AudioMath::Pi_2, .1));
    assert(AudioMath::closeTo(log(10), AudioMath::Ln10, .001));
}

static void test2()
{
    const double d = .0001;
    std::function<double(double)> f = AudioMath::makeFunc_Sin();
    assert(AudioMath::closeTo(f(0), 0, d));
    assert(AudioMath::closeTo(f(.5), 0, d));
    assert(AudioMath::closeTo(f(.25), 1, d));
    assert(AudioMath::closeTo(f(.75), -1, d));
    assert(AudioMath::closeTo(f(.125), 1.0 / sqrt(2), d));
}

static void test3()
{
    const double d = .0001;
    std::function<double(double)> f = AudioMath::makeFunc_Exp(0, 4, 2, 32);

    assert(AudioMath::closeTo(f(0), 2, d));
    assert(AudioMath::closeTo(f(1), 4, d));
    assert(AudioMath::closeTo(f(2), 8, d));
    assert(AudioMath::closeTo(f(3), 16, d));
    assert(AudioMath::closeTo(f(4), 32, d));
    assert(f(5) > 33);
    assert(f(-1) < 1.5);
}

static void testAudioTaper()
{
    double db = -18;
    std::function<double(double)> f = AudioMath::makeFunc_AudioTaper(db);
    assertClose(f(1), 1, .001);
    assertClose(f(.25), .125, .001);
    assertClose(f(.251), .126, .001);
    assertClose(f(.249), 1.0 / 8.0, .001);
    assertClose(f(0), 0, .001);
}

static void testScaler()
{
    AudioMath::ScaleFun<float> f = AudioMath::makeLinearScaler<float>(3, 4);
    // scale(cv, knob, trim

    // knob comes through only shifted
    assertEQ(f(0, -5, 0), 3.);
    assertEQ(f(0, 5, 0), 4.);
    assertEQ(f(0, 0, 0), 3.5);

    // cv also come through, it trim up
    assertEQ(f(-5, 0, 1), 3.);
    assertEQ(f(5, 0, 1), 4.);
    assertEQ(f(0, 0, 1), 3.5);

    // no cv if trim 0
    assertEQ(f(-5, 0, 0), 3.5);

    // neg trim inverts cv
    assertEQ(f(-5, 0, -1), 4.);


    // trim half way
    assertEQ(f(5, 0, .5), 3.75);
}

static void testBipolarAudioScaler()
{
    AudioMath::ScaleFun<float> f = AudioMath::makeScalerWithBipolarAudioTrim(3, 4);
    // scale(cv, knob, trim

    // knob comes through only shifted
    assertEQ(f(0, -5, 0), 3.);
    assertEQ(f(0, 5, 0), 4.);
    assertEQ(f(0, 0, 0), 3.5);

    // cv also come through, it trim up
    assertEQ(f(-5, 0, 1), 3.);
    assertEQ(f(5, 0, 1), 4.);
    assertEQ(f(0, 0, 1), 3.5);

    // no cv if trim 0
    assertEQ(f(-5, 0, 0), 3.5);

    // neg trim inverts cv
    assertEQ(f(-5, 0, -1), 4.);

    // trim quarter  - should be audio knee
    auto f2 = AudioMath::makeScalerWithBipolarAudioTrim(-1, 1);
    float x = f2(5, 0, .25);
    float y = (float) AudioMath::gainFromDb(LookupTableFactory<float>::audioTaperKnee());
    assertClose(x, y, .001);
}

static void testAudioScaler()
{
    AudioMath::SimpleScaleFun<float> f = AudioMath::makeSimpleScalerAudioTaper(3, 4);

    float kneeGain = (float) AudioMath::gainFromDb(LookupTableFactory<float>::audioTaperKnee());

    // knob comes through with taper
    assertEQ(f(0, -5), 3.);
    assertEQ(f(0, 5), 4.);

    // 1/4, is the knee
    assertEQ(f(0, -2.5), 3.0f + kneeGain);

    // cv also come through, it trim up
    assertEQ(f(-5, 0), 3.);
    assertEQ(f(5, 0), 4.);
    assertEQ(f(-2.5, 0), 3.0f + kneeGain);
}


static void testAudioScaler2()
{
    AudioMath::SimpleScaleFun<float> f = AudioMath::makeSimpleScalerAudioTaper(0, 20);

    float kneeGain = (float) AudioMath::gainFromDb(LookupTableFactory<float>::audioTaperKnee());

    // knob comes through with taper
    assertEQ(f(0, -5), 0);
    assertEQ(f(0, 5), 20);

    // 1/4, is the knee
    assertEQ(f(0, -2.5), 20 * kneeGain);

    // cv also come through, it trim up
    assertEQ(f(-5, 0), 0);
    assertEQ(f(5, 0), 20);
    assertEQ(f(-2.5, 0), 20 * kneeGain);
}



static void testFold()
{
    float last = 0;
    const float deltaX = 0.05f;
    for (float x = 0; x < 15; x += deltaX) {
        float y = AudioMath::fold(x);
        float absChange = std::abs(y - last);
        last = y;
        assertLE(absChange, deltaX + .0001f);
    }
}


static void testFoldNegative()
{
    float last = 0;
    const float deltaX = 0.05f;
    for (float x = 0; x > -15; x -= deltaX) {
        float y = AudioMath::fold(x);
        float absChange = std::abs(y - last);
        last = y;
        assertLE(absChange, deltaX + .0001f);
    }
}

void testAudioMath()
{
    test0();
    test1();
    test2();
    test3();
    testAudioTaper();
    testScaler();
    testBipolarAudioScaler();
    testAudioScaler();
    testAudioScaler2();
    testFold();
    testFoldNegative();
}