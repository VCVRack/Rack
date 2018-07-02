#include <stdio.h>
#include <math.h>

#include "table.hpp"
#include "analyzer.hpp"

using namespace bogaudio::dsp;

void Table::generate() {
	if (!_table) {
		_table = new float[_length] {};
		_generate();
	}
}


void SineTable::_generate() {
	const float twoPI = 2.0f * M_PI;
	for (int i = 0, j = _length / 4; i <= j; ++i) {
		_table[i] = sinf(twoPI * (i / (float)_length));
	}
	for (int i = 1, j = _length / 4; i < j; ++i) {
		_table[i + j] = _table[j - i];
	}
	for (int i = 0, j = _length / 2; i < j; ++i) {
		_table[i + j] = -_table[i];
	}
}


void BlepTable::_generate() {
	// some amount of a sinc function.
	const float scaledPi = M_PI * 10.0f;
	_table[_length / 2] = 0.0f;
	for (int i = 1, j = _length / 2; i < j; ++i) {
		float radians = scaledPi * (i / (float)j);
		_table[j + i] = sinf(radians) / radians;
	}

	// "integrate": FIXME: avoid magic normalization value.
	const float norm = _length / 40.0f;
	float sum = 0.0f;
	for (int i = _length / 2; i < _length; ++i) {
		sum += _table[i];
		_table[i] = sum / norm;
	}

	// offset.
	for (int i = _length / 2; i < _length; ++i) {
		_table[i] -= 1.0f; // assumes successful normalization to 1-ish.
	}

	// copy to first half of table.
	for (int i = 0, j = _length / 2; i < j; ++i) {
		_table[i] = -_table[_length - 1 - i];
	}

	// smooth it out even more.
	HammingWindow hw(_length);
	hw.apply(_table, _table);
}
