#pragma once

#include "GateTrigger.h"
#include "MidiPlayer.h"
#include "MidiSong.h"


template <class TBase>
class Seq : public TBase
{
public:
    template <class Tx>
    friend class SeqHost;

    Seq(struct Module * module) : TBase(module), gateTrigger(true)
    {
        init();
    }
    Seq() : TBase(), gateTrigger(true)
    {
        init();
    }

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
        CV_OUTPUT,
        GATE_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        GATE_LIGHT,
        NUM_LIGHTS
    };

    void step() override;

    MidiSongPtr getSong()
    {
        return player->getSong();
    }


    void stop()
    {
        player->stop();
    }
private:
    GateTrigger gateTrigger;
    void init();

    std::shared_ptr<MidiPlayer> player;
};

#if 1
template <class TBase>
class SeqHost : public IPlayerHost
{
public:
    SeqHost(Seq<TBase>* s) : seq(s)
    {
    }
    void setGate(bool gate) override
    {
        //fprintf(stderr, "setGate %d\n", gate); fflush(stderr);
        seq->outputs[Seq<TBase>::GATE_OUTPUT].value = gate ? 10.f : 0.f;
    }
    void setCV(float cv) override
    {
       // fprintf(stderr, "setCV %f\n", cv); fflush(stderr);
        seq->outputs[Seq<TBase>::CV_OUTPUT].value = cv;
    }
    void onLockFailed() override
    {

    }
private:
    Seq<TBase>* const seq;
};
#endif

template <class TBase>
void  Seq<TBase>::init()
{ 
    std::shared_ptr<IPlayerHost> host = std::make_shared<SeqHost<TBase>>(this);
    std::shared_ptr<MidiSong> song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    player = std::make_shared<MidiPlayer>(host, song);
}

template <class TBase>
void  Seq<TBase>::step()
{
    player->timeElapsed(TBase::engineGetSampleTime());
}



