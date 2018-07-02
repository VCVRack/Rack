#pragma once

#include <string.h>
#include <algorithm>

#include "rack.hpp"

using namespace rack;

namespace bogaudio {

template <typename TModule, typename TModuleWidget, typename... Tags>
Model* createModel(
	const char* slug,
	const char* name,
	const char* description,
	Tags... tags
) {
	const int n = 256;
	char buf[n];
	std::string uName = name;
	for (auto& c: uName) {
		c = toupper(c);
	}
	snprintf(buf, n, "%s - %s - %dHP", uName.c_str(), description, TModuleWidget::hp);
	return Model::create<TModule, TModuleWidget>(
		"Bogaudio",
		slug,
		buf,
		tags...
	);
}

} // namespace bogaudio
