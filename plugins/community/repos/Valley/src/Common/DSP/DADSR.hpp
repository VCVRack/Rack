#pragma once

class DEnv {
public:
    DEnv() {
        _idling = true;
        _attacking = false;
        _decaying = false;
        _sustaining = false;
        _releasing = false;
        loop = false;
        value = 0.f;

        attackTime = 0.f;
        decayTime = 0.f;
        sustain = 1.f;
        releaseTime = 0.f;

        base = 20000.f;
        maxTime = 10.f;
        timeScale = 1.f;
        prevTrigState = 0.f;
        setSampleRate(44100.f);
    }
    void process(float gate, float trig) {
        if(gate >= 0.5f) {
            if(trig >= 0.5f && prevTrigState < 0.5f && !_triggered) {
                _triggered = true;
            }
            prevTrigState = trig;
            if(_idling || _releasing || _triggered) {
                _triggered = false;
                _attacking = true;
                _idling = false;
                _decaying = false;
                _sustaining = false;
                _releasing = false;
            }
        }
        else if(gate < 0.5f) {
            if(!_idling) {
                _attacking = false;
                _decaying = false;
                _sustaining = false;
                _releasing = true;
            }
        }

        if(_idling) {
            value = 0.f;
        }
        if(_attacking) {
            value += powf(base, 1 - attackTime) / maxTime * (1.01f - value) * _sampleTime * timeScale;
            if(value > (1.f - 1e-4)) {
                value = 1.f;
                _attacking = false;
                _decaying = true;
            }
        }
        if(_decaying) {
            value += powf(base, 1 - decayTime) / maxTime * (sustain - value) * _sampleTime * timeScale;
            if(value < sustain + 1e-4) {
                value = sustain;
                _decaying = false;
                _sustaining = true;
            }
        }
        if(loop) {
            if(_sustaining || _releasing) {
                value = 0.f;
                _attacking = true;
                _sustaining = false;
                _releasing = false;
            }
        }

        if(_sustaining) {
            value = sustain;
        }
        if(_releasing) {
            value += powf(base, 1 - releaseTime) / maxTime * (0.0f - value) * _sampleTime * timeScale;
            if(value < 1e-4) {
                value = 0.f;
                _releasing = false;
                _idling = true;
            }
        }
    }

    void setSampleRate(float sampleRate) {
        _sampleRate = sampleRate;
        _sampleTime = 1.f / _sampleRate;
    }

    float attackTime;
    float decayTime;
    float sustain;
    float releaseTime;
    float value;
    float timeScale;
    bool loop;
private:
    float _sampleRate, _sampleTime;
    float base, maxTime;
    float prevTrigState;
    bool _idling, _attacking, _decaying, _sustaining, _releasing, _triggered;

};
