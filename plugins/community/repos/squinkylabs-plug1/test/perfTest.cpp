#include <functional>
#include <time.h>
#include <cmath>
#include <limits>

#include "AudioMath.h"
#include "BiquadParams.h"
#include "BiquadFilter.h"
#include "BiquadState.h"
#include "ColoredNoise.h"
#include "FrequencyShifter.h"
#include "HilbertFilterDesigner.h"
#include "LookupTableFactory.h"
#include "TestComposite.h"
#include "Tremolo.h"
#include "VocalAnimator.h"
#include "VocalFilter.h"

using Shifter = FrequencyShifter<TestComposite>;
using Animator = VocalAnimator<TestComposite>;
using VocFilter = VocalFilter<TestComposite>;
using Colors = ColoredNoise<TestComposite>;
using Trem = Tremolo<TestComposite>;

#include "MeasureTime.h"

// There are many tests that are disabled with #if 0.
// In most cases they still work, but don't need to be run regularly

#if 0
static void test1()
{
    double d = .1;
    srand(57);
    const double scale = 1.0 / RAND_MAX;

    MeasureTime<float>::run("test1 (do nothing)", [&d, scale]() {
        return TestBuffers<float>::get();
        }, 1);

    MeasureTime<float>::run("test1 sin", []() {
        float x = std::sin(TestBuffers<float>::get());
        return x;
        }, 1);

    MeasureTime<double>::run("test1 sin double", []() {
        float x = std::sin(TestBuffers<float>::get());
        return x;
        }, 1);

    MeasureTime<float>::run("test1 sinx2 float", []() {
        float x = std::sin(TestBuffers<float>::get());
        x = std::sin(x);
        return x;
        }, 1);

    MeasureTime<float>::run("mult float-10", []() {
        float x = TestBuffers<float>::get();
        float y = TestBuffers<float>::get();
        return x * y;
        }, 10);

    MeasureTime<double>::run("mult dbl", []() {
        double x = TestBuffers<double>::get();
        double y = TestBuffers<double>::get();
        return x * y;
        }, 1);

    MeasureTime<float>::run("div float", []() {
        float x = TestBuffers<float>::get();
        float y = TestBuffers<float>::get();
        return x / y;
        }, 1);

    MeasureTime<double>::run("div dbl", []() {
        double x = TestBuffers<double>::get();
        double y = TestBuffers<double>::get();
        return x / y;
        }, 1);

    MeasureTime<float>::run("test1 (do nothing)", [&d, scale]() {
        return TestBuffers<float>::get();
        }, 1);

    MeasureTime<float>::run("test1 pow2 float", []() {
        float x = std::pow(2, TestBuffers<float>::get());
        return x;
        }, 1);
    MeasureTime<float>::run("test1 pow rnd float", []() {
        float x = std::pow(TestBuffers<float>::get(), TestBuffers<float>::get());
        return x;
        }, 1);

    MeasureTime<float>::run("test1 exp float", []() {
        float x = std::exp(TestBuffers<float>::get());
        return x;
        }, 1);
}
#endif

template <typename T>
static void testHilbert()
{
    BiquadParams<T, 3> paramsSin;
    BiquadParams<T, 3> paramsCos;
    BiquadState<T, 3> state;
    HilbertFilterDesigner<T>::design(44100, paramsSin, paramsCos);

    MeasureTime<T>::run("hilbert", [&state, &paramsSin]() {

        T d = BiquadFilter<T>::run(TestBuffers<T>::get(), state, paramsSin);
        return d;
        }, 1);
}

#if 0
static void testExpRange()
{
    using T = float;
    LookupTableParams<T> table;
    LookupTableFactory<T>::makeExp2(table);

    MeasureTime<T>::run("exp lookup", [&table]() {

        T d = LookupTable<T>::lookup(table, TestBuffers<T>::get());
        return d;
        }, 1);
}
#endif

static void testShifter()
{
    Shifter fs;

    fs.setSampleRate(44100);
    fs.init();

    fs.inputs[Shifter::AUDIO_INPUT].value = 0;

    MeasureTime<float>::run("shifter", [&fs]() {
        fs.inputs[Shifter::AUDIO_INPUT].value = TestBuffers<float>::get();
        fs.step();
        return fs.outputs[Shifter::SIN_OUTPUT].value;
        }, 1);
}

static void testAnimator()
{
    Animator an;

    an.setSampleRate(44100);
    an.init();

    an.inputs[Shifter::AUDIO_INPUT].value = 0;

    MeasureTime<float>::run("animator", [&an]() {
        an.inputs[Shifter::AUDIO_INPUT].value = TestBuffers<float>::get();
        an.step();
        return an.outputs[Shifter::SIN_OUTPUT].value;
        }, 1);
}


static void testVocalFilter()
{
    VocFilter an;

    an.setSampleRate(44100);
    an.init();

    an.inputs[Shifter::AUDIO_INPUT].value = 0;

    MeasureTime<float>::run("vocal filter", [&an]() {
        an.inputs[Shifter::AUDIO_INPUT].value = TestBuffers<float>::get();
        an.step();
        return an.outputs[Shifter::SIN_OUTPUT].value;
        }, 1);
}



static void testColors()
{
    Colors co;

    co.setSampleRate(44100);
    co.init();


    MeasureTime<float>::run("colors", [&co]() {
        co.step();
        return co.outputs[Colors::AUDIO_OUTPUT].value;
        }, 1);
}

static void testTremolo()
{
    Trem tr;

    tr.setSampleRate(44100);
    tr.init();


    MeasureTime<float>::run("trem", [&tr]() {
        tr.inputs[Trem::AUDIO_INPUT].value = TestBuffers<float>::get();
        tr.step();
        return tr.outputs[Trem::AUDIO_OUTPUT].value;
        }, 1);
}

#if 0
static void testAttenuverters()
{
    auto scaler = AudioMath::makeLinearScaler<float>(-2, 2);
    MeasureTime<float>::run("linear scaler", [&scaler]() {
        float cv = TestBuffers<float>::get();
        float knob = TestBuffers<float>::get();
        float trim = TestBuffers<float>::get();
        return scaler(cv, knob, trim);
        }, 1);

    LookupTableParams<float> lookup;
    LookupTableFactory<float>::makeBipolarAudioTaper(lookup);
    MeasureTime<float>::run("bipolar lookup", [&lookup]() {
        float x = TestBuffers<float>::get();
        return LookupTable<float>::lookup(lookup, x);
        }, 1);


   // auto refFuncPos = AudioMath::makeFunc_AudioTaper(LookupTableFactory<T>::audioTaperKnee());

    {

        auto bipolarScaler = [&lookup, &scaler](float cv, float knob, float trim) {
            float scaledTrim = LookupTable<float>::lookup(lookup, cv);
            return scaler(cv, knob, scaledTrim);
        };

        MeasureTime<float>::run("bipolar scaler", [&bipolarScaler]() {
            float cv = TestBuffers<float>::get();
            float knob = TestBuffers<float>::get();
            float trim = TestBuffers<float>::get();
            return bipolarScaler(cv, knob, trim);
            }, 1);
    }
}
#endif

void perfTest()
{
#if 0
    testAttenuverters();
    testExpRange();
#endif
    testVocalFilter();
    testAnimator();
    testShifter();

    testColors();
    testTremolo();

   // test1();
#if 0
    testHilbert<float>();
    testHilbert<double>();
#endif
}