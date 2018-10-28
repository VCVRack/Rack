
#pragma once


template <class TBase>
class Blank : public TBase
{
public:

    Blank(struct Module * module) : TBase(module)
    {
    }
    Blank() : TBase()
    {
    }

    /**
    * re-calc everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
    void init();

    enum ParamIds
    {
        NUM_PARAMS
    };

    enum InputIds
    {
        NUM_INPUTS
    };

    enum OutputIds
    {
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

};


template <class TBase>
inline void Blank<TBase>::init()
{
}


template <class TBase>
inline void Blank<TBase>::step()
{
}

