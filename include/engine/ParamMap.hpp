#pragma once
#include "common.hpp"
#include <jansson.h>


namespace rack {
namespace engine {


struct ParamMap {
	int moduleId = -1;
	int paramId = -1;

	json_t *toJson();
	void fromJson(json_t *rootJ);
};


} // namespace engine
} // namespace rack
