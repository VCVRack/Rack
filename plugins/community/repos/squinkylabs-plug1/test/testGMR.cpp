
#include "asserts.h"
#include "GMR.h"
#include "TestComposite.h"

#include <set>


using G = GMR<TestComposite>;


// test that we get triggers out
static void test0()
{
    G gmr;
    std::set<float> data;

    gmr.setSampleRate(44100);
    gmr.init();

    for (int i = 0; i < 10; ++i) {

        gmr.inputs[G::CLOCK_INPUT].value = 0;
        for (int i = 0; i < 100; ++i) {
            gmr.step();
            float out = gmr.outputs[G::TRIGGER_OUTPUT].value;
            data.insert(out);
        }
        gmr.inputs[G::CLOCK_INPUT].value = 10;
        for (int i = 0; i < 100; ++i) {
            gmr.step();
            float out = gmr.outputs[G::TRIGGER_OUTPUT].value;
            data.insert(out);
        }
    }

    assert(data.find(cGateOutHi) != data.end());
    assert(data.find(cGateOutLow) != data.end());
    assertEQ(data.size(), 2);
}

void testGMR()
{
    test0();
}