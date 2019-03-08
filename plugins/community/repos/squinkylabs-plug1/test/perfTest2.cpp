
#include "LookupTable.h"
#include "MultiLag.h"
#include "ObjectCache.h"

#include "MeasureTime.h"

extern double overheadOutOnly;
extern double overheadInOut;

static void testMultiLPF()
{
    MultiLPF<8> lpf;

    lpf.setCutoff(.01f);
    float input[8];

    MeasureTime<float>::run(overheadInOut, "multi lpf", [&lpf, &input]() {
        float x = TestBuffers<float>::get();
        for (int i = 0; i < 8; ++i) {
            input[i] = x;
        }
        lpf.step(input);

        return lpf.get(3);
        }, 1);
}

static void testMultiLag()
{
    MultiLag<8> lpf;

    lpf.setAttack(.01f);
    lpf.setRelease(.02f);
    float input[8];

    MeasureTime<float>::run(overheadInOut, "multi lag", [&lpf, &input]() {
        float x = TestBuffers<float>::get();
        for (int i = 0; i < 8; ++i) {
            input[i] = x;
        }
        lpf.step(input);

        return lpf.get(3);
        }, 1);
}


static void testMultiLPFMod()
{
    MultiLPF<8> lpf;

    lpf.setCutoff(.01f);
    float input[8];

    MeasureTime<float>::run(overheadInOut, "multi lpf mod", [&lpf, &input]() {

        float x = TestBuffers<float>::get();
        float y = TestBuffers<float>::get();
        y = std::abs(y) + .1f;
        while (y > .49f) {
            y -= .48f;
        }
        assert(y > 0);
        assert(y < .5);

        lpf.setCutoff(y);
        for (int i = 0; i < 8; ++i) {
            input[i] = x;
        }
        lpf.step(input);

        return lpf.get(3);
        }, 1);
}


static void testMultiLagMod()
{
    MultiLag<8> lag;

    lag.setAttack(.01f);
    lag.setRelease(.02f);
    float input[8];

    MeasureTime<float>::run(overheadInOut, "multi lag mod", [&lag, &input]() {

        float x = TestBuffers<float>::get();
        float y = TestBuffers<float>::get();
        y = std::abs(y) + .1f;
        while (y > .49f) {
            y -= .48f;
        }
        assert(y > 0);
        assert(y < .5);

        lag.setAttack(y);
        lag.setRelease(y/2);
        for (int i = 0; i < 8; ++i) {
            input[i] = x;
        }
        lag.step(input);

        return lag.get(3);
        }, 1);
}

static void testUniformLookup()
{
    std::shared_ptr<LookupTableParams<float>> lookup = ObjectCache<float>::getSinLookup();
    MeasureTime<float>::run(overheadInOut, "uniform", [lookup]() {
        float x = TestBuffers<float>::get();
        return LookupTable<float>::lookup(*lookup, x, true);

    }, 1);
}
static void testNonUniform()
{
    std::shared_ptr<NonUniformLookupTableParams<float>> lookup = makeLPFilterL_Lookup<float>();

    MeasureTime<float>::run(overheadInOut, "non-uniform", [lookup]() {
        float x = TestBuffers<float>::get();
        return  NonUniformLookupTable<float>::lookup(*lookup, x);
     
    }, 1);
}

void perfTest2()
{
    testUniformLookup();
    testNonUniform();
    testMultiLPF();
    testMultiLPFMod();
    testMultiLag();
    testMultiLagMod();
}