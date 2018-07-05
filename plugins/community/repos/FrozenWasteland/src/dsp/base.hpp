#pragma once

namespace frozenwasteland {
namespace dsp {

struct Generator {
	float _current = 0.0;

	Generator() {}
	virtual ~Generator() {}

	float current() {
		return _current;
	}

	float next() {
		return _current = _next();
	}

	virtual float _next() = 0;
};

} // namespace dsp
} // namespace frozenwasteland
