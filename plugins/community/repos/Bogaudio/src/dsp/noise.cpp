
#include <algorithm>

#include "noise.hpp"

using namespace bogaudio::dsp;


Seeds::Seeds() {
  std::random_device rd;
  _generator.seed(rd());
}

unsigned int Seeds::_next() {
  return _generator();
}

Seeds& Seeds::getInstance() {
  static Seeds instance;
  return instance;
}

unsigned int Seeds::next() {
  return getInstance()._next();
};


void RandomWalk::setParams(float sampleRate, float change) {
	assert(sampleRate > 0.0f);
	assert(change >= 0.0f);
	assert(change <= 1.0f);

	_filter.setParams(sampleRate, std::max(2.0f, change * 0.49f * sampleRate));

	const float maxDamp = 0.98;
	const float minDamp = 0.9999;
	_damp = maxDamp + (1 - change)*(minDamp - maxDamp);
}

void RandomWalk::jump() {
	// FIXME
	_bias = _noise.next() * 5.0f;
	_filter.reset();
}

float RandomWalk::_next() {
	float delta = _noise.next();
	if ((_lastOut >= _max - _bias && delta > 0.0f) || (_lastOut <= _min - _bias && delta < 0.0f)) {
		delta = -delta;
	}
	_last = _damp*_last + delta;
	return _lastOut = std::min(std::max(_bias + _filter.next(_last), _min), _max);
}
