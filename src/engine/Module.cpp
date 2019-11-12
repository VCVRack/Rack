#include <engine/Module.hpp>
#include <plugin.hpp>


namespace rack {
namespace engine {


Module::Module() {
}

Module::~Module() {
	for (ParamQuantity* paramQuantity : paramQuantities) {
		if (paramQuantity)
			delete paramQuantity;
	}
}

void Module::config(int numParams, int numInputs, int numOutputs, int numLights) {
	// This method should only be called once.
	assert(params.empty() && inputs.empty() && outputs.empty() && lights.empty() && paramQuantities.empty());
	params.resize(numParams);
	inputs.resize(numInputs);
	outputs.resize(numOutputs);
	lights.resize(numLights);
	paramQuantities.resize(numParams);
	// Initialize paramQuantities
	for (int i = 0; i < numParams; i++) {
		configParam(i, 0.f, 1.f, 0.f);
	}
}

json_t* Module::toJson() {
	json_t* rootJ = json_object();

	// id
	json_object_set_new(rootJ, "id", json_integer(id));

	// plugin
	json_object_set_new(rootJ, "plugin", json_string(model->plugin->slug.c_str()));

	// version
	json_object_set_new(rootJ, "version", json_string(model->plugin->version.c_str()));

	// model
	json_object_set_new(rootJ, "model", json_string(model->slug.c_str()));

	// params
	json_t* paramsJ = json_array();
	for (size_t paramId = 0; paramId < params.size(); paramId++) {
		// Don't serialize unbounded Params
		if (!paramQuantities[paramId]->isBounded())
			continue;

		json_t* paramJ = json_object();

		json_object_set_new(paramJ, "id", json_integer(paramId));

		float value = params[paramId].getValue();
		json_object_set_new(paramJ, "value", json_real(value));

		json_array_append(paramsJ, paramJ);
	}
	json_object_set_new(rootJ, "params", paramsJ);

	// bypass
	if (bypass)
		json_object_set_new(rootJ, "bypass", json_boolean(bypass));

	// leftModuleId
	if (leftExpander.moduleId >= 0)
		json_object_set_new(rootJ, "leftModuleId", json_integer(leftExpander.moduleId));

	// rightModuleId
	if (rightExpander.moduleId >= 0)
		json_object_set_new(rootJ, "rightModuleId", json_integer(rightExpander.moduleId));

	// data
	json_t* dataJ = dataToJson();
	if (dataJ) {
		json_object_set_new(rootJ, "data", dataJ);
	}

	return rootJ;
}

void Module::fromJson(json_t* rootJ) {
	// Check if plugin and model are incorrect
	json_t* pluginJ = json_object_get(rootJ, "plugin");
	std::string pluginSlug;
	if (pluginJ) {
		pluginSlug = json_string_value(pluginJ);
		pluginSlug = plugin::normalizeSlug(pluginSlug);
		if (pluginSlug != model->plugin->slug) {
			WARN("Plugin %s does not match Module's plugin %s.", pluginSlug.c_str(), model->plugin->slug.c_str());
			return;
		}
	}

	json_t* modelJ = json_object_get(rootJ, "model");
	std::string modelSlug;
	if (modelJ) {
		modelSlug = json_string_value(modelJ);
		modelSlug = plugin::normalizeSlug(modelSlug);
		if (modelSlug != model->slug) {
			WARN("Model %s does not match Module's model %s.", modelSlug.c_str(), model->slug.c_str());
			return;
		}
	}

	// Check plugin version
	json_t* versionJ = json_object_get(rootJ, "version");
	if (versionJ) {
		std::string version = json_string_value(versionJ);
		if (version != model->plugin->version) {
			INFO("Patch created with %s v%s, currently using v%s.", pluginSlug.c_str(), version.c_str(), model->plugin->version.c_str());
		}
	}

	// Only set ID if unset
	if (id < 0) {
		// id
		json_t* idJ = json_object_get(rootJ, "id");
		if (idJ)
			id = json_integer_value(idJ);
	}

	// params
	json_t* paramsJ = json_object_get(rootJ, "params");
	size_t i;
	json_t* paramJ;
	json_array_foreach(paramsJ, i, paramJ) {
		// Get paramId
		json_t* paramIdJ = json_object_get(paramJ, "id");
		// Legacy v0.6 to <v1
		if (!paramIdJ)
			paramIdJ = json_object_get(paramJ, "paramId");
		size_t paramId;
		if (paramIdJ)
			paramId = json_integer_value(paramIdJ);
		// Use index if all else fails
		else
			paramId = i;

		// Check ID bounds
		if (paramId >= params.size())
			continue;

		// Check that the Param is bounded
		if (!paramQuantities[paramId]->isBounded())
			continue;

		json_t* valueJ = json_object_get(paramJ, "value");
		if (valueJ)
			params[paramId].setValue(json_number_value(valueJ));
	}

	// bypass
	json_t* bypassJ = json_object_get(rootJ, "bypass");
	if (bypassJ)
		bypass = json_boolean_value(bypassJ);

	// These do not need to be deserialized, since the module positions will set them correctly when added to the rack.
	// // leftModuleId
	// json_t *leftModuleIdJ = json_object_get(rootJ, "leftModuleId");
	// if (leftModuleIdJ)
	// 	leftExpander.moduleId = json_integer_value(leftModuleIdJ);

	// // rightModuleId
	// json_t *rightModuleIdJ = json_object_get(rootJ, "rightModuleId");
	// if (rightModuleIdJ)
	// 	rightExpander.moduleId = json_integer_value(rightModuleIdJ);

	// data
	json_t* dataJ = json_object_get(rootJ, "data");
	if (dataJ)
		dataFromJson(dataJ);
}


} // namespace engine
} // namespace rack
