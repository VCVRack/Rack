#ifndef TRIGGERSEQUENCER
#define TRIGGERSEQUENCER

class TriggerSequencer
{
public:
    enum EVT
    {
        TRIGGER,
        END
    };
    class Event
    {
    public:
        short int evt;
        short int delay;
    };

    // data is a buffer that will be bound to the sequencer, although it is "owned"
    // by the caller
    TriggerSequencer(const Event * data) : curEvent(data), trigger(false), delay(0)
    {
        reset(data);
    }

    // send one clock to the seq
    void clock();

    // reset by concatenating new data to seq
    // note that reset may generate a trigger
    void reset(const Event * data)
    {
        //printf("reset: initial delay = %d\n", delay);
        curEvent = data;
        delay += data->delay;		// should this be += delay??
        //printf("after reset: delay = %d\n", delay);
        processClocks();
    }

    bool getTrigger() const
    {
        return trigger;
    }	// get trigger state after last clock
    bool getEnd() const
    {
        return curEvent == 0;
    }		// did sequencer end after last clock?


    // checks that a sequence is valid
    static bool isValid(const Event * data);
private:
    const Event * curEvent;
    bool trigger;
    short delay;

    void processClocks();
};

inline void TriggerSequencer::clock()
{
    --delay;
    processClocks();
}

inline void TriggerSequencer::processClocks()
{
    trigger = false;
    //printf("enter proc clock, curevt =%p, delay = %d\n", curEvent, delay);
    if (!curEvent) {
        //printf("leave clock early - ended\n");
        return;	// seq is stopped
    }


    while (delay < 0) {
        //printf("delay went to %d, evt=%d\n", delay, curEvent->evt);
        switch (curEvent->evt) {
            case END:
                //printf("setting end at 58\n");
                curEvent = 0;		// stop seq by clering ptr
                return;
            case TRIGGER:
                trigger = true;
                ++curEvent;			// and go to next one
                //printf("trigger set true\n");
                break;

            default:
                assert(false);
        }

        delay += curEvent->delay;
    }

    //printf("leave clock %d\n", delay);
};


inline bool TriggerSequencer::isValid(const Event * data)
{
    while (data->evt != END) {
        assert(data->evt == TRIGGER);
        assert(data->delay >= 0);
        assert(data->delay < 2000);	// just for now - we expect them to be small
        ++data;
    }

    return true;
}



#endif
