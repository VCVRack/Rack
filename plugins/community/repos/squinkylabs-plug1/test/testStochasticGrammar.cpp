
#include "GenerativeTriggerGenerator.h"
#include "StochasticGrammar.h"
#include "TriggerSequencer.h"

#include <string>
#include <vector>
#include <set>
#include <random>

static const int numRules = fullRuleTableSize;

typedef GKEY(*INITFN)();
static ProductionRule rules[numRules];



// Test basic integrity of key data
static void test0()
{
    GKEY key;
    GKEY outKeys[ProductionRuleKeys::bufferSize];
    for (key = sg_first; key <= sg_last; ++key) {
        ProductionRuleKeys::breakDown(key, outKeys);
        for (GKEY* p = outKeys; *p != sg_invalid; ++p) {
            assert(*p >= sg_first);
            assert(*p <= sg_last);
        }

        std::string s = ProductionRuleKeys::toString(key);
        assert(!s.empty());
        assert(s.length() < 256);

        int dur = ProductionRuleKeys::getDuration(key);
        assert(dur > 0);
        assert(dur <= PPQ * 8);

    }
}

// test all the gkeys
void testAllKeys()
{
    const int siz = ProductionRuleKeys::bufferSize;
    GKEY buffer[siz];

    for (GKEY gk = sg_first; gk <= sg_last; ++gk) {
       // printf("testing key %d\n", gk);
       // printf("to string: %s\n", ProductionRuleKeys::toString(gk));
        const int dur = ProductionRuleKeys::getDuration(gk);
        ProductionRuleKeys::breakDown(gk, buffer);
        int sum = 0;
        for (GKEY * p = buffer; *p != sg_invalid; ++p) {
           // printf("adding to sum %d\n", ProductionRuleKeys::getDuration(*p));
            sum += ProductionRuleKeys::getDuration(*p);

        }
       // printf("dur = %d sum = %d (should be the same)\n", dur, sum);
        assert(dur == sum);
    }
}



/**************************************************************************************
 * Make some simple grammars and test them
 **************************************************************************************/

#ifdef _DEBUG
void gdt0()
{
    {
        static ProductionRule rules[numRules];
        bool b = ProductionRule::isGrammarValid(rules, numRules, sg_invalid);
        assert(!b);
    }
    {
        // throw in a positive case
        static ProductionRule rules[numRules];
        ProductionRule& r = rules[sg_w];
        r.entries[0].probability = 1.0f;
        r.entries[0].code = sg_invalid;

        bool b = ProductionRule::isGrammarValid(rules, numRules, sg_w);
        assert(b);
    }
    {
        // terminal code wrong
        static ProductionRule rules[numRules];
        ProductionRule& r = rules[sg_w];
        r.entries[0].probability = 1.0f;
        r.entries[0].code = sg_q;

        bool b = ProductionRule::isGrammarValid(rules, numRules, sg_w);
        assert(!b);
    }
    {
        // bad order of probability
        static ProductionRule rules[numRules];
        ProductionRule& r = rules[sg_w];
        r.entries[0].probability = 1.0f;
        r.entries[0].code = sg_q;

        bool b = ProductionRule::isGrammarValid(rules, numRules, sg_w);
        assert(!b);
    }
    {
        // rule branches to nowhere
        static ProductionRule rules[numRules];

        // break w2 into w,w prob 100
        ProductionRule& r = rules[sg_w2];
        r.entries[0].probability = 1.0f;
        r.entries[0].code = sg_ww;
        bool b = ProductionRule::isGrammarValid(rules, numRules, sg_w);
        assert(!b);
    }
}
#endif


class TestEvaluator : public ProductionRule::EvaluationState
{
public:
    TestEvaluator(AudioMath::RandomUniformFunc xr) : ProductionRule::EvaluationState(xr)
    {
    }

    void writeSymbol(GKEY key) override
    {
        keys.push_back(key);
    }

    int getNumSymbols()
    {
        //printf("final keys: ");
       // for (size_t i = 0; i< keys.size(); ++i) printf("%s, ", ProductionRuleKeys::toString(keys[i]));
       // printf("\n");
        return (int) keys.size();
    }
private:
    std::vector<GKEY> keys;
};


/**
 * simplest possible grammar.
 */
static GKEY init0()
{
   // printf("called init0\n");
    // This rule always generate sg-w2 (two whole notes tied together)
    ProductionRule& r = rules[sg_w2];

    r.entries[0].probability = 1;
    r.entries[0].code = sg_invalid;		// terminate expansion		


    return sg_w2;
}

/**
 * Simple grammar with a rule but no random.
 */
static GKEY init1()
{

    {
        // start with w2 duration
        ProductionRule& r = rules[sg_w2];

        // break into w,w prob 100
        r.entries[0].probability = 1.0f;
        r.entries[0].code = sg_ww;
    }

    {
        // now need rule for w hole
        //printf("in init1 making 100 for %d\n", sg_w);
        ProductionRule& r = rules[sg_w];
        r.entries[0].probability = 1.0f;
        r.entries[1].code = sg_invalid;
    }
    //printf("leave init 1. rule 1 p0 = %f\n", rules[sg_w2].entries[0].probability);
    return sg_w2;
}


/**
 * Simple grammar with randomness initializer
 */
static GKEY init2()
{
    {
        // start with w2 duration
        ProductionRule& r = rules[sg_w2];

        // break into w,w prob 50

        r.entries[0].probability = .5f;
        r.entries[0].code = sg_ww;
        r.entries[1].probability = 1.0f;
        r.entries[1].code = sg_invalid;		// always terminate
    }

    {
        // now need rule for w hole
        ProductionRule& r = rules[sg_w];
        r.entries[1].probability = 1.0f;
        r.entries[1].code = sg_invalid;		// always terminate
    }

    return sg_w2;
}

#ifdef _DEBUG
static void testGrammarSub(INITFN f)
{
    GKEY init = f();


    bool b = ProductionRule::isGrammarValid(rules, numRules, init);
    assert(b);

    TestEvaluator es(AudioMath::random());
    es.rules = rules;
    es.numRules = numRules;
    ProductionRule::evaluate(es, init);

    assert(es.getNumSymbols() > 0);
}
#endif


/*********************************************************************************************************
 * TriggerSequencer
 **********************************************************************************************************/

 // test event at zero fires at zero
static void ts0()
{
    TriggerSequencer::Event seq[] =
    {
        {TriggerSequencer::TRIGGER, 0},
    {TriggerSequencer::END, 100}
    };
    TriggerSequencer ts(seq);

    ts.clock();
    assert(ts.getTrigger());

    ts.clock();
    assert(!ts.getTrigger());

}

// test trigger at 1 happens at 1
static void ts1()
{
    TriggerSequencer::Event seq[] =
    {
        {TriggerSequencer::TRIGGER, 1},
    {TriggerSequencer::END, 0}
    };
    TriggerSequencer ts(seq);

    ts.clock();
    assert(!ts.getTrigger());

    ts.clock();
    assert(ts.getTrigger());

    ts.clock();
    assert(!ts.getTrigger());

    ts.clock();
    assert(!ts.getTrigger());
}


// 4 clock loop: delay 4, trigger, end
static void ts2()
{
    TriggerSequencer::Event seq[] =
    {
        {TriggerSequencer::TRIGGER, 4},
    {TriggerSequencer::END, 0}
    };
    TriggerSequencer ts(seq);

    bool firstTime = true;
    // first time through, 4 clocks of nothing. then clock, 0,0,0
    for (int i = 0; i < 4; ++i) {
        ts.clock();
        if (firstTime) {
            assert(!ts.getTrigger()); assert(!ts.getEnd());
            firstTime = false;
        } else {
            //printf("second time around, t=%d e=%d\n", ts.getTrigger(), ts.getEnd());

            // second time around we finally see the trigger

            assert(ts.getTrigger());

            // second time around, need to clock the end of the last time
            assert(ts.getEnd());
            ts.reset(seq);				// start it up again
            assert(!ts.getTrigger());	// resetting should not set us up for a trigger
        }
        ts.clock(); assert(!ts.getTrigger()); assert(!ts.getEnd());
        ts.clock(); assert(!ts.getTrigger()); assert(!ts.getEnd());

        ts.clock(); assert(!ts.getTrigger());
        //	assert(ts.getEnd());

        //	ts.reset(seq);
    }
}

// test trigger seq qith
// 4 clock loop: trigger, delay 4 end
static void ts3()
{
    TriggerSequencer::Event seq[] =
    {
        {TriggerSequencer::TRIGGER, 0},
    {TriggerSequencer::END, 4}
    };
    TriggerSequencer ts(seq);


    bool firstLoop = true;
    for (int i = 0; i < 4; ++i) {
        //printf("--- loop ----\n");

        // 1

        ts.clock();
        if (firstLoop) {
            assert(ts.getTrigger());
            // we just primed loop at top, so it's got a ways
            assert(!ts.getEnd());
            firstLoop = false;
        } else {
            // second time around, need to clock the end of the last time
            assert(ts.getEnd());
            ts.reset(seq);				// start it up again
            assert(ts.getTrigger());	// resetting should have set us up for a trigger
        }
        // 2
        ts.clock(); assert(!ts.getTrigger()); assert(!ts.getEnd());
        // 3
        ts.clock(); assert(!ts.getTrigger()); assert(!ts.getEnd());
        // 4
        ts.clock();
        assert(!ts.getTrigger());
        assert(!ts.getEnd());
    }
}

// test trigger seq with straight ahead 4/4 as generated by a grammar
static void ts4()
{
    TriggerSequencer::Event seq[] =
    {
        {TriggerSequencer::TRIGGER, 0},
    {TriggerSequencer::TRIGGER, 4},
    {TriggerSequencer::TRIGGER, 4},
    {TriggerSequencer::TRIGGER, 4},
    {TriggerSequencer::END, 4}
    };
    TriggerSequencer ts(seq);


    //bool firstLoop = true;
    for (int i = 0; i < 100; ++i) {
        bool firstTime = (i == 0);
        // repeating pattern of trigg, no, no, no
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                //	printf("test loop, i=%d, j=%d, k=%d\n", i, j, k);
                ts.clock();

                bool expectEnd = (k == 0) && (j == 0) && !firstTime;
                assert(ts.getEnd() == expectEnd);
                if (ts.getEnd()) {
                    ts.reset(seq);
                }
                assert(ts.getTrigger() == (k == 0));
            }
        }
    }
}

/*******************************************************************************
 ** StochasticGrammarDictionary
 */

#ifdef _DEBUG
void gdt1()
{
    assert(StochasticGrammarDictionary::getNumGrammars() > 0);
    for (int i = 0; i < StochasticGrammarDictionary::getNumGrammars(); ++i) {
        StochasticGrammarDictionary::Grammar g = StochasticGrammarDictionary::getGrammar(i);
        bool b = ProductionRule::isGrammarValid(g.rules, g.numRules, g.firstRule);
        assert(b);
    }
}
#endif

/********************************************************************************************
* GenerativeTriggerGenerator
**********************************************************************************************/

// test that we get some clocks and some not
static void gtg0()
{
    GKEY key = init1();
    GenerativeTriggerGenerator gtg(AudioMath::random(), rules, numRules, key);
    bool yes = false;
    bool no = false;
    for (int i = 0; i < 100000; ++i) {
        if (gtg.clock())
            yes = true;
        else
            no = true;

        if (yes && no) {
            //printf("clocked at %d\n", i);
            return;
        }
    }
    assert(false);

}


// test that we get everything in even quarter notes
static void gtg1()
{
    GKEY key = init1();
    std::set<int> counts;

    GenerativeTriggerGenerator gtg(AudioMath::random(), rules, numRules, key);

    int ct = 0;
    for (int i = 0; i < 10000; ++i) {
        bool b = gtg.clock();
        if (b) {
            //printf("clocked at %d\n", ct);
            counts.insert(ct);
            ct = 0;
        }
        ct++;
    }
    //counts.insert(50);
    assert(!counts.empty());
    for (std::set<int>::iterator it = counts.begin(); it != counts.end(); ++it) {
        int c = *it;

        if ((c % PPQ) != 0) {
            //printf("PPQ=%d, c modePPQ =%d\n", PPQ, (c % PPQ));
            //printf("2ppq = %d, 4ppq=%d\n", 2 * PPQ, 4 * PPQ);
            assert(false);
        }
    }
}






void testStochasticGrammar()
{
    test0();
    testAllKeys();

#ifdef _DEBUG
    gdt0();
    testGrammarSub(init0);
    testGrammarSub(init1);
    testGrammarSub(init2);
#endif
    ts0();
    ts1();
    ts2();
    ts3();
    ts4();

#ifdef _DEBUG
    gdt1();
#endif

    gtg0();
    gtg1();

}