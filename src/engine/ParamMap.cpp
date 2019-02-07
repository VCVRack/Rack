#include "engine/ParamMap.hpp"


namespace rack {
namespace engine {


json_t *ParamMap::toJson() {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "moduleId", json_integer(moduleId));
	json_object_set_new(rootJ, "paramId", json_integer(paramId));
	return rootJ;
}

void ParamMap::fromJson(json_t *rootJ) {
	json_t *moduleIdJ = json_object_get(rootJ, "moduleId");
	if (moduleIdJ)
		moduleId = json_integer_value(moduleIdJ);

	json_t *paramIdJ = json_object_get(rootJ, "paramId");
	if (paramIdJ)
		paramId = json_integer_value(paramIdJ);
}


} // namespace engine
} // namespace rack
