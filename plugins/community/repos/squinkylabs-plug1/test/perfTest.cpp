#include <functional>
#include <time.h>
#include <cmath>
#include <limits>

#include "EvenVCO.h"
//#include "EvenVCO_orig.h"

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
#include "LFN.h"
#include "GMR.h"
#include "CHB.h"
#include "FunVCOComposite.h"
#include "EV3.h"
#include "daveguide.h"
#include "Shaper.h"
#include "Super.h"


using Shifter = FrequencyShifter<TestComposite>;
using Animator = VocalAnimator<TestComposite>;
using VocFilter = VocalFilter<TestComposite>;
using Colors = ColoredNoise<TestComposite>;
using Trem = Tremolo<TestComposite>;


#include "MeasureTime.h"

#if defined(_MSC_VER) || defined(ARCH_WIN)
double SqTime::frequency = 0;
#endif

// There are many tests that are disabled with #if 0.
// In most cases they still work, but don't need to be run regularly

#if 0
static void test1()
{
    double d = .1;
    srand(57);
    const double scale = 1.0 / RAND_MAX;



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

double overheadInOut = 0;
double overheadOutOnly = 0;

static void setup()
{
#ifdef _DEBUG
//    assert(false);  // don't run this in debug
#endif
    double d = .1;
    const double scale = 1.0 / RAND_MAX;
    overheadInOut = MeasureTime<float>::run(0.0, "test1 (do nothing i/o)", [&d, scale]() {
        return TestBuffers<float>::get();
        }, 1);

    overheadOutOnly = MeasureTime<float>::run(0.0, "test1 (do nothing oo)", [&d, scale]() {
        return 0.0f;
        }, 1);

}

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

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(overheadInOut, "shifter", [&fs]() {
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

    MeasureTime<float>::run(overheadInOut, "animator", [&an]() {
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

    MeasureTime<float>::run(overheadInOut, "vocal filter", [&an]() {
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


    MeasureTime<float>::run(overheadInOut, "colors", [&co]() {
        co.step();
        return co.outputs[Colors::AUDIO_OUTPUT].value;
        }, 1);
}

static void testTremolo()
{
    Trem tr;

    tr.setSampleRate(44100);
    tr.init();


    MeasureTime<float>::run(overheadInOut, "trem", [&tr]() {
        tr.inputs[Trem::AUDIO_INPUT].value = TestBuffers<float>::get();
        tr.step();
        return tr.outputs[Trem::AUDIO_OUTPUT].value;
        }, 1);
}

static void testLFN()
{
    LFN<TestComposite> lfn;

    lfn.setSampleTime(1.0f / 44100.f);
    lfn.init();

    MeasureTime<float>::run(overheadOutOnly, "lfn", [&lfn]() {
        lfn.step();
        return lfn.outputs[LFN<TestComposite>::OUTPUT].value;
        }, 1);
}

#if 0
static void testEvenOrig()
{
    EvenVCO_orig<TestComposite> lfn;

    lfn.outputs[EvenVCO_orig<TestComposite>::EVEN_OUTPUT].active = true;
    lfn.outputs[EvenVCO_orig<TestComposite>::SINE_OUTPUT].active = true;
    lfn.outputs[EvenVCO_orig<TestComposite>::TRI_OUTPUT].active = true;
    lfn.outputs[EvenVCO_orig<TestComposite>::SQUARE_OUTPUT].active = true;
    lfn.outputs[EvenVCO_orig<TestComposite>::SAW_OUTPUT].active = true;

    for (int i = 0; i < 100; ++i) lfn.step();

    MeasureTime<float>::run(overheadOutOnly, "Even orig", [&lfn]() {
        lfn.inputs[EvenVCO_orig<TestComposite>::PITCH1_INPUT].value = TestBuffers<float>::get();
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].value;
        }, 1);
}
#endif

static void testEven()
{
    EvenVCO<TestComposite> lfn;


    lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].active = true;
    lfn.outputs[EvenVCO<TestComposite>::SINE_OUTPUT].active = true;
    lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].active = true;
    lfn.outputs[EvenVCO<TestComposite>::SQUARE_OUTPUT].active = true;
    lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].active = true;
    MeasureTime<float>::run(overheadOutOnly, "Even, all outs", [&lfn]() {
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].value;
        }, 1);
}

static void testEvenEven()
{
    EvenVCO<TestComposite> lfn;

    lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].active = true;
    lfn.outputs[EvenVCO<TestComposite>::SINE_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::SQUARE_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].active = false;

    MeasureTime<float>::run(overheadOutOnly, "Even, even only", [&lfn]() {
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].value;
        }, 1);
}

static void testEvenSin()
{
    EvenVCO<TestComposite> lfn;

    lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::SINE_OUTPUT].active = true;
    lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::SQUARE_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].active = false;

    MeasureTime<float>::run(overheadOutOnly, "Even, sin only", [&lfn]() {
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].value;
        }, 1);
}

static void testEvenSaw()
{
    EvenVCO<TestComposite> lfn;

    lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::SINE_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::SQUARE_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].active = true;

    for (int i = 0; i < 100; ++i) lfn.step();

    MeasureTime<float>::run(overheadOutOnly, "Even, saw only", [&lfn]() {
        lfn.inputs[EvenVCO<TestComposite>::PITCH1_INPUT].value = TestBuffers<float>::get();
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].value;
        }, 1);
}


static void testEvenTri()
{
    EvenVCO<TestComposite> lfn;

    lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::SINE_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].active = true;
    lfn.outputs[EvenVCO<TestComposite>::SQUARE_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].active = false;

    MeasureTime<float>::run(overheadOutOnly, "Even, tri only", [&lfn]() {
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].value;
        }, 1);
}

static void testEvenSq()
{
    EvenVCO<TestComposite> lfn;

    lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::SINE_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::SQUARE_OUTPUT].active = true;
    lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].active = false;

    MeasureTime<float>::run(overheadOutOnly, "Even, Sq only", [&lfn]() {
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].value;
        }, 1);
}

static void testEvenSqSaw()
{
    EvenVCO<TestComposite> lfn;

    lfn.outputs[EvenVCO<TestComposite>::EVEN_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::SINE_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].active = false;
    lfn.outputs[EvenVCO<TestComposite>::SQUARE_OUTPUT].active = true;
    lfn.outputs[EvenVCO<TestComposite>::SAW_OUTPUT].active = true;

    MeasureTime<float>::run(overheadOutOnly, "Even, Sq Saw", [&lfn]() {
        lfn.step();
        return lfn.outputs[EvenVCO<TestComposite>::TRI_OUTPUT].value;
        }, 1);
}

static void testFun()
{
    FunVCOComposite<TestComposite> lfn;

    for (int i = 0; i < lfn.NUM_OUTPUTS; ++i) {
        lfn.outputs[i].active = true;
    }

    lfn.setSampleRate(44100.f);
    const bool isAnalog = false;
    lfn.params[FunVCOComposite<TestComposite>::MODE_PARAM].value = isAnalog ? 1.0f : 0.f;

    MeasureTime<float>::run(overheadOutOnly, "Fun all on, digital", [&lfn]() {
        lfn.step();
        return lfn.outputs[FunVCOComposite<TestComposite>::TRI_OUTPUT].value +
            lfn.outputs[FunVCOComposite<TestComposite>::SAW_OUTPUT].value +
            lfn.outputs[FunVCOComposite<TestComposite>::SIN_OUTPUT].value +
            lfn.outputs[FunVCOComposite<TestComposite>::SQR_OUTPUT].value;
        }, 1);
}

static void testFunNone()
{
    FunVCOComposite<TestComposite> lfn;

    for (int i = 0; i < lfn.NUM_OUTPUTS; ++i) {
        lfn.outputs[i].active = false;
    }

    lfn.setSampleRate(44100.f);

    MeasureTime<float>::run(overheadOutOnly, "Fun all off", [&lfn]() {
        lfn.step();
        return lfn.outputs[FunVCOComposite<TestComposite>::TRI_OUTPUT].value;
        }, 1);
}

static void testFunSaw(bool isAnalog)
{
    FunVCOComposite<TestComposite> lfn;

    lfn.outputs[FunVCOComposite<TestComposite>::SIN_OUTPUT].active = false;
    lfn.outputs[FunVCOComposite<TestComposite>::TRI_OUTPUT].active = false;
    lfn.outputs[FunVCOComposite<TestComposite>::SQR_OUTPUT].active = false;
    lfn.outputs[FunVCOComposite<TestComposite>::SAW_OUTPUT].active = true;

    //  oscillator.analog = TBase::params[MODE_PARAM].value > 0.0f;
    lfn.params[FunVCOComposite<TestComposite>::MODE_PARAM].value = isAnalog ? 1.0f : 0.f;

    lfn.setSampleRate(44100.f);

    std::string title = isAnalog ? "Fun Saw Analog" : "Fun Saw Digital";
    MeasureTime<float>::run(overheadOutOnly, title.c_str(), [&lfn]() {
        lfn.step();
        return lfn.outputs[FunVCOComposite<TestComposite>::SAW_OUTPUT].value;
        }, 1);
}

static void testFunSin(bool isAnalog)
{
    FunVCOComposite<TestComposite> lfn;

    lfn.outputs[FunVCOComposite<TestComposite>::SIN_OUTPUT].active = true;
    lfn.outputs[FunVCOComposite<TestComposite>::TRI_OUTPUT].active = false;
    lfn.outputs[FunVCOComposite<TestComposite>::SQR_OUTPUT].active = false;
    lfn.outputs[FunVCOComposite<TestComposite>::SAW_OUTPUT].active = false;

    lfn.params[FunVCOComposite<TestComposite>::MODE_PARAM].value = isAnalog ? 1.0f : 0.f;

    lfn.setSampleRate(44100.f);

    std::string title = isAnalog ? "Fun Sin Analog" : "Fun Sin Digital";
    MeasureTime<float>::run(overheadOutOnly, title.c_str(), [&lfn]() {
        lfn.step();
        return lfn.outputs[FunVCOComposite<TestComposite>::SAW_OUTPUT].value;
        }, 1);
}

static void testFunSq()
{
    FunVCOComposite<TestComposite> lfn;

    lfn.outputs[FunVCOComposite<TestComposite>::SIN_OUTPUT].active = false;
    lfn.outputs[FunVCOComposite<TestComposite>::TRI_OUTPUT].active = false;
    lfn.outputs[FunVCOComposite<TestComposite>::SQR_OUTPUT].active = true;
    lfn.outputs[FunVCOComposite<TestComposite>::SAW_OUTPUT].active = false;

    lfn.setSampleRate(44100.f);

    MeasureTime<float>::run(overheadOutOnly, "Fun sq", [&lfn]() {
        lfn.step();
        return lfn.outputs[FunVCOComposite<TestComposite>::SAW_OUTPUT].value;
        }, 1);
}

static void testCHB(bool econ)
{
    CHB<TestComposite> chb;

//    chb.init();
    chb.setEconomy(econ);

    std::string name = "chb ";
    name += econ ? "econ" : "full";
    MeasureTime<float>::run(overheadOutOnly, name.c_str(), [&chb]() {
        chb.step();
        return chb.outputs[CHB<TestComposite>::MIX_OUTPUT].value;
        }, 1);
}

static void testCHBdef()
{
    CHB<TestComposite> chb;
    std::string name = "chbdef ";
    MeasureTime<float>::run(overheadOutOnly, name.c_str(), [&chb]() {
        chb.step();
        return chb.outputs[CHB<TestComposite>::MIX_OUTPUT].value;
        }, 1);
}

static void testEV3()
{
    EV3<TestComposite> ev3;

    //    chb.init();

    MeasureTime<float>::run(overheadOutOnly, "ev3", [&ev3]() {
        ev3.step();
        return ev3.outputs[EV3<TestComposite>::MIX_OUTPUT].value;
        }, 1);
}

static void testGMR()
{
    GMR<TestComposite> gmr;

    gmr.setSampleRate(44100);
    gmr.init();

    MeasureTime<float>::run(overheadOutOnly, "gmr", [&gmr]() {
        gmr.step();
        return gmr.outputs[GMR<TestComposite>::TRIGGER_OUTPUT].value;
        }, 1);
}

static void testDG()
{
    Daveguide<TestComposite> gmr;

   // gmr.setSampleRate(44100);
   // gmr.init();

    MeasureTime<float>::run(overheadOutOnly, "dg", [&gmr]() {
        gmr.step();
        return gmr.outputs[GMR<TestComposite>::TRIGGER_OUTPUT].value;
        }, 1);
}

// 95
// down to 67 for just the oversampler.
static void testShaper1a()
{
    Shaper<TestComposite> gmr;

    // gmr.setSampleRate(44100);
    // gmr.init();
    gmr.params[Shaper<TestComposite>::PARAM_OVERSAMPLE].value = 0;
    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) Shaper<TestComposite>::Shapes::FullWave;

    MeasureTime<float>::run(overheadOutOnly, "shaper fw 16X", [&gmr]() {
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO].value = TestBuffers<float>::get();
        gmr.step();
        return gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO].value;
        }, 1);
}

static void testShaper1b()
{
    Shaper<TestComposite> gmr;

    // gmr.setSampleRate(44100);
    // gmr.init();
    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) Shaper<TestComposite>::Shapes::FullWave;
    gmr.params[Shaper<TestComposite>::PARAM_OVERSAMPLE].value = 1;

    MeasureTime<float>::run(overheadOutOnly, "shaper fw 4X", [&gmr]() {
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO].value = TestBuffers<float>::get();
        gmr.step();
        return gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO].value;
        }, 1);
}

static void testSuper()
{
    Super<TestComposite> gmr;


    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) Shaper<TestComposite>::Shapes::FullWave;
    gmr.params[Shaper<TestComposite>::PARAM_OVERSAMPLE].value = 2;

    MeasureTime<float>::run(overheadOutOnly, "super", [&gmr]() {
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO].value = TestBuffers<float>::get();
        gmr.step();
        return gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO].value;
        }, 1);
}

static void testShaper1c()
{
    Shaper<TestComposite> gmr;

    // gmr.setSampleRate(44100);
    // gmr.init();
    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) Shaper<TestComposite>::Shapes::FullWave;
    gmr.params[Shaper<TestComposite>::PARAM_OVERSAMPLE].value = 2;

    MeasureTime<float>::run(overheadOutOnly, "shaper fw 1X", [&gmr]() {
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO].value = TestBuffers<float>::get();
        gmr.step();
        return gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO].value;
        }, 1);
}

// 284
static void testShaper2()
{
    Shaper<TestComposite> gmr;

    // gmr.setSampleRate(44100);
    // gmr.init();
    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) Shaper<TestComposite>::Shapes::Crush;

    MeasureTime<float>::run(overheadOutOnly, "shaper crush", [&gmr]() {
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO].value = TestBuffers<float>::get();
        gmr.step();
        return gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO].value;
        }, 1);
}

// 143
static void testShaper3()
{
    Shaper<TestComposite> gmr;

    // gmr.setSampleRate(44100);
    // gmr.init();
    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) Shaper<TestComposite>::Shapes::AsymSpline;

    MeasureTime<float>::run(overheadOutOnly, "shaper asy", [&gmr]() {
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO].value = TestBuffers<float>::get();
        gmr.step();
        return gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO].value;
        }, 1);
}

static void testShaper4()
{
    Shaper<TestComposite> gmr;

    // gmr.setSampleRate(44100);
    // gmr.init();
    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) Shaper<TestComposite>::Shapes::Fold;

    MeasureTime<float>::run(overheadOutOnly, "folder", [&gmr]() {
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO].value = TestBuffers<float>::get();
        gmr.step();
        return gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO].value;
        }, 1);
}

static void testShaper5()
{
    Shaper<TestComposite> gmr;

    // gmr.setSampleRate(44100);
    // gmr.init();
    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) Shaper<TestComposite>::Shapes::Fold2;

    MeasureTime<float>::run(overheadOutOnly, "folder II", [&gmr]() {
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO].value = TestBuffers<float>::get();
        gmr.step();
        return gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO].value;
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

#if 0
static void testNoise(bool useDefault)
{

    std::default_random_engine defaultGenerator{99};
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<double> distribution{0, 1.0};

    std::string title = useDefault ? "default random" : "fancy random";

    MeasureTime<float>::run(overheadOutOnly, title.c_str(), [useDefault, &distribution, &defaultGenerator, &gen]() {
        if (useDefault) return distribution(defaultGenerator);
        else return distribution(gen);
        }, 1);
}


static uint64_t xoroshiro128plus_state[2] = {};

static uint64_t rotl(const uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

static uint64_t xoroshiro128plus_next(void)
{
    const uint64_t s0 = xoroshiro128plus_state[0];
    uint64_t s1 = xoroshiro128plus_state[1];
    const uint64_t result = s0 + s1;

    s1 ^= s0;
    xoroshiro128plus_state[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
    xoroshiro128plus_state[1] = rotl(s1, 36); // c

    return result;
}

float randomUniformX()
{
    // 24 bits of granularity is the best that can be done with floats while ensuring that the return value lies in [0.0, 1.0).
    return (xoroshiro128plus_next() >> (64 - 24)) / powf(2, 24);
}

float randomNormalX()
{
    // Box-Muller transform
    float radius = sqrtf(-2.f * logf(1.f - randomUniformX()));
    float theta = 2.f * M_PI * randomUniformX();
    return radius * sinf(theta);

    // // Central Limit Theorem
    // const int n = 8;
    // float sum = 0.0;
    // for (int i = 0; i < n; i++) {
    // 	sum += randomUniform();
    // }
    // return (sum - n / 2.f) / sqrtf(n / 12.f);
}

static void testNormal()
{
    MeasureTime<float>::run(overheadOutOnly, "normal", []() {
        return randomNormalX();
        }, 1);
}
#endif

void perfTest()
{
    printf("starting perf test\n");
    fflush(stdout);
    setup();
#if 0
    testAttenuverters();
    testExpRange();
#endif

#if 0
    testNoise(false);
    testNoise(true);
    testNormal();
#endif

    testSuper();
    testShaper1a();
    testShaper1b();
    testShaper1c();
    testShaper2();
    testShaper3();
    testShaper4();
    testShaper5();

    testEV3();
    testCHBdef();
    testFunSaw(true);
#if 0
    testFunSaw(false);
    testFunSin(true);
    testFunSin(false);
    testFunSq();
    testFun();
    testFunNone();
#endif

   // testEvenOrig();
    testEvenSaw();
#if 0
    testEven();
    testEvenEven();
    testEvenSin();
    testEvenSaw();
    testEvenTri();
    testEvenSq();
    testEvenSqSaw();
#endif

#if 0

    testVocalFilter();
    testAnimator();
    testShifter();

    testColors();
    testTremolo();
   
    testLFN();
    testGMR();
#endif
   

   // test1();
#if 0
    testHilbert<float>();
    testHilbert<double>();
#endif
}