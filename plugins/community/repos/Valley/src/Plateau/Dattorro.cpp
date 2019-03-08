#include "Dattorro.hpp"

Dattorro::Dattorro() {
    _dattorroScaleFactor = _sampleRate / _dattorroSampleRate;

    _preDelay = InterpDelay<double>(192010, 0);

    _inputLpf = OnePoleLPFilter(22000.0);
    _inputHpf = OnePoleHPFilter(0.0);

    _inApf1 = AllpassFilter<double>(dattorroScale(20 * _kInApf1Time), dattorroScale(_kInApf1Time), inputDiffusion1);
    _inApf2 = AllpassFilter<double>(dattorroScale(20 * _kInApf2Time), dattorroScale(_kInApf2Time), inputDiffusion1);
    _inApf3 = AllpassFilter<double>(dattorroScale(20 * _kInApf3Time), dattorroScale(_kInApf3Time), inputDiffusion2);
    _inApf4 = AllpassFilter<double>(dattorroScale(20 * _kInApf4Time), dattorroScale(_kInApf4Time), inputDiffusion2);

    _leftApf1 = AllpassFilter<double>(dattorroScale(40 * _kLeftApf1Time), dattorroScale(_kLeftApf1Time), -plateDiffusion1);
    _leftDelay1 = InterpDelay<double>(dattorroScale(40 * _kLeftDelay1Time), dattorroScale(_kLeftDelay1Time));
    _leftFilter = OnePoleLPFilter(reverbHighCut);
    _leftHpf = OnePoleHPFilter(reverbLowCut);
    _leftApf2 = AllpassFilter<double>(dattorroScale(40 * _kLeftApf2Time), dattorroScale(_kLeftApf2Time), plateDiffusion2);
    _leftDelay2 = InterpDelay<double>(dattorroScale(40 * _kLeftDelay2Time), dattorroScale(_kLeftDelay2Time));

    _rightApf1 = AllpassFilter<double>(dattorroScale(40 * _kRightApf1Time), dattorroScale(_kRightApf1Time), -plateDiffusion1);
    _rightDelay1 = InterpDelay<double>(dattorroScale(40 * _kRightDelay1Time), dattorroScale(_kRightDelay1Time));
    _rightFilter = OnePoleLPFilter(reverbHighCut);
    _rightHpf = OnePoleHPFilter(reverbLowCut);
    _rightApf2 = AllpassFilter<double>(dattorroScale(40 * _kRightApf2Time), dattorroScale(_kRightApf2Time), plateDiffusion2);
    _rightDelay2 = InterpDelay<double>(dattorroScale(40 * _kRightDelay2Time), dattorroScale(_kRightDelay2Time));

    _leftApf1Time = dattorroScale(_kLeftApf1Time);
    _leftApf2Time = dattorroScale(_kLeftApf2Time);
    _rightApf1Time = dattorroScale(_kRightApf1Time);
    _rightApf2Time = dattorroScale(_kRightApf2Time);

    for(auto i = 0; i < 7; ++i) {
        _scaledLeftTaps[i] = dattorroScale(_kLeftTaps[i]);
        _scaledRightTaps[i] = dattorroScale(_kRightTaps[i]);
    }

    _leftInputDCBlock.setCutoffFreq(20.0);
    _rightInputDCBlock.setCutoffFreq(20.0);
    _leftOutDCBlock.setCutoffFreq(20.0);
    _rightOutDCBlock.setCutoffFreq(20.0);

    _lfo1.setFrequency(_lfo1Freq);
    _lfo2.setFrequency(_lfo2Freq);
    _lfo3.setFrequency(_lfo3Freq);
    _lfo4.setFrequency(_lfo4Freq);

    _lfo2.phase = 0.25;
    _lfo3.phase = 0.5;
    _lfo4.phase = 0.75;

    _lfo1.setRevPoint(0.5);
    _lfo2.setRevPoint(0.5);
    _lfo3.setRevPoint(0.5);
    _lfo4.setRevPoint(0.5);

    _lfoDepth = dattorroScale(_kLfoExcursion);
}

void Dattorro::process(double leftInput, double rightInput) {
    if(!_freeze) {
        _decay = decay;
    }

    _leftFilter.setCutoffFreq(reverbHighCut);
    _leftHpf.setCutoffFreq(reverbLowCut);
    _rightFilter.setCutoffFreq(reverbHighCut);
    _rightHpf.setCutoffFreq(reverbLowCut);

    _lfo1.setFrequency(_lfo1Freq * modSpeed);
    _lfo2.setFrequency(_lfo2Freq * modSpeed);
    _lfo3.setFrequency(_lfo3Freq * modSpeed);
    _lfo4.setFrequency(_lfo4Freq * modSpeed);

    _leftApf1.gain = -plateDiffusion1;
    _leftApf2.gain = plateDiffusion2;
    _rightApf1.gain = -plateDiffusion1;
    _rightApf2.gain = plateDiffusion2;

    _leftApf1.delay.delayTime = _lfo1.process() * _lfoDepth * modDepth + _leftApf1Time;
    _leftApf2.delay.delayTime = _lfo2.process() * _lfoDepth * modDepth + _leftApf2Time;
    _rightApf1.delay.delayTime = _lfo3.process() * _lfoDepth * modDepth + _rightApf1Time;
    _rightApf2.delay.delayTime = _lfo4.process() * _lfoDepth * modDepth + _rightApf2Time;

    _leftInputDCBlock.input = leftInput;
    _rightInputDCBlock.input = rightInput;
    _inputLpf.setCutoffFreq(inputHighCut);
    _inputHpf.setCutoffFreq(inputLowCut);
    _inputLpf.input = _leftInputDCBlock.process() + _rightInputDCBlock.process();
    _inputHpf.input = _inputLpf.process();
    _inputHpf.process();
    _preDelay.input = _inputHpf.output;
    _preDelay.process();
    _inApf1.input = _preDelay.output;
    _inApf2.input = _inApf1.process();
    _inApf3.input = _inApf2.process();
    _inApf4.input = _inApf3.process();
    _tankFeed = _preDelay.output * (1.0 - diffuseInput) + _inApf4.process() * diffuseInput;
    _leftSum += _tankFeed;
    _rightSum += _tankFeed;

    _leftApf1.input = _leftSum;
    _leftDelay1.input = _leftApf1.process();
    _leftFilter.input = _leftDelay1.process();
    _leftHpf.input = _leftFilter.process();
    _leftApf2.input = (_leftDelay1.output * (1.0 - _fade) + _leftHpf.process() * _fade) * _decay;
    _leftDelay2.input = _leftApf2.process();
    _leftDelay2.process();

    _rightApf1.input = _rightSum;
    _rightDelay1.input = _rightApf1.process();
    _rightFilter.input = _rightDelay1.process();
    _rightHpf.input =  _rightFilter.process();
    _rightApf2.input = (_rightDelay1.output * (1.0 - _fade) + _rightHpf.process() * _fade) * _decay;
    _rightDelay2.input = _rightApf2.process();
    _rightDelay2.process();

    _rightSum = _leftDelay2.output * _decay;
    _leftSum = _rightDelay2.output * _decay;

    _leftOutDCBlock.input = _leftApf1.output;
    _leftOutDCBlock.input += _leftDelay1.tap(_scaledLeftTaps[0]);
    _leftOutDCBlock.input += _leftDelay1.tap(_scaledLeftTaps[1]);
    _leftOutDCBlock.input -= _leftApf2.delay.tap(_scaledLeftTaps[2]);
    _leftOutDCBlock.input += _leftDelay2.tap(_scaledLeftTaps[3]);
    _leftOutDCBlock.input -= _rightDelay1.tap(_scaledLeftTaps[4]);
    _leftOutDCBlock.input -= _rightApf2.delay.tap(_scaledLeftTaps[5]);
    _leftOutDCBlock.input -= _rightDelay2.tap(_scaledLeftTaps[6]);

    _rightOutDCBlock.input = _rightApf1.output;
    _rightOutDCBlock.input += _rightDelay1.tap(_scaledRightTaps[0]);
    _rightOutDCBlock.input += _rightDelay1.tap(_scaledRightTaps[1]);
    _rightOutDCBlock.input -= _rightApf2.delay.tap(_scaledRightTaps[2]);
    _rightOutDCBlock.input += _rightDelay2.tap(_scaledRightTaps[3]);
    _rightOutDCBlock.input -= _leftDelay1.tap(_scaledRightTaps[4]);
    _rightOutDCBlock.input -= _leftApf2.delay.tap(_scaledRightTaps[5]);
    _rightOutDCBlock.input -= _leftDelay2.tap(_scaledRightTaps[6]);
    leftOut = _leftOutDCBlock.process() * 0.5;
    rightOut = _rightOutDCBlock.process() * 0.5;

    _fade += _fadeStep * _fadeDir;
    _fade = (_fade < 0.0) ? 0.0 : ((_fade > 1.0) ? 1.0 : _fade);
}

void Dattorro::clear() {
    _preDelay.clear();
    _inputLpf.clear();
    _inputHpf.clear();
    _inApf1.clear();
    _inApf2.clear();
    _inApf3.clear();
    _inApf4.clear();

    _leftApf1.clear();
    _leftDelay1.clear();
    _leftFilter.clear();
    _leftHpf.clear();
    _leftApf2.clear();
    _leftDelay2.clear();

    _rightApf1.clear();
    _rightDelay1.clear();
    _rightFilter.clear();
    _rightHpf.clear();
    _rightApf2.clear();
    _rightDelay2.clear();

    _leftInputDCBlock.clear();
    _rightInputDCBlock.clear();
    _leftOutDCBlock.clear();
    _rightOutDCBlock.clear();

    _leftSum = 0.0f;
    _rightSum = 0.0f;
    leftOut = 0.0f;
    rightOut = 0.0f;
}

void Dattorro::setTimeScale(double timeScale) {
    _timeScale = timeScale;
    if(_timeScale < 0.0001) {
        _timeScale = 0.0001;
    }

    _leftDelay1.delayTime = dattorroScale(_kLeftDelay1Time * _timeScale);
    _leftDelay2.delayTime = dattorroScale(_kLeftDelay2Time * _timeScale);
    _rightDelay1.delayTime = dattorroScale(_kRightDelay1Time * _timeScale);
    _rightDelay2.delayTime = dattorroScale(_kRightDelay2Time * _timeScale);
    _leftApf1Time = dattorroScale(_kLeftApf1Time * _timeScale);
    _leftApf2Time = dattorroScale(_kLeftApf2Time * _timeScale);
    _rightApf1Time = dattorroScale(_kRightApf1Time * _timeScale);
    _rightApf2Time = dattorroScale(_kRightApf2Time * _timeScale);
}

void Dattorro::setPreDelay(double t) {
    _preDelayTime = t;
    _preDelay.delayTime = _preDelayTime * _sampleRate;
}

void Dattorro::setModShape(double shape) {
    _lfo1.setRevPoint(shape);
    _lfo2.setRevPoint(shape);
    _lfo3.setRevPoint(shape);
    _lfo4.setRevPoint(shape);
}

void Dattorro::setSampleRate(double sampleRate) {
    _sampleRate = sampleRate;
    _dattorroScaleFactor = _sampleRate / _dattorroSampleRate;
    setPreDelay(_preDelayTime);
    _inApf1.delay.delayTime = dattorroScale(_kInApf1Time);
    _inApf2.delay.delayTime = dattorroScale(_kInApf2Time);
    _inApf3.delay.delayTime = dattorroScale(_kInApf3Time);
    _inApf4.delay.delayTime = dattorroScale(_kInApf4Time);
    _leftDelay1.delayTime = dattorroScale(_kLeftDelay1Time * _timeScale);
    _leftDelay2.delayTime = dattorroScale(_kLeftDelay2Time * _timeScale);
    _rightDelay1.delayTime = dattorroScale(_kRightDelay1Time * _timeScale);
    _rightDelay2.delayTime = dattorroScale(_kRightDelay2Time * _timeScale);
    _leftApf1Time = dattorroScale(_kLeftApf1Time * _timeScale);
    _leftApf2Time = dattorroScale(_kLeftApf2Time * _timeScale);
    _rightApf1Time = dattorroScale(_kRightApf1Time * _timeScale);
    _rightApf2Time = dattorroScale(_kRightApf2Time * _timeScale);

    for(auto i = 0; i < 7; ++i) {
        _scaledLeftTaps[i] = dattorroScale(_kLeftTaps[i]);
        _scaledRightTaps[i] = dattorroScale(_kRightTaps[i]);
    }

    _lfoDepth = dattorroScale(_kLfoExcursion);
    _lfo1.setSamplerate(sampleRate);
    _lfo2.setSamplerate(sampleRate);
    _lfo3.setSamplerate(sampleRate);
    _lfo4.setSamplerate(sampleRate);
    _inputHpf.setSampleRate(sampleRate);
    _inputLpf.setSampleRate(sampleRate);
    _leftFilter.setSampleRate(sampleRate);
    _leftHpf.setSampleRate(sampleRate);
    _rightFilter.setSampleRate(sampleRate);
    _rightHpf.setSampleRate(sampleRate);

    _leftInputDCBlock.setSampleRate(sampleRate);
    _rightInputDCBlock.setSampleRate(sampleRate);
    _leftOutDCBlock.setSampleRate(sampleRate);
    _rightOutDCBlock.setSampleRate(sampleRate);
    _fadeStep = 1.0 / (0.1 * sampleRate);
    clear();
}

void Dattorro::freeze() {
    _freeze = true;
    _fadeDir = -1.0;
    _decay = 1.0;
}

void Dattorro::unFreeze() {
    _freeze = false;
    _fadeDir = 1.0;
    _decay = decay;
}

double Dattorro::dattorroScale(double delayTime) {
    return delayTime * _dattorroScaleFactor;
}
