#include "asserts.h"


#include "ExtremeTester.h"
#include "VocalAnimator.h"
#include "TestComposite.h"
#include "VocalFilter.h"
#include "FormantTables2.h"

using Animator = VocalAnimator<TestComposite>;

/**
 * Verify no output with no input.
 */
static void test0()
{
    Animator anim;
    anim.setSampleRate(44100);
    anim.init();

    anim.outputs[Animator::AUDIO_OUTPUT].value = 0;
    anim.step();                // prime it

                                // with no input, should have no output
    for (int i = 0; i < 50; ++i) {
        anim.step();
        assert(anim.outputs[Animator::AUDIO_OUTPUT].value == 0);
    }
}

/**
 * Verify output with input.
 */
static void test1()
{
    Animator anim;
    anim.setSampleRate(44100);
    anim.init();

    anim.outputs[Animator::AUDIO_OUTPUT].value = 0;
    anim.inputs[Animator::AUDIO_INPUT].value = 1;
    anim.step();                // prime it
                                // with  input, should have  output
    for (int i = 0; i < 50; ++i) {
        anim.step();
        assert(anim.outputs[Animator::AUDIO_OUTPUT].value != 0);
    }
}

/**
 * Verify filter settings with no mod.
 */
static void test2()
{
    Animator anim;
    anim.setSampleRate(44100);
    anim.init();
    for (int i = 0; i < 4; ++i) {
        float freq = anim.normalizedFilterFreq[i] * 44100;
        assertEQ(freq, anim.nominalFilterCenterHz[i]);
    }
}

/**
* Verify filter settings respond to Fc.
*/
static void test3()
{
    Animator anim;
    anim.setSampleRate(44100);
    anim.init();
    anim.params[anim.FILTER_FC_PARAM].value = 0;
    anim.step();

    for (int i = 0; i < 4; ++i) {
       // assert(anim.filterFrequency[i] == anim.nominalFilterCenter[i]);
        float freq = anim.normalizedFilterFreq[i] * 44100;
        assertClose(freq, anim.nominalFilterCenterHz[i], 1);
    }

    anim.params[anim.FILTER_FC_PARAM].value = 1;
    anim.step();


    // assert that when we shift up, the expected values shift up
    for (int i = 0; i < 4; ++i) {
        float freq = anim.normalizedFilterFreq[i] * 44100;
        //printf("i=%d, freq=%f, nominal=%f\n", i, freq, anim.nominalFilterCenterHz[i]);
        if (i == 3) {
            assertClose(freq, anim.nominalFilterCenterHz[i], 1);
        } else
            assert(freq > anim.nominalFilterCenterHz[i]);
    }

#if 0
    anim.params[anim.FILTER_FC_PARAM].value = -1;
    anim.step();
    for (int i = 0; i < 4; ++i) {
        if (i == 3)
            assert(anim.filterFrequency[i] == anim.nominalFilterCenter[i]);
        else
            assert(anim.filterFrequency[i] < anim.nominalFilterCenter[i]);
    }
#endif
}

static void testScalers()
{
    Animator anim;
    anim.setSampleRate(44100);
    anim.init();

    // cv/knob, trim


    // cases with no CV
    assertClose(.5, anim.scale0_1(0, 0, 1), .001);              // knob half, full trim
    assertClose(.5, anim.scale0_1(0, 0, -1), .001);             // knob half, full neg trim
    assertClose(1, anim.scale0_1(0, 5, 0), .001);               // knob full
    assertClose(0, anim.scale0_1(0, -5, 0), .001);              // knob down full
    assertClose(.75, anim.scale0_1(0, (5.0f * .5f), 0), .001); // knob 3/4

    // CV, no knob
    assertClose(1, anim.scale0_1(5, 0, 1), .001);              // full cv, untrimmed
    assertClose(0, anim.scale0_1(-5, 0, 1), .001);              // full cv, untrimmed
    assertClose(.25, anim.scale0_1((-5.0f * .5f), 0, 1), .001);       // 3/4 cv, untrimmed

   // assertClose(.75, anim.scale0_1(5, 0, .5f), .001);           // full cv, half trim
    assertClose(0, anim.scale0_1(5, 0, -1), .001);              // full cv, full neg trim

}


#if 0
static void dump(const char * msg, const Animator& anim)
{
    std::cout << "dumping " << msg << "\nfiltFreq"
        << " " << std::pow(2, anim.filterFrequencyLog[0])
        << " " << std::pow(2, anim.filterFrequencyLog[1])
        << " " << std::pow(2, anim.filterFrequencyLog[2])
        << " " << std::pow(2, anim.filterFrequencyLog[3])
        << std::endl;
}

static void x()
{
    Animator anim;
    anim.setSampleRate(44100);
    anim.init();
    anim.params[anim.FILTER_FC_PARAM].value = 0;
    anim.step();

    dump("init", anim);

    // TODO: assert here
    anim.params[anim.FILTER_FC_PARAM].value = 5;
    anim.step();
    dump("fc 5", anim);

    anim.params[anim.FILTER_FC_PARAM].value = -5;
    anim.step();
    dump("fc -5", anim);

    std::cout << "\nabout to modulate up. maxLFO, def depth\n";
    anim.params[anim.FILTER_FC_PARAM].value = 0;
    anim.params[anim.FILTER_MOD_DEPTH_PARAM].value = 0;
    anim.jamModForTest = true;
    anim.modValueForTest = 5;
    anim.step();
    dump("max up def", anim);

    std::cout << "\nabout to modulate up. minLFO, def depth\n";
    anim.params[anim.FILTER_FC_PARAM].value = 0;
    anim.params[anim.FILTER_MOD_DEPTH_PARAM].value = 0;
    anim.jamModForTest = true;
    anim.modValueForTest = -5;
    anim.step();
    dump("max down def", anim);

    std::cout << "\nabout to modulate up. maxLFO, max depth\n";
    anim.params[anim.FILTER_FC_PARAM].value = 0;
    anim.params[anim.FILTER_MOD_DEPTH_PARAM].value = 5;
    anim.jamModForTest = true;
    anim.modValueForTest = 5;
    anim.step();
    dump(" modulate up. maxLFO, max depthf", anim);


    std::cout << "\nabout to modulate down. minLFO, max depth\n";
    anim.params[anim.FILTER_FC_PARAM].value = 0;
    anim.params[anim.FILTER_MOD_DEPTH_PARAM].value = 5;
    anim.jamModForTest = true;
    anim.modValueForTest = -5;
    anim.step();
    dump(" modulate up. maxLFO, max depthf", anim);


#if 0
    // TODO: would be nice to be able to inject an LFO voltage
    anim.params[anim.FILTER_FC_PARAM].value = 0;
    anim.params[anim.FILTER_MOD_DEPTH_PARAM].value = 5;
    for (int i = 0; i < 40000; ++i) {
        anim.step();
    }
    dump("fc 0 depth 1", anim);

    std::cout << "about to to depth -\n";
    // TODO: would be nice to be able to inject an LFO voltage
    anim.params[anim.FILTER_FC_PARAM].value = 0;
    anim.params[anim.FILTER_MOD_DEPTH_PARAM].value = -5;
    for (int i = 0; i < 4000; ++i) {
        anim.step();
    }
    dump("fc 0 depth -5", anim);
#endif
}
#endif

/**
*Interpolates the frequency using lookups
* @param model = 0(bass) 1(tenor) 2(countertenor) 3(alto) 4(soprano)
* @param index = 0..4 (formant F1..F5)
* @param vowel is the continuous index into the per / vowel lookup tables(0..4)
* 0 = a, 1 = e, 2 = i, 3 = o 4 = u
*/
//float getLogFrequency(int model, int index, float vowel)
static void testFormantTables()
{
    FormantTables2 ff;
    float x = ff.getLogFrequency(0, 0, 0);
    assert(x > 0);

    x = ff.getNormalizedBandwidth(0, 0, 0);
    assert(x > 0);

    x = ff.getGain(0, 0, 0);
#if 1 // store DB, not gain
    assert(x <= 0);
    assert(x >= -62);
#else
    assert(x > 0)
#endif

    // spot check a few freq
    // formant F2 of alto, 'u' 
        x = ff.getLogFrequency(3, 1, 4);
    assertClose(x, std::log2(700), .0001);
    // formant F3 of soprano, 'o' 
    x = ff.getLogFrequency(4, 2, 3);
    assertClose(x, std::log2(2830), .0001);
}

static void testFormantTables2()
{
    FormantTables2 ff;
    for (int model = 0; model < FormantTables2::numModels; ++model) {
        for (int formantBand = 0; formantBand < FormantTables2::numFormantBands; ++formantBand) {
            for (int vowel = 0; vowel < FormantTables2::numVowels; ++vowel) {
                const float f = ff.getLogFrequency(model, formantBand, float(vowel));

                // check that the frequencies are possible formants
                assert(std::pow(2, f) > 100);
                assert(std::pow(2, f) < 5500);

                const float nBw = ff.getNormalizedBandwidth(model, formantBand, float(vowel));
                assert(nBw < .5);
                assert(nBw > .01);

                // db now
                const float gain = ff.getGain(model, formantBand, float(vowel));
                assertLE(gain, 0);
                assertGT(gain, -70);
               // assertLE(gain, 1);
              //  assert(gain > 0);
            }
        }
    }
}


static void testVocalFilter()
{
    VocalFilter<TestComposite> vf;
    vf.setSampleRate(44100);
    vf.init();

    vf.outputs[VocalFilter<TestComposite>::AUDIO_OUTPUT].value = 0;
    vf.inputs[VocalFilter<TestComposite>::AUDIO_INPUT].value = 1;
    vf.step();                // prime it
                                // with  input, should have  output
    for (int i = 0; i < 50; ++i) {
        vf.step();
        assert(vf.outputs[VocalFilter<TestComposite>::AUDIO_OUTPUT].value != 0);
    }
}

static void testInputExtremes()
{
    VocalAnimator<TestComposite> va;
    va.setSampleRate(44100);
    va.init();

    using fp = std::pair<float, float>;
    std::vector< std::pair<float, float> > paramLimits;

    paramLimits.resize(va.NUM_PARAMS);
    paramLimits[va.LFO_RATE_PARAM] = fp(-5.0f, 5.0f);
  //  paramLimits[va.LFO_SPREAD_PARAM] = fp(-5.0f, 5.0f);
    paramLimits[va.FILTER_FC_PARAM] = fp(-5.0f, 5.0f);
    paramLimits[va.FILTER_Q_PARAM] = fp(-5.0f, 5.0f);
    paramLimits[va.FILTER_MOD_DEPTH_PARAM] = fp(-5.0f, 5.0f);


    paramLimits[va.LFO_RATE_TRIM_PARAM] = fp(-1.0f, 1.0f);
    paramLimits[va.FILTER_Q_TRIM_PARAM] = fp(-1.0f, 1.0f);
    paramLimits[va.FILTER_FC_TRIM_PARAM] = fp(-1.0f, 1.0f);
    paramLimits[va.FILTER_MOD_DEPTH_TRIM_PARAM] = fp(-1.0f, 1.0f);

    paramLimits[va.BASS_EXP_PARAM] = fp(0.f, 1.0f);
    paramLimits[va.TRACK_EXP_PARAM] = fp(0.f, 2.0f);
    paramLimits[va.LFO_MIX_PARAM] = fp(0.f, 1.0f);

    // TODO: why is output going so high?
    ExtremeTester< VocalAnimator<TestComposite>>::test(va, paramLimits, false, "vocal animator");
}


static void testVocalExtremes()
{

    VocalFilter<TestComposite> va;
    va.setSampleRate(44100);
    va.init();

    using fp = std::pair<float, float>;
    std::vector< std::pair<float, float> > paramLimits;

    paramLimits.resize(va.NUM_PARAMS);

    paramLimits[va.FILTER_Q_PARAM] = fp(-5.0f, 5.0f);
    paramLimits[va.FILTER_Q_TRIM_PARAM] = fp(-1.0f, 1.0f);
    paramLimits[va.FILTER_FC_PARAM] = fp(-5.0f, 5.0f);

    paramLimits[va.FILTER_FC_TRIM_PARAM] = fp(-1.0f, 1.0f);
    paramLimits[va.FILTER_VOWEL_PARAM] = fp(-5.f, 5.0f);
    paramLimits[va.FILTER_VOWEL_TRIM_PARAM] = fp(-1.f, 1.0f);
    paramLimits[va.FILTER_MODEL_SELECT_PARAM] = fp(0.f, 4.0f);

    paramLimits[va.FILTER_BRIGHTNESS_PARAM] = fp(-5.f, 5.0f);
    paramLimits[va.FILTER_BRIGHTNESS_TRIM_PARAM] = fp(-1.0f, 1.0f);

    ExtremeTester< VocalFilter<TestComposite>>::test(va, paramLimits, false, "vocal filter");

}
void testVocalAnimator()
{
    test0();
    test1();
    test2();
    test3();
    testScalers();
    testFormantTables();
    testFormantTables2();

    testVocalFilter();
#if defined(_DEBUG) && true
    printf("skipping extremes\n");
#else
    testVocalExtremes();
    testInputExtremes();
#endif

}