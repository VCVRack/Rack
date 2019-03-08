#include "engine/Module.hpp"


namespace rack {
namespace engine {


Module::Module() {
}

void Module::config(int numParams, int numInputs, int numOutputs, int numLights) {
	params.resize(numParams);
	// Create default param labels
	for (int i = 0; i < numParams; i++) {
		params[i].label = string::f("#%d", i + 1);
	}
	inputs.resize(numInputs);
	outputs.resize(numOutputs);
	lights.resize(numLights);
}

json_t *Module::toJson() {
	json_t *rootJ = json_object();

	// params
	json_t *paramsJ = json_array();
	for (Param &param : params) {
		json_t *paramJ = param.toJson();
		json_array_append_new(paramsJ, paramJ);
	}
	json_object_set_new(rootJ, "params", paramsJ);

	// bypass
	if (bypass)
		json_object_set_new(rootJ, "bypass", json_boolean(bypass));

	// data
	json_t *dataJ = dataToJson();
	if (dataJ) {
		json_object_set_new(rootJ, "data", dataJ);
	}

	// leftModuleId
	json_object_set_new(rootJ, "leftModuleId", json_integer(leftModuleId));

	// rightModuleId
	json_object_set_new(rootJ, "rightModuleId", json_integer(rightModuleId));

	return rootJ;
}

void Module::fromJson(json_t *rootJ) {
	// params
	json_t *paramsJ = json_object_get(rootJ, "params");
	size_t i;
	json_t *paramJ;
	json_array_foreach(paramsJ, i, paramJ) {
		uint32_t paramId = i;
		// Get paramId
		// Legacy v0.6.0 to <v1.0
		json_t *paramIdJ = json_object_get(paramJ, "paramId");
		if (paramIdJ) {
			paramId = json_integer_value(paramIdJ);
		}

		if (paramId < params.size()) {
			params[paramId].fromJson(paramJ);
		}
	}

	// bypass
	json_t *bypassJ = json_object_get(rootJ, "bypass");
	if (bypassJ)
		bypass = json_boolean_value(bypassJ);

	// data
	json_t *dataJ = json_object_get(rootJ, "data");
	if (dataJ)
		dataFromJson(dataJ);

	// leftModuleId
	json_t *leftModuleIdJ = json_object_get(rootJ, "leftModuleId");
	if (leftModuleIdJ)
		leftModuleId = json_integer_value(leftModuleIdJ);

	// rightModuleId
	json_t *rightModuleIdJ = json_object_get(rootJ, "rightModuleId");
	if (rightModuleIdJ)
		rightModuleId = json_integer_value(rightModuleIdJ);
}

void Module::reset() {
	for (Param &param : params) {
		param.reset();
	}
	onReset();
}

void Module::randomize() {
	for (Param &param : params) {
		if (param.randomizable)
			param.randomize();
	}
	onRandomize();
}


} // namespace engine
} // namespace rack
