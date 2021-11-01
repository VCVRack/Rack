#pragma once
#include <common.hpp>
#include <engine/Module.hpp>


namespace rack {
namespace engine {


struct Cable {
	/** Unique ID for referring to the cable in the engine.
	Between 0 and 2^53 since this is serialized with JSON.
	Assigned when added to the engine.
	*/
	int64_t id = -1;
	Module* inputModule = NULL;
	int inputId = -1;
	Module* outputModule = NULL;
	int outputId = -1;

	json_t* toJson();
	void fromJson(json_t* rootJ);
	PRIVATE static void jsonStripIds(json_t* rootJ);
};


} // namespace engine
} // namespace rack
