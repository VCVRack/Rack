#pragma once

#include <assert.h>

#include "base.hpp"

namespace bogaudio {
namespace dsp {

class Table {
protected:
	int _length = 0;
	float* _table = NULL;

public:
	Table(int n = 10) {
		assert(n > 0);
		assert(n <= 16);
		_length = 1 << n;
	}
	virtual ~Table() {
		if (_table) {
			delete[] _table;
		}
	}

	inline int length() const { return _length; }

	inline float value(int i) const {
		assert(i >= 0 && i < _length);
		assert(_table);
		return _table[i];
	}

	void generate();

protected:
	virtual void _generate() = 0;
};

template<class T, int N> class StaticTable {
private:
	Table* _table = NULL;

	StaticTable() {
	}
	~StaticTable() {
		if (_table) {
			delete _table;
		}
	}

public:
	StaticTable(const StaticTable&) = delete;
	void operator=(const StaticTable&) = delete;

	static const Table& table() {
		static StaticTable<T, N> instance;
		if (!instance._table) {
			instance._table = new T(N);
			instance._table->generate();
		}
		return *instance._table;
	}
};

struct SineTable : Table {
	SineTable(int n = 10) : Table(n) {}
	void _generate() override;
};
struct StaticSineTable : StaticTable<SineTable, 12> {};

struct BlepTable : Table {
	BlepTable(int n = 10) : Table(n) {}
	void _generate() override;
};
struct StaticBlepTable : StaticTable<BlepTable, 12> {};

} // namespace dsp
} // namespace bogaudio
