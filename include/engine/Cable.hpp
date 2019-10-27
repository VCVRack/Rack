#pragma once
#include <common.hpp>
#include <engine/Module.hpp>


namespace rack {
namespace engine {


struct Cable {
	int id = -1;
	Module* inputModule = NULL;
	int inputId;
	Module* outputModule = NULL;
	int outputId;

	json_t* toJson();
	void fromJson(json_t* rootJ);
};


} // namespace engine
} // namespace rack
