//
// Oneshot.cpp
// Author: Dale Johnson
// Contact: valley.audio.soft@gmail.com
// Date: 5/12/2017
//
// Copyright 2017 Dale Johnson
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#include "Oneshot.hpp"

Oneshot::Oneshot() {
    _state = 0;
    _sampleRate = 44100.0;
    setDuration(0.100);
    _elapsedTime = 0.0;
}

Oneshot::Oneshot(float duration, float sampleRate) {
    _state = 0;
    _sampleRate = sampleRate;
    setDuration(duration);
    _elapsedTime = 0.0;
}

void Oneshot::trigger() {
    _state = 1;
    _elapsedTime = 0.0;
}

void Oneshot::process() {
    if(_elapsedTime >= _duration) {
        _state = 0;
        _elapsedTime = 0.0;
    }
    if(_state == 1) {
        _elapsedTime += _deltaTime;
    }
}

void Oneshot::setSampleRate(float sampleRate) {
    _sampleRate = sampleRate;
    _deltaTime = 1.0 / _sampleRate;
}

void Oneshot::setDuration(float duration) {
    _duration = duration;
    if(_duration < 0.0) {
        _duration = 0.0;
    }
    _deltaTime = 1.0 / _sampleRate;
}

int Oneshot::getState() const {
    return _state;
}
