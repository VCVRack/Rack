
#pragma once

#include <assert.h>
#include <stdio.h>
#include <functional>
#include "AudioMath.h"

/***********************************************
 ********** rhythmic grouping codes *************
************************************************/

typedef unsigned short GKEY;

/* Rules for keys - super important!
 *
 * There are two kinds of keys: terminal keys and non-terminal
 *
 * Terminal keys either directly generate themselves, or may be divided if there is a
 * specific production rule to divide them. As an example, sg_q (quarter note) is a terminal key.
 * It will always generate itself unless a production rule divides it.
 *
 * On the other hand sg_w2 is a non-terminal representing all the time in two bars of 4/4.
 * sg_w2 will NEVER generate itself. Lacking a specific rule, it will auto generate two whole notes
 *
 * programmer must be aware of the difference in two places:
 *		when making production rules for a specific grammar
 *		when implementing ProductionRuleKeys::breakDown
 */


const GKEY sg_invalid = 0;		// either uninitialized rule, or return value that stops recursion.
                                // Note that this means table of production rules must have a dummy entry up front
const GKEY sg_w2 = 1;		    // duration of two whole notes
const GKEY sg_w = 2;		    // whole
const GKEY sg_ww = 3;		    // w,w
const GKEY sg_h = 4;
const GKEY sg_hh = 5;
const GKEY sg_q = 6;
const GKEY sg_qq = 7;
const GKEY sg_e = 8;
const GKEY sg_ee = 9;

// triplets
const GKEY sg_e3e3e3 = 10;		// three trip eights
const GKEY sg_e3 = 11;			//  trip eight

const GKEY sg_sx = 12;
const GKEY sg_sxsx = 13;

// crazy stuff for syncopation (unequal measure divisions)
// Note that there are not "tuples", they are just straight durations
// of a group of notes.
const GKEY sg_68 = 14;		// the time duration of 6/8
const GKEY sg_78 = 15;		// the time duration of 7/8
const GKEY sg_98 = 16;		// the time duration of 9/8
const GKEY sg_798 = 17;		// 7/8 + 9/8 = 2w

// dotted notes
const GKEY sg_dq = 18;		// dotted quarter
const GKEY sg_dh = 19;		// dotted half
const GKEY sg_de = 20;		// dotted eighth

// odd groupings
const GKEY sg_hdq = 21;		// half + dotted Q
const GKEY sg_qhe = 22;		// q,h,e
const GKEY sg_hq = 23;	    // h,q
const GKEY sg_qh = 24;	    // h,q
const GKEY sg_q78 = 25;		// q + 7x8
const GKEY sg_qe68 = 26;	// q+e+6x8

const GKEY sg_first = 1;		// first valid one
const GKEY sg_last = 26;

const int fullRuleTableSize = sg_last + 1;

// Do we really want to use something this coarse?
const int PPQ = 24;

/* class ProductionRuleKeys
 * collection of utility functions around rule keys
 */
class ProductionRuleKeys
{
public:
    static const int bufferSize = 6;	// size of a buffer that must be passed to breakDown

    /**
     * Turn a key into a 0 terminated list of keys for individual notes.
     * If called with a terminal key, just returns itself.
     */
    static void breakDown(GKEY key, GKEY * outKeys);

    /**
     * get the duration in clocks for a key
     */
    static int getDuration(GKEY key);

    /**
     * Get a human readable string representation
     */
    static const char * toString(GKEY key);
};


/* class ProductionRuleEntry
 * A single entry in a production rule.
 * if A -> B or A -> C, then each of these would be a separate rule entry
 */
class ProductionRuleEntry
{
public:
    ProductionRuleEntry() : probability(0), code(sg_invalid)
    {
    }
    float probability;	// 0 to 1
    GKEY code;			// what to do if this one fires
};

inline bool operator == (const ProductionRuleEntry& a, const ProductionRuleEntry& b)
{
    return a.probability == b.probability && a.code == b.code;
}

/* class ProductionRule
 * A production rule encapsulates every way that a starting symbol
 * can produce others.
 * if A -> B or A -> C, then a single production rule could represent this
 *
 */
class ProductionRule
{
public:
    static const int numEntries = 3;
    class EvaluationState
    {
    public:
        EvaluationState(AudioMath::RandomUniformFunc xr) : r(xr)
        {
        }
        const ProductionRule * rules;
        int numRules;
        AudioMath::RandomUniformFunc r;		//random number generator to use 
        virtual void writeSymbol(GKEY)
        {
        }
    };

    ProductionRule()
    {
    }

    void makeTerminal()
    {
        entries[0].code = sg_invalid;
        entries[0].probability = 1.0f;
    }

    /* the data */

    // each possible production rule for this state
    ProductionRuleEntry entries[numEntries];

    static void evaluate(EvaluationState& es, int ruleToEval);
#ifdef _DEBUG
    static bool isGrammarValid(const ProductionRule * rules, int numRules, GKEY firstRule);
#endif
private:
    static int _evaluateRule(const ProductionRule& rule, float random);
#ifdef _DEBUG
    bool _isValid(int index) const;
#endif
};


 
/* class StochasticGrammarDictionary
 *
 * just a collection of pre-made grammars
 *
 * 0: simple test
 * 1: mixed duration, with some trips
 * 2: some syncopation, no trips
 */
class StochasticGrammarDictionary
{
public:
    class Grammar
    {
    public:
        const ProductionRule * rules;
        int numRules;
        GKEY firstRule;
    };
    static Grammar getGrammar(int index);
    static int getNumGrammars();
private:
    static bool _didInitRules;
    static void initRules();
    static void initRule0(ProductionRule * rules);
    static void initRule1(ProductionRule * rules);
    static void initRule2(ProductionRule * rules);
    static void initRule3(ProductionRule * rules);
};

