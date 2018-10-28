#include <assert.h>

#if !defined(M_PI)
#define M_PI 3.14159265358979323846264338327950288
#endif

#include "asserts.h"
#include "EV3.h"



#include "MinBLEPVCO.h"
#include "TestComposite.h"

static float sampleTime = 1.0f / 44100.0f;


class TestMB
{
public:
    //static void  setAllWaveforms(MinBLEPVCO* vco);
   // static void test1();
    static void testSync2();
    static void testSync3();
    static void setPitch(EV3<TestComposite>& ev3);
};

#if 0
// puts non-zero in all the waveforms
 void TestMB::setAllWaveforms(MinBLEPVCO* vco)
{
  //  float * wave = vco->_getWaveforms();
    for (int i = 0; i < (int)MinBLEPVCO::Waveform::END; ++i) {
        vco->waveformOutputs[i] = 1;
    }
}
#endif

#if 0
void TestMB::test1()
{
    MinBLEPVCO vco;
    setAllWaveforms(&vco);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Saw), 0);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Sin), 0);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Square), 0);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Tri), 0);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Even), 0);

 
    vco.zeroOutputsExcept(MinBLEPVCO::Waveform::Saw);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Saw), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Sin), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Square), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Tri), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Even), 0);

    // special case for even and sin
    setAllWaveforms(&vco);
    vco.zeroOutputsExcept(MinBLEPVCO::Waveform::Even);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Saw), 0);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Sin), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Square), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Tri), 0);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Even), 0);

    setAllWaveforms(&vco);
    vco.zeroOutputsExcept(MinBLEPVCO::Waveform::Square);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Saw), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Sin), 0);
    assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Square), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Tri), 0);
    assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Even), 0);
}


static void test0()
{
    MinBLEPVCO vco;

    // Don't enable any waveforms
   // vco.setSampleTime(sampleTime);
    vco.setNormalizedFreq(1000 * sampleTime);
    vco.step();

    // should get nothing out.
    assert(vco.getWaveform(MinBLEPVCO::Waveform::Sin) == 0);
    assert(vco.getWaveform(MinBLEPVCO::Waveform::Square) == 0);
    assert(vco.getWaveform(MinBLEPVCO::Waveform::Saw) == 0);
    assert(vco.getWaveform(MinBLEPVCO::Waveform::Tri) == 0);
    assert(vco.getWaveform(MinBLEPVCO::Waveform::Even) == 0);
}
#endif


static void testSaw1()
{
    MinBLEPVCO vco;

    vco.setNormalizedFreq(1000 * sampleTime, sampleTime);
    vco.setWaveform(MinBLEPVCO::Waveform::Saw);
    vco.step();

    // should get saw out.
  //  assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Sin), 0);
   // assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Square), 0);
  //  assertNE(vco.getWaveform(MinBLEPVCO::Waveform::Saw), 0);
   // assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Tri), 0);
   // assertEQ(vco.getWaveform(MinBLEPVCO::Waveform::Even), 0);
    assertNE(vco.getOutput(), 0);

}

static void testSync1()
{
    MinBLEPVCO vco;

    vco.setNormalizedFreq(1000 * sampleTime, sampleTime);
    vco.setWaveform(MinBLEPVCO::Waveform::Saw);
    float lastOut = -1000;
    vco.step();

    // first make sure it's going up.
    for (int i = 0; i < 10; ++i) {
        vco.step();
        const float x = vco.getOutput();
        assertGT(x, lastOut);
        lastOut = x;
    }

    vco.onMasterSync(10);       // set a reset to VCO
    vco.step();
    const float x = vco.getOutput();
    assertLT(x, lastOut);
}

void TestMB::setPitch(EV3<TestComposite>& ev3)
{
    ev3.params[EV3<TestComposite>::OCTAVE1_PARAM].value = 2;
    ev3.params[EV3<TestComposite>::OCTAVE2_PARAM].value = 3;
    ev3.params[EV3<TestComposite>::OCTAVE3_PARAM].value = 3;

    // raise 2,3 by an oct and a semitone
    ev3.params[EV3<TestComposite>::SEMI1_PARAM].value = 0;
    ev3.params[EV3<TestComposite>::SEMI2_PARAM].value = 1;
    ev3.params[EV3<TestComposite>::SEMI3_PARAM].value = 1;

    ev3.vcos[0].setWaveform(MinBLEPVCO::Waveform::Saw);
    ev3.vcos[1].setWaveform(MinBLEPVCO::Waveform::Saw);
    ev3.vcos[2].setWaveform(MinBLEPVCO::Waveform::Saw);

    ev3.params[EV3<TestComposite>::SYNC2_PARAM].value = 1;

    ev3.vcos[0].name = "VCO1";
    ev3.vcos[1].name = "VCO2";
    ev3.vcos[2].name = "VCO3";

}

void TestMB::testSync2()
{
    printf("***** testSync2*****\n");
    EV3<TestComposite> ev3;
    setPitch(ev3);

    ev3.step();
    const float f0 = ev3._freq[0];
    const float f1 = ev3._freq[1];
    
    assertClose(f0, 2093.02, .005);
    assertClose(f1, 4434.95, .005);

    float last0 = -10;
    float last1 = -10;
    for (int i = 0; i < 100; ++i) {
        ev3.step();
 
        //printf("phase==%.2f phase1==%.2f ", ev3.vcos[0].phase, ev3.vcos[1].phase);
        float x = ev3._out[0];
       // assert(x > last0);
        //printf("%d delta0=%.2f",i, x - last0);
        last0 = x;

        x = ev3._out[1];
       // assert(x > last1);
       
        printf(" delta1=%.2f", x - last1);
        printf(" 0=%.2f 1=%.2f\n", last0, last1);
        fflush(stdout);
        last1 = x;
    }

    // TODO: test the sync on/off
  
}



void TestMB::testSync3()
{
    EV3<TestComposite> ev3;
    setPitch(ev3);

    ev3.step();
    const float f0 = ev3._freq[0];
    const float f1 = ev3._freq[1];

    assertClose(f0, 2093.02, .01);
    assertClose(f1, 4434.95, .01);

    float last0 = -10;
    float last1 = -10;
    for (int i = 0; i < 100; ++i) {
      //  printf("-------- sample %d -----------\n", i);
        ev3.step();
    }

    // TODO: test the sync on/off

}


static void testBlepx(float crossing, float jump)
{
    printf("BLEP crossing = %.2f, jump =%.2f\n", crossing, jump);
    rack::MinBLEP<16> syncMinBLEP;

    syncMinBLEP.minblep = rack::minblep_16_32;
    syncMinBLEP.oversample = 32;

   // syncMinBLEP.jump(-.5, -2);
    syncMinBLEP.jump(crossing, jump);
    for (int i = 0; i < 32; ++i) {
    //float saw = -1.0 + 2.0*phase;
        float x = syncMinBLEP.shift();
        printf("blep[%d] = %.2f\n", i, x);
    }

}

static void testEnums()
{
    assertEQ((int) EV3<TestComposite>::Waves::SIN, (int) MinBLEPVCO::Waveform::Sin);
    assertEQ((int) EV3<TestComposite>::Waves::TRI, (int) MinBLEPVCO::Waveform::Tri);
    assertEQ((int) EV3<TestComposite>::Waves::SAW, (int) MinBLEPVCO::Waveform::Saw);
    assertEQ((int) EV3<TestComposite>::Waves::SQUARE, (int) MinBLEPVCO::Waveform::Square);
    assertEQ((int) EV3<TestComposite>::Waves::EVEN, (int) MinBLEPVCO::Waveform::Even);
}

static void testOutput(MinBLEPVCO::Waveform wf, bool expectFlat)
{
    MinBLEPVCO osc;

    osc.setWaveform(wf);
    osc.setNormalizedFreq(.1f, 1.0f / 44100);      // high freq

    bool hasChanged = false;
    float last = -100;
    for (int i = 0; i < 100; ++i) {
        osc.step();
        float x = osc.getOutput();
        if (!expectFlat) assertNE(x, last);
        if (last != x) hasChanged = true;
        last = x;
    }
    assert(hasChanged);
}

static void testOutputs()
{
    testOutput(MinBLEPVCO::Waveform::Saw, false);
    testOutput(MinBLEPVCO::Waveform::Square, true);
    testOutput(MinBLEPVCO::Waveform::Sin, false);
    testOutput(MinBLEPVCO::Waveform::Tri, false);
    testOutput(MinBLEPVCO::Waveform::Even, false);
   
}

static void testBlep()
{
    testBlepx(-.5, -2);
    testBlepx(-.5, 1);
    testBlepx(-.9f, .2f);
}

static void testZero()
{
    MinBLEPVCO osc;

    osc.setWaveform(MinBLEPVCO::Waveform::Saw);
    osc.setNormalizedFreq(.1f, 1.0f / 44100);      // high freq
    osc.step();
    osc.setWaveform(MinBLEPVCO::Waveform::END);
    osc.step();
    float x = osc.getOutput();
    assertEQ(x, 0);
}

static void testSyncOut(MinBLEPVCO::Waveform wf)
{
    MinBLEPVCO osc;

    int callbackCount = 0;
    osc.setWaveform(wf);
    osc.setNormalizedFreq(.1f, 1.0f / 44100);      // high freq
    osc.setSyncCallback([&callbackCount](float p) {
        assert(p <= 0 && p >= -1);
        callbackCount++;
    });

    for (int i = 0; i < 15; ++i) {
        osc.step();
       
    }
    assertEQ(callbackCount, 1);
  //  assertGE(callbackCount, 1);     // TODO: why does square do 3??

}

static void testSyncOut()
{
    testSyncOut(MinBLEPVCO::Waveform::Saw);
    testSyncOut(MinBLEPVCO::Waveform::Square);
    testSyncOut(MinBLEPVCO::Waveform::Sin);
    testSyncOut(MinBLEPVCO::Waveform::Tri);
    testSyncOut(MinBLEPVCO::Waveform::Even);
}


static void testSyncIn(MinBLEPVCO::Waveform wf)
{
    MinBLEPVCO osc;

    osc.setWaveform(wf);
    osc.setNormalizedFreq(.1f, 1.0f / 44100);      // high freq

    for (int i = 0; i < 4; ++i) {
        osc.step();
    }
    osc.step();
    float x = osc.getOutput();
    osc.onMasterSync(0);
    osc.step();                 // this one catches the callback
    osc.step();                 // this one generates new sample (TODO: is this extra required?)
    float y = osc.getOutput();
    assert(!AudioMath::closeTo(x, y, .1));

    // TODO: pick freq that will make this more robust

}

static void testSyncIn()
{
    testSyncIn(MinBLEPVCO::Waveform::Saw);
    testSyncIn(MinBLEPVCO::Waveform::Sin);
    testSyncIn(MinBLEPVCO::Waveform::Tri);
    testSyncIn(MinBLEPVCO::Waveform::Square);
    testSyncIn(MinBLEPVCO::Waveform::Even);
}

void testMinBLEPVCO()
{
    // A lot of these tests are from old API
//    TestMB::test1();
  //  printf("fix the minb tests\n");
  //  test0();

    testSaw1();
    //testSync1();

    // this one doesn't work, either.
    //TestMB::testSync2();
    TestMB::testSync3();
  //  testBlep();
    testEnums();
    testOutputs();
    testZero();
    testSyncOut();
    testSyncIn();
}