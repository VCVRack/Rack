#pragma once

#include <memory>
#include "assert.h"

#include "AudioMath.h"
#include "ManagedPool.h"
#include "ThreadClient.h"
#include "ThreadServer.h"
#include "ThreadSharedState.h"

#include "FFTData.h"
#include "FFT.h"
#include "FFTCrossFader.h"

class NoiseMessage;

const int crossfadeSamples = 4 * 1024;
template <class TBase>

/**
 * Implementation of the "Colors" noises generator
 */
class ColoredNoise : public TBase
{
public:
    ColoredNoise(struct Module * module) : TBase(module), crossFader(crossfadeSamples)
    {
        commonConstruct();
    }
    ColoredNoise() : TBase(), crossFader(crossfadeSamples)
    {
        commonConstruct();
    }
    ~ColoredNoise()
    {
        thread.reset();     // kill the threads before deleting other things
    }

    void setSampleRate(float rate)
    {
    }

    // must be called after setSampleRate
    void init()
    {
        cv_scaler = AudioMath::makeLinearScaler<T>(-8, 8);
    }

    // Define all the enums here. This will let the tests and the widget access them.
    enum ParamIds
    {
        SLOPE_PARAM,
        SLOPE_TRIM,
        NUM_PARAMS
    };

    enum InputIds
    {
        SLOPE_CV,
        NUM_INPUTS
    };

    enum OutputIds
    {
        AUDIO_OUTPUT,
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

    float getSlope() const;

    int _msgCount() const;  // just for debugging

    typedef float T;        // use floats for all signals
private:

    AudioMath::ScaleFun<T> cv_scaler;
    bool isRequestPending = false;

    /**
     * crossFader generates the audio, but we must
     * feed it with NoiseMessage data from the ThreadServer
     */
    FFTCrossFader   crossFader;

    // just for debugging
    int messageCount = 0;

    std::unique_ptr<ThreadClient> thread;

    /**
     * Messages moved between thread, messagePool, and crossFader
     * as new noise slopes are requested in response to CV/knob changes.
     */
    ManagedPool<NoiseMessage, 2> messagePool;

    void serviceFFTServer();
    void serviceAudio();
    void serviceInputs();
    void commonConstruct();
};

class NoiseMessage : public ThreadMessage
{
public:

    NoiseMessage() : ThreadMessage(Type::NOISE),
        dataBuffer(new FFTDataReal(defaultNumBins))
    {
    }

    NoiseMessage(int numBins) : ThreadMessage(Type::NOISE),
        dataBuffer(new FFTDataReal(numBins))
    {
    }
    ~NoiseMessage()
    {
    }
    const int defaultNumBins = 64 * 1024;

    ColoredNoiseSpec noiseSpec;

    /** Server is going to fill this buffer up with time-domain data
     */
    std::unique_ptr<FFTDataReal> dataBuffer;
};

class NoiseServer : public ThreadServer
{
public:
    NoiseServer(std::shared_ptr<ThreadSharedState> state) : ThreadServer(state)
    {
    }
protected:
    /**
     * This is called on the server thread, not the audio thread.
     * We have plenty of time to do some heavy lifting here.
     */
    virtual void handleMessage(ThreadMessage* msg) override
    {
        if (msg->type != ThreadMessage::Type::NOISE) {
            assert(false);
            return;
        }

        // Unpack the parameters, convert to frequency domain "noise" recipe
        NoiseMessage* noiseMessage = static_cast<NoiseMessage*>(msg);
        reallocSpectrum(noiseMessage);
        FFT::makeNoiseSpectrum(noiseSpectrum.get(),
            noiseMessage->noiseSpec);

        // Now inverse FFT to time domain noise in client's buffer
        FFT::inverse(noiseMessage->dataBuffer.get(), *noiseSpectrum.get());
        FFT::normalize(noiseMessage->dataBuffer.get(), 5);          // use 5v amplitude.
        sendMessageToClient(noiseMessage);
    }
private:
    std::unique_ptr<FFTDataCpx> noiseSpectrum;

    // may do nothing, may create the first buffer,
    // may delete the old buffer and make a new one.
    void reallocSpectrum(const NoiseMessage* msg)
    {
        if (noiseSpectrum && ((int) noiseSpectrum->size() == msg->dataBuffer->size())) {
            return;
        }

        noiseSpectrum.reset(new FFTDataCpx(msg->dataBuffer->size()));
    }
};

template <class TBase>
float ColoredNoise<TBase>::getSlope() const
{
    const NoiseMessage* curMsg = crossFader.playingMessage();
    return curMsg ? curMsg->noiseSpec.slope : 0;
}

template <class TBase>
void ColoredNoise<TBase>::commonConstruct()
{
    crossFader.enableMakeupGain(true);
    std::shared_ptr<ThreadSharedState> threadState = std::make_shared<ThreadSharedState>();
    std::unique_ptr<ThreadServer> server(new NoiseServer(threadState));

    std::unique_ptr<ThreadClient> client(new ThreadClient(threadState, std::move(server)));
    this->thread = std::move(client);
}

template <class TBase>
int ColoredNoise<TBase>::_msgCount() const
{
    return messageCount;
}


template <class TBase>
void ColoredNoise<TBase>::serviceFFTServer()
{
    // see if we need to request first frame of sample data
    // first request will be white noise. Is that ok?
    if (!isRequestPending && crossFader.empty()) {
        assert(!messagePool.empty());
        NoiseMessage* msg = messagePool.pop();

        bool sent = thread->sendMessage(msg);
        if (sent) {
            isRequestPending = true;
        } else {
            messagePool.push(msg);
        }
    }

    // see if any messages came back for us
    ThreadMessage* newMsg = thread->getMessage();
    if (newMsg) {
        ++messageCount;
        assert(newMsg->type == ThreadMessage::Type::NOISE);
        NoiseMessage* noise = static_cast<NoiseMessage*>(newMsg);

        isRequestPending = false;

        // put it in the cross fader for playback
        // give the last one back
        NoiseMessage* oldMsg = crossFader.acceptData(noise);
        if (oldMsg) {
            messagePool.push(oldMsg);
        }
    }
}

template <class TBase>
void ColoredNoise<TBase>::serviceAudio()
{
    float output = 0;
    NoiseMessage* oldMessage = crossFader.step(&output);
    if (oldMessage) {
        // One frame may be done fading - we can take it back.
        messagePool.push(oldMessage);
    }

    TBase::outputs[AUDIO_OUTPUT].value = output;
}


template <class TBase>
void ColoredNoise<TBase>::serviceInputs()
{
    if (isRequestPending) {
        return;     // can't do anything until server is free.
    }
    if (crossFader.empty()) {
        return;     // if we don't have data, we will be asking anyway
    }
    if (messagePool.empty()) {
        return;     // all our buffers are in use
    }

    T combinedSlope = cv_scaler(
        TBase::inputs[SLOPE_CV].value,
        TBase::params[SLOPE_PARAM].value,
        TBase::params[SLOPE_TRIM].value);

    // get slope input to one decimal place
    int i = int(combinedSlope * 10);
    combinedSlope = i / 10.f;
    ColoredNoiseSpec sp;
    sp.slope = combinedSlope;
    sp.highFreqCorner = 6000;
    const NoiseMessage* playingData = crossFader.playingMessage();
    if (!playingData || !(sp != playingData->noiseSpec)) {
        // If we aren't playing yet, or no change in slope,
        // the don't do anything
        return;
    }

    assert(!messagePool.empty());
    NoiseMessage* msg = messagePool.pop();
    assert(msg);
    if (!msg) {
        return;
    }
    msg->noiseSpec = sp;
    // TODO: put this logic in one place
    bool sent = thread->sendMessage(msg);
    if (sent) {
        isRequestPending = true;
    } else {
        messagePool.push(msg);
    }
}

template <class TBase>
void ColoredNoise<TBase>::step()
{
    serviceFFTServer();
    serviceAudio();
    serviceInputs();
}