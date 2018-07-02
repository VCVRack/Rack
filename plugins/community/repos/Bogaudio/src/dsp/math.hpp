#pragma once

#include "base.hpp"
#include "table.hpp"

namespace bogaudio {
namespace dsp {

struct FastTanhf {
	struct TanhfTable : Table {
		TanhfTable(int n) : Table(n) {}
		void _generate() override;
	};
	struct StaticTanhfTable : StaticTable<TanhfTable, 11> {};
	const Table& _table;

	FastTanhf() : _table(StaticTanhfTable::table())	{
	}

	float value(float radians);
};

} // namespace dsp
} // namespace bogaudio
