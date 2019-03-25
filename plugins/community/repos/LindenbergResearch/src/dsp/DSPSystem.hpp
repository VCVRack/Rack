#pragma once
#define DEFAULT_SR 44100.0f
#define TRIGGER_PROCESSING true

/**
 * @brief Basic DSP types
 */

namespace dsp {

/**
 * @brief Represents an IOPort of a DSP system
 */
struct DSPPort {
    float value;
};


/**
 * @brief Represents a parameter of a DSP system
 */
struct DSPParam {
    float value;
};


/**
 * @brief Base class for all signal processors
 */
template<int NUM_IN, int NUM_OUT, int NUM_PARAM>
struct DSPSystem {

    /**
     * @brief Enumerate all Inputs
     */
    enum Inputs {
    };

    /**
     * @brief Enumerate all Outputs
     */
    enum Outputs {
    };

    /**
     * @brief Enumerate all Parameters
     */
    enum Params {
    };

protected:
#ifdef _MSC_VER
        DSPPort input[NUM_IN + 1];
        DSPPort output[NUM_OUT + 1];
        DSPParam param[NUM_PARAM + 1];
#else
        DSPPort input[NUM_IN + 1] = {};
        DSPPort output[NUM_OUT + 1] = {};
        DSPParam param[NUM_PARAM] = {};
#endif

    float sr;

public:

    /**
     * @brief Default constructor
     */
    DSPSystem() {
        sr = DEFAULT_SR;
    }


    /**
     * @brief Init system with sample rate
     * @param sr
     */
    explicit DSPSystem(float sr) : sr(sr) {}


    virtual /**
         * @brief Update sample rate on change
         * @param sr
         */
    void updateSampleRate(float sr) {
        DSPSystem::sr = sr;
        invalidate();
    }


    /**
     * @brief Update a parameter of the system
     * @param id Parameter ID
     * @param value Value
     * @param trigger Trigger call of invalidate() - use false to supress
     */
    void setParam(int id, float value, bool trigger = true) {
        if (param[id].value != value) {
            param[id].value = value;

            /* setup of new parameter triggers invalidation per default */
            if (trigger) {
                invalidate();
            }
        }

    }


    /**
     * @brief Get the current parameter value by ID
     * @param id Parameter ID
     * @return current value
     */
    float getParam(int id) {
        return param[id].value;
    }


    /**
     * @brief Get the current value of the output by ID
     * @param id
     * @return
     */
    float getOutput(int id) {
        return output[id].value;
    }


    /**
     * @brief Set input port to new value
     * @param id Port ID
     * @param value
     */
    void setInput(int id, float value, bool proccess = false) {
        input[id].value = value;

        if (proccess) {
            process();
        }
    }


    /**
     * @brief Method for mark parameters as invalidate to trigger recalculation
     */
    virtual void invalidate() {};


    /**
     * @brief Process one step and return the computed sample
     * @return
     */
    virtual void process() {};
};


/**
 * @brief Basic 1 in and 1 out system definition
 */
struct DSPSystem1x1 : DSPSystem<1, 1, 0> {
    enum Inputs {
        IN
    };

    enum Outputs {
        OUT
    };


    /**
      * @brief Get the delayed sample from signal, to processing are triggered
      * @return
      */
    float get() {
        return output[OUT].value;
    }


    /**
     * @brief Set new value to Input and trigger processing
     * @param value
     */
    void set(float value) {
        setInput(IN, value, TRIGGER_PROCESSING);
    }
};


/**
 * @brief Basic 1 in and 1 out system definition
 */
struct DSPSystem2x1 : DSPSystem<2, 1, 0> {
    enum Inputs {
        IN1,
        IN2
    };

    enum Outputs {
        OUT
    };


    /**
      * @brief Get the delayed sample from signal, to processing are triggered
      * @return
      */
    float get() {
        return output[OUT].value;
    }


    /**
     * @brief Set new value to Input and trigger processing
     * @param value
     */
    void set(float in1, float in2, bool proccess = true) {
        setInput(IN1, in1);
        setInput(IN2, in2, proccess);

    }
};


/**
 * @brief Basic 1 in and 1 out system definition
 */
struct DSPSystem2x2 : DSPSystem<2, 2, 0> {
    enum Inputs {
        IN1,
        IN2
    };

    enum Outputs {
        OUT1,
        OUT2
    };


    /**
      * @brief Get the delayed sample from signal, to processing are triggered
      * @return
      */
    float get(int out = 0) {
        return output[out].value;
    }


    /**
     * @brief Set new value to Input and trigger processing
     * @param value
     */
    void set(float in1, float in2, bool process = true) {
        setInput(IN1, in1);
        setInput(IN2, in2, process);

    }
};


/**
 * @brief Delayed signal model
 * @tparam SIZE
 */
template<int SIZE>
struct DSPDelay : DSPSystem1x1 {

private:
    float buffer[SIZE] = {};


    /**
     * @brief Shift left all elements
     */
    void shift() {
        for (int i = 0; i < SIZE - 1; i++) {
            buffer[i] = buffer[i + 1];
        }
    }


public:

    /**
     * @brief Proccess the Delay
     */
    void process() override {
        /* shift all elements left */
        shift();
        /* set last element to current input */
        buffer[SIZE - 1] = input[IN].value;
        /* set output */
        output[OUT].value = buffer[0];
    }
};


/**
 * @brief Shortcut for a classic z^-1 delay (1-Sample)
 */
typedef DSPDelay<1> DSPDelay1;
}
