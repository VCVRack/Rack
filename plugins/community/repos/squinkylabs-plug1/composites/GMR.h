
#pragma once
#include "ObjectCache.h"
#include "GenerativeTriggerGenerator.h"
#include "TriggerOutput.h"

#include <memory>

/**
 */
template <class TBase>
class GMR : public TBase
{
public:
    GMR(struct Module * module) : TBase(module), inputClockProcessing(true)
    {
    }
    GMR() : TBase(), inputClockProcessing(true)
    {
    }
    void setSampleRate(float rate)
    {
        reciprocalSampleRate = 1 / rate;
    }

    // must be called after setSampleRate
    void init();

    enum ParamIds
    {
        NUM_PARAMS
    };

    enum InputIds
    {
        CLOCK_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        TRIGGER_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

private:
    float reciprocalSampleRate = 0;
    std::shared_ptr<GenerativeTriggerGenerator> gtg;
    GateTrigger inputClockProcessing;
    TriggerOutput outputProcessing;
};



template <class TBase>
inline void GMR<TBase>::init()
{
    StochasticGrammarDictionary::Grammar grammar = StochasticGrammarDictionary::getGrammar(0);
    gtg = std::make_shared<GenerativeTriggerGenerator>(
        AudioMath::random(),
        grammar.rules,
        grammar.numRules,
        grammar.firstRule);
}

template <class TBase>
inline void GMR<TBase>::step()
{
    bool outClock = false;
    float inClock = TBase::inputs[CLOCK_INPUT].value;
    inputClockProcessing.go(inClock);
    if (inputClockProcessing.trigger()) {
        outClock = gtg->clock();
    }
    outputProcessing.go(outClock);
    TBase::outputs[TRIGGER_OUTPUT].value = outputProcessing.get();
}

