#include <engine/Cable.hpp>
#include <engine/Engine.hpp>
#include <context.hpp>


namespace rack {
namespace engine {


json_t* Cable::toJson() {
	json_t* rootJ = json_object();
	json_object_set_new(rootJ, "id", json_integer(id));
	json_object_set_new(rootJ, "outputModuleId", json_integer(outputModule->id));
	json_object_set_new(rootJ, "outputId", json_integer(outputId));
	json_object_set_new(rootJ, "inputModuleId", json_integer(inputModule->id));
	json_object_set_new(rootJ, "inputId", json_integer(inputId));
	return rootJ;
}


void Cable::fromJson(json_t* rootJ) {
	// inputModuleId
	json_t* inputModuleIdJ = json_object_get(rootJ, "inputModuleId");
	if (!inputModuleIdJ)
		throw Exception("inputModuleId not found for cable");
	int64_t inputModuleId = json_integer_value(inputModuleIdJ);
	inputModule = APP->engine->getModule(inputModuleId);
	if (!inputModule)
		throw Exception("inputModule not found for cable");

	// inputId
	json_t* inputIdJ = json_object_get(rootJ, "inputId");
	if (!inputIdJ)
		throw Exception("inputId not found for cable");
	inputId = json_integer_value(inputIdJ);

	// outputModuleId
	json_t* outputModuleIdJ = json_object_get(rootJ, "outputModuleId");
	if (!outputModuleIdJ)
		throw Exception("outputModuleId not found for cable");
	int64_t outputModuleId = json_integer_value(outputModuleIdJ);
	outputModule = APP->engine->getModule(outputModuleId);
	if (!outputModule)
		throw Exception("outputModule not found for cable");

	// outputId
	json_t* outputIdJ = json_object_get(rootJ, "outputId");
	if (!outputIdJ)
		throw Exception("outputId not found for cable");
	outputId = json_integer_value(outputIdJ);

	// Only set ID if unset
	if (id < 0) {
		// id
		json_t* idJ = json_object_get(rootJ, "id");
		// Before 1.0, cables IDs were not used, so just leave it as default and Engine will assign one automatically.
		if (idJ)
			id = json_integer_value(idJ);
	}
}


} // namespace engine
} // namespace rack
